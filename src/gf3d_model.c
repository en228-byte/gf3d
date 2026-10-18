#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_types.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_text.h"

#include "gf3d_buffers.h"
#include "gf3d_swapchain.h"
#include "gf3d_commands.h"
#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_texture.h"
#include "gf3d_model.h"
#include "gf2d_sprite.h"
#include "gf3d_mesh.h"

#define MESH_ATTRIBUTE_COUNT 3

typedef struct
{
    Uint32          model_max;      /**<maximum concurrent sprites supported*/
    VkDevice        device;           /**<logical vulkan device*/
    Pipeline       *pipe;             /**<the pipeline associated with sprite rendering*/
    Texture        *defaultTexture;
    Model          *model_list;
}ModelManager;

void gf3d_model_close();

static ModelManager gf3d_model = {0};

/**
 * @brief initializes the mesh system / configures internal data about mesh based rendering
 * @param model_max the maximum allowed simultaneous meshes supported at once.  Must be > 0
 * @note keep in mind that many models will be comprised of multiple sub meshes.  So this number may need to be very large
 */
void gf3d_model_manager_init(Uint32 model_max){
    if (!model_max)
    {
        slog("cannot intialize model manager without model_max");
        return;
    }
    if (gf3d_model.model_max != 0)
    {
        slog("Cannot initialize model manager more than once");
        return;
    }
    if (model_max == 0) {
        slog("Cannot initialize for 0 models");
        return;
    }
    gf3d_model.model_list = gfc_allocate_array(sizeof(Model),model_max);
    if (!gf3d_model.model_list) {
        slog("cannot create model list");
        return;
    }
    gf3d_model.device = gf3d_vgraphics_get_default_logical_device();
    if (gf3d_model.device) {
        slog("something model device");
    }

    gf3d_mesh_init(model_max);
    gf3d_model.model_max = model_max;
    
    if (gf3d_mesh_get_bind_description() == NULL) {
        slog("something wrong");
            return;
    }
    if (gf3d_mesh_get_attribute_descriptions(NULL) == NULL) {
        slog("something wrong 2");
        return;
    }

    slog("we did it jo");
    slog_sync();

    
    /*
    gf3d_model.pipe = gf3d_pipeline_create_from_config(
        gf3d_model.device,
        "config/model_pipeline.cfg",
        gf3d_vgraphics_get_view_extent(),
        model_max,
        gf3d_mesh_get_bind_description(),
        gf3d_mesh_get_attribute_descriptions(NULL),
        MESH_ATTRIBUTE_COUNT,
        sizeof(ModelUBO),
        VK_INDEX_TYPE_UINT16
    );     
    */
    VkVertexInputBindingDescription* binding =
        gf3d_mesh_get_bind_description();

    VkVertexInputAttributeDescription* attributes =
        gf3d_mesh_get_attribute_descriptions(NULL);

    VkExtent2D extent = gf3d_vgraphics_get_view_extent();

    slog("device: %p", (void*)gf3d_model.device);
    slog("binding: %p", (void*)binding);
    slog("attributes: %p", (void*)attributes);
    slog("extent: %u x %u", extent.width, extent.height);
    slog("size: %p", sizeof(ModelUBO));
    slog_sync();

    SJson* test = sj_load("config/model_pipeline.cfg");

    if (!test) {
        slog("FAILED to load config/model_pipeline.cfg");
    }
    else {
        slog("Successfully loaded model pipeline config");
        sj_free(test);
    }

    slog_sync();

    gf3d_model.pipe = gf3d_pipeline_create_from_config(
        gf3d_model.device,
        "config/model_pipeline.cfg",
        extent,
        model_max,
        binding,
        attributes,
        3,
        sizeof(ModelUBO),
        VK_INDEX_TYPE_UINT16
    );

    
    
    if (gf3d_model.pipe) {
        slog("no issue w pipe");
    }
    slog_sync();
    slog("model manager initiliazed");
    atexit(gf3d_model_close);
}

void gf3d_model_close(){
    int i;
    for (i = 0; i < gf3d_model.model_max; i++)
    {
        gf3d_model_delete(&gf3d_model.model_list[i]);
    }
    if (gf3d_model.model_list)
    {
        free(gf3d_model.model_list);
    }

    memset(&gf3d_model, 0, sizeof(ModelManager));
    slog("model manager closed");
}

void gf3d_model_free(Model *model){
    if (!model) return;

    model->refcount--;

    if (model->refcount <= 0)gf3d_model_delete(model);
}

void gf3d_model_delete(Model *model)
{
    if (!model)return;
    
    if (model->buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(gf3d_model.device, model->buffer, NULL);
    }
    if (model->bufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf3d_model.device, model->bufferMemory, NULL);
    }

    gf3d_texture_free(model->texture);
    gf3d_mesh_free(&model->mesh);
    memset(model,0,sizeof(Model));
}


