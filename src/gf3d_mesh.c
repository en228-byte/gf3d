#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_types.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_text.h"

#include "gf3d_buffers.h"
#include "gf3d_pipeline.h"
#include "gf3d_vgraphics.h"
#include "gf3d_texture.h"
#include "gf3d_mesh.h"
#include "gf3d_obj_load.h"
#include "gf3d_model.h" 
#include "gf2d_sprite.h"

#define MESH_ATTRIBUTE_COUNT 3

typedef struct
{
    Mesh           *mesh_list;
    Uint32          mesh_max;      /**<maximum concurrent sprites supported*/
    Uint32          chain_length;     /**<length of swap chain*/
    VkDevice        device;           /**<logical vulkan device*/
    VkVertexInputAttributeDescription   attributeDescriptions[MESH_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription     bindingDescription;
}MeshManager;

static MeshManager gf3d_mesh = {0};
void gf3d_mesh_close();



/**
 * @brief initializes the mesh system / configures internal data about mesh based rendering
 * @param mesh_max the maximum allowed simultaneous meshes supported at once.  Must be > 0
 * @note keep in mind that many models will be comprised of multiple sub meshes.  So this number may need to be very large
 */
void gf3d_mesh_init(Uint32 mesh_max)
{
    if (!mesh_max) {
        slog("no mesh max?");
        return;
    }
    if (mesh_max == 0){
        slog("cannot intilizat mesh manager for 0 meshes");
        return;
    }
    if (gf3d_mesh.mesh_max != 0){
        slog("cannot initialize what already exists");
        return;
    }
    //gf3d_mesh.chain_length = gf3d_swapchain_get_chain_length();
    gf3d_mesh.mesh_list = gfc_allocate_array(sizeof(Mesh),mesh_max);
    if (!gf3d_mesh.mesh_list)return;
    gf3d_mesh.mesh_max = mesh_max;
    gf3d_mesh.device = gf3d_vgraphics_get_default_logical_device();
    
    slog("mesh manager initiliazed");
    atexit(gf3d_mesh_close);
}

/**
 * @brief get a new empty model
 * @return NULL on error, or an empty model
 */
Mesh *gf3d_mesh_new()
{
    int i;

    for (i = 0; i < gf3d_mesh.mesh_max; i++)
    {
        slog("test1");
        if ((gf3d_mesh.mesh_list[i].refcount == 0) && (strlen(gf3d_mesh.mesh_list[i].filename) == 0))
        {
            gf3d_mesh.mesh_list[i].primitives = gfc_list_new();
            slog("new 1");

            if (gf3d_mesh.mesh_list[i].primitives == NULL) 
            {
                slog("no free slots for new mesh");
                return NULL;
            }
            gf3d_mesh.mesh_list[i].refcount = 1;
            return &gf3d_mesh.mesh_list[i];
        }
    }
    for (i = 0; i < gf3d_mesh.mesh_max; i++)
    {
        slog("test2");
        if (gf3d_mesh.mesh_list[i].refcount == 0) 
        {
            if (strlen(gf3d_mesh.mesh_list[i].filename) > 0)
            {
                gf3d_mesh_delete(&gf3d_mesh.mesh_list[i]);
            }

            gf3d_mesh.mesh_list[i].primitives = gfc_list_new();
            slog("new 2");

            if (gf3d_mesh.mesh_list[i].primitives == NULL)
            {
                slog("no free slots for new mesh");
                return NULL;
            }
            gf3d_mesh.mesh_list[i].refcount = 1;
            return &gf3d_mesh.mesh_list[i];
         }
     }
    slog("hm");
    return NULL;
}

void gf3d_mesh_close(){
    int i;
    for (i = 0; i < gf3d_mesh.mesh_max; i++)
    {
        gf3d_mesh_delete(&gf3d_mesh.mesh_list[i]);
    }
    if (gf3d_mesh.mesh_list)
    {
        free(gf3d_mesh.mesh_list);
    }

    memset(&gf3d_mesh, 0, sizeof(MeshManager));
    slog("mesh manager closed");
}

/**
 * @brief load mesh data from an obj filename.
 * @note: currently only supporting obj files
 * @note this free's the intermediate data loaded from the obj file, no longer needed for most applications
 * @param filename the name of the file to load
 * @return NULL on error or Mesh data
 */
Mesh *gf3d_mesh_load_obj(const char *filename)
{
    Mesh *mesh;
    MeshPrimitive* prim;
    ObjData* data;

    if (!filename) {
        slog("cannot get this file %s", filename);
        return;
    }
    slog("SEARCHING FOR MESH");
    slog_sync();
    mesh = gf3d_get_mesh_by_filename(filename);

    if (mesh)
    {
        slog("got mesh");
        mesh->refcount++;
        return mesh;
    }
    slog_sync();
    mesh = gf3d_mesh_new();

    if (!mesh)
    {
        slog("no mesh....");
        return NULL;
    }
    slog_sync();
    prim = gf3d_mesh_primitive_new();
    if (!prim) {
        slog("couldn't allocate new prim mesh");
        return NULL;
    }
    slog("mesh and prim?");
    gfc_list_append(mesh->primitives, prim);
    slog_sync();
    data = gf3d_obj_load_from_file(filename);
    if (!data) {
        slog("oh no data");
        gf3d_mesh_delete(mesh);
        gf3d_mesh_primitive_free(prim);
        return NULL;
    }
    slog_sync();
    prim->objData = data;
    if (!prim->objData) {
        gf3d_mesh_delete(mesh);
        slog("failed at getting obj data from file");
        return NULL;
    }
    slog_sync();
    if (!gf3d_mesh_create_vertex_buffer(prim)) {
        gf3d_mesh_delete(mesh);
        slog("failed to create vertex buffers");
        return NULL;
    }

    slog("got your mesh i think");
    slog_sync();
    gfc_line_cpy(mesh->filename, filename);
    return mesh;
}

/**
 * @brief make an exact, but separate copy of the input mesh
 * @param in the mesh to duplicate
 * @return NULL on error, or a copy of in
 */
Mesh *gf3d_mesh_copy(Mesh *in)
{
    if (!in){
        slog("cannot make copy of mesh that does not exist");
        return;
    }
    //make a copy
}

/**
 * @brief move all of the vertices of the mesh by offset at the buffer level
 * @param in the mesh to move
 * @param offset how much to move it
 * @param rotation apply this rotation to the vertices and normals
 */
void gf3d_mesh_move_vertices(Mesh* in, GFC_Vector3D offset, GFC_Vector3D rotation);

/**
 * @brief allocate a zero initialized mesh primitive
 * @return NULL on error or the primitive
 */
MeshPrimitive *gf3d_mesh_primitive_new()
{
    MeshPrimitive*  mesh = gfc_allocate_array(sizeof(MeshPrimitive), 1);

    if (!mesh){
        slog("failed to initialize mesh primitive");
        return NULL;
    }
    return mesh;
}

/**
 * @brief free a mesh that has been loaded from memory
 */
void gf3d_mesh_free(Mesh *mesh)
{
    if (!mesh)return;
    mesh->refcount--;
    if (mesh->refcount <= 0)gf3d_mesh_delete(mesh);
}

void gf3d_mesh_primitive_free(MeshPrimitive *prim){
    if(!prim) return;

    if (prim->vertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(gf3d_mesh.device, prim->vertexBuffer, NULL);
        prim->vertexBuffer = VK_NULL_HANDLE;
    }
    if (prim->faceBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(gf3d_mesh.device, prim->faceBuffer, NULL);
        prim->faceBuffer = VK_NULL_HANDLE;
    }
    if (prim->vertexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(gf3d_mesh.device, prim->vertexBufferMemory, NULL);
        prim->vertexBufferMemory = VK_NULL_HANDLE;
    }
    if (prim->faceBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(gf3d_mesh.device, prim->faceBufferMemory, NULL);
        prim->faceBufferMemory = VK_NULL_HANDLE;
    }
}

void gf3d_mesh_delete(Mesh *mesh)
{
    int i;
    int c;
    MeshPrimitive* prim;

    if (!mesh) return;
    c = gfc_list_count(mesh->primitives);

    for (i = 0; i < c; i++) {
        prim = gfc_list_nth(mesh->primitives, i);
        if (!prim) continue;

        gf3d_mesh_primitive_free(prim);
    }
    memset(mesh, 0, sizeof(Mesh));
}

Mesh *gf3d_get_mesh_by_filename(const char* filename) {
    int i;
    if (!filename)return NULL;

    for (i = 0; i < gf3d_mesh.mesh_max; i++)
    {
        if (!gf3d_mesh.mesh_list[i].refcount)continue;
        if (gfc_line_cmp(gf3d_mesh.mesh_list[i].filename, filename) == 0)
        {
            return &gf3d_mesh.mesh_list[i];
        }
    }
    return NULL;
}

/**
 * @brief get the input attribute descriptions for mesh based rendering
 * @param count (optional, output) the number of attributes
 * @return a pointer to a vertex input attribute description array
 */
VkVertexInputAttributeDescription* gf3d_mesh_get_attribute_descriptions(Uint32* count) {
    if (count)*count = MESH_ATTRIBUTE_COUNT;
    gf3d_mesh.attributeDescriptions[0].binding = 0;
    gf3d_mesh.attributeDescriptions[0].location = 0;
    gf3d_mesh.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    gf3d_mesh.attributeDescriptions[0].offset = offsetof(Vertex, vertex);

    gf3d_mesh.attributeDescriptions[1].binding = 0;
    gf3d_mesh.attributeDescriptions[1].location = 1;
    gf3d_mesh.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    gf3d_mesh.attributeDescriptions[1].offset = offsetof(Vertex, normal);

    gf3d_mesh.attributeDescriptions[2].binding = 0;
    gf3d_mesh.attributeDescriptions[2].location = 2;
    gf3d_mesh.attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    gf3d_mesh.attributeDescriptions[2].offset = offsetof(Vertex, texel);

    if (!gf3d_mesh.attributeDescriptions) {
        slog("att");
        return NULL;
    }
    slog("not att");
    return gf3d_mesh.attributeDescriptions;
}

/**
 * @brief get the binding description for mesh based rendering
 * @return vertex input binding descriptions compatible with mesh data
 */
VkVertexInputBindingDescription* gf3d_mesh_get_bind_description() {
    gf3d_mesh.bindingDescription.binding = 0;
    gf3d_mesh.bindingDescription.stride = sizeof(Vertex);
    gf3d_mesh.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    if (!&gf3d_mesh.bindingDescription) {
        slog("bind");
        return NULL;
    }
    slog("not bind");
    return &gf3d_mesh.bindingDescription;
}

/**
 * @brief get the current command buffer for the mesh system
 */
VkCommandBuffer gf3d_mesh_get_model_command_buffer();

/**
 * @brief queue up a render for the current draw frame
 * @param mesh the mesh to render
 * @param pipe the pipeline to use
 * @param uboData the data to use to draw the mesh
 * @param texture texture data to use
 */
void gf3d_mesh_queue_render(Mesh *mesh,Pipeline *pipe,void *uboData,Texture *texture)
{
    int i,c;
    MeshPrimitive *prim;

    if (!mesh) return;
    if (!pipe) return;
    if (!uboData) return;
    if (!texture) return;
    c = gfc_list_get_count(mesh->primitives);

    for (i = 0; i < c; i++) {
        prim = gfc_list_nth(mesh->primitives,i);
        gf3d_pipeline_queue_render
        (
            pipe,
            prim->vertexBuffer,
            prim->vertexCount,
            prim->faceBuffer,
            uboData,
            texture
        );
    }
}


/**
 * @brief adds a mesh to the render pass rendered as an outline highlight
 * @note: must be called within the render pass
 * @param mesh the mesh to render
 * @param com the command pool to use to handle the request we are rendering with
 */
void gf3d_mesh_render(Mesh *mesh,VkCommandBuffer commandBuffer, VkDescriptorSet * descriptorSet);

/**
 * @brief render a mesh through a given pipeline
 */
void gf3d_mesh_render_generic(Mesh *mesh,Pipeline *pipe,VkDescriptorSet * descriptorSet);

/**
 * @brief create a mesh's internal buffers based on vertices
 * @param primitive the mesh primitive to populate
 * @note the primitive must have the objData set and it must have be organizes in buffer order
 */
void gf3d_mesh_create_vertex_buffer_from_vertices(MeshPrimitive *primitive);

/**
 * @brief given a model matrix and basic color, build the meshUBO needed to render a model
 * @param modelMat the model Matrix
 * @param colorMod the color for the UBO
 */

int gf3d_mesh_create_vertex_buffer(MeshPrimitive* mesh)
{
    void* data = NULL;
    VkDevice device = gf3d_mesh.device;
    size_t bufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if (!mesh) return 0;
    if (!mesh->objData) return 0;
    if (!mesh->objData->face_count) return 0;

    bufferSize = sizeof(Face) * mesh->objData->face_count;
    mesh->faceCount = mesh->objData->face_count;
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, mesh->objData->outFace, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mesh->faceBuffer, &mesh->faceBufferMemory);

    gf3d_buffer_copy(stagingBuffer, mesh->faceBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);


    bufferSize = sizeof(Vertex) * mesh->objData->face_vert_count;
    mesh->vertexCount = mesh->objData->face_vert_count;
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, mesh->objData->faceVertices, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mesh->vertexBuffer, &mesh->vertexBufferMemory);

    gf3d_buffer_copy(stagingBuffer, mesh->vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    return 1;
}