/**
 * @brief get a pointer to a free sprite
 * @note this provides a pointer to a zero initialized sprite, no other work is done
 * @return NULL on error or an empty sprite otherwise
 */
Model *gf3d_model_new(){
    int i;

    for (i = 0; i < gf3d_model.model_max; i++)
    {
        if (gf3d_model.model_list[i].refcount == 0 && (strlen(gf3d_model.model_list[i].filename) == 0)) {
            gf3d_model.model_list[i].refcount = 1;
            return &gf3d_model.model_list[i];
        }
    }
    for (i = 0; i < gf3d_model.model_max; i++) {
        if (gf3d_model.model_list[i].refcount == 0) {
            if (strlen(gf3d_model.model_list[i].filename) > 0) {
                gf3d_model_delete(&gf3d_model.model_list[i]);
            }
            gf3d_model.model_list[i].refcount = 1;
            return &gf3d_model.model_list[i];
        }
    }
    slog("no free slots for new model");
    return NULL;
}

/**
 * @brief loads a sprite sheet into memory
 * @param filename the name of the file containing the image data
 * @param frame_width how wide an individual frame is on the sprite sheet.  if <= 0 this is assumed to be the image size
 * @param frame_height how high an individual frame is on the sprite sheet.  if <= 0 this is assumed to be the image size
 * @param frames_per_line how many frames across are on the sprite sheet
 * @return NULL on error (check logs) or a pointer to a sprite that can be draw to the 2d overlay
 */
Model * gf3d_model_load(const char * filename){
    Model      *model;
    SJson      *json;
    SJson      *data;
    Mesh       *mesh;
    Texture    *texture;
    const char *str = NULL;
    const char *str2 = NULL;

    if (!filename) {
        slog("missing file");
        return NULL;
    }
    model = gf3d_get_model_by_filename(filename);
    if (model) {
        slog("got your model");
        model->refcount++;
        return model;
    }

    json = sj_load(filename);
    if (!json){
        slog("failed model load");
        return NULL;
    }
    data = sj_object_get_value(json, "model");
    if (!data) {
        slog("failed to get model data");
        sj_free(json);
        return NULL;
    }
    slog_sync();
    str = sj_object_get_string( data, "obj" );
    if (!str){
        slog("failed to find obj data");
        sj_free(json);
        return NULL;
    }
    mesh = gf3d_mesh_load_obj(str);
    if (!mesh) {
        slog("no meshies");
        sj_free(json);
        return NULL;
    }
    str2 = sj_object_get_string( data, "texture");
    if (str2){
        texture = gf3d_texture_load(str2);
        slog("TEXTURE IS HERE");
    } else {
        texture = gf3d_model.defaultTexture;
        slog("failed to find model texture. default texture enabled");
    }
    model = gf3d_model_new();
    if (!model) {
        slog("failed to find model from filename");
        return NULL;
    }
    model->mesh = mesh;
    model->texture = texture;
    gfc_line_cpy(model->filename,filename);
    slog("successfully loaded model?");
    slog_sync();
    sj_free(json);
    return model;
}

Model* gf3d_get_model_by_filename(const char *filename){
    int i;
    if (!filename) {
        slog("no filename");
        return NULL;
    }
    for (i = 0; i < gf3d_model.model_max; i++)
    {
        if (gf3d_model.model_list[i].refcount == 0) {
            continue;
        }
        if (gfc_strlcmp(gf3d_model.model_list[i].filename, filename) == 0)
        {
            return &gf3d_model.model_list[i];
        }
    }
    return NULL;
}

ModelUBO gf3d_model_get_ubo(GFC_Matrix4 modelMat, GFC_Color color)
{
    ModelUBO ubo = {0};
    GFC_Matrix4 *view;

    gfc_matrix4_copy(ubo.modelMat, modelMat);

    view = gf3d_vgraphics_get_view_matrix();
    if (view) {
        gfc_matrix4_copy(ubo.view, *view);
    }
    else {
        slog("something wrong copying matrix");
    }

    gf3d_vgraphics_get_projection_matrix(&ubo.proj);

    ubo.color = gfc_color_to_vector4f(color);

    return ubo;
}

void gf3d_model_queue_render(Model* model, GFC_Matrix4 mat, GFC_Color color) {
    if (!model) return;

    ModelUBO ubo = gf3d_model_get_ubo(mat, color);

    if (!model->mesh) {
        slog("1");
        return;
    }
    if (!gf3d_model.pipe) {
        slog("2");
        return;
    }
    if (!&ubo) {
        slog("3");
        return;
    }
    if (!model->texture) {
        slog("4");
        return;
    }

    gf3d_mesh_queue_render(model->mesh, gf3d_model.pipe, &ubo, model->texture);
    //gf3d_mesh_queue_render(Mesh * mesh, Pipeline * pipe, void* uboData, Texture * texture)
}