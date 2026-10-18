#ifndef __GF3D_MODEL_H__
#define __GF3D_MODEL_H__
 
#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_types.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_text.h"
#include "gf3d_mesh.h"


#include "gf3d_pipeline.h"
#include "gf3d_texture.h"

typedef struct
{
    GFC_Matrix4     modelMat;
    GFC_Matrix4     view;
    GFC_Matrix4     proj;
    GFC_Vector4D    color;
}ModelUBO;
typedef struct
{
    int                       refcount;
    GFC_TextLine                filename;               /**<the name of the file used to create the sprite*/
    Mesh                       *mesh;
    Texture                    *texture;                /**<texture memory pointer*/
    VkBuffer                    buffer;
    VkDeviceMemory              bufferMemory;
    VkDescriptorSet            *descriptorSet;          /**<descriptor sets used for this sprite to render*/
    SDL_Surface                *surface;                /**<pointer to the cpu surface data*/
}Model;

/**
 * @brief initialize the internal management system for sprites, auto-cleaned up on program exit
 * @param max_models how many concurrent sprites to support
 */
void gf3d_model_manager_init(Uint32 max_models);

/**
 * @brief get a pointer to a free sprite
 * @note this provides a pointer to a zero initialized sprite, no other work is done
 * @return NULL on error or an empty sprite otherwise
 */
Model *gf3d_model_new();


Model *gf3d_get_model_by_filename(const char* filename);


Model *gf3d_model_parse(SJson *json);

/**
 * @brief create the internal support for rendering the sprite.
 * @note automatically called for loaded sprites, but any created must call this before it will render properly
 * @param sprite the sprite to setup
 */
void gf3d_model_create_vertex_buffer(Model *model);


/**
 * @brief free a previously loaded sprite
 * @param sprite a pointer to the sprite to be freed
 */
void gf3d_model_free(Model *model);

void gf3d_model_delete(Model* model);

Model* gf3d_model_load(const char* filename);

/**
 * @brief get the default pipeline for overlay rendering
 * @return NULL on error or not yet initlialized, the pipeline otherwise
 */
Pipeline *gf3d_model_get_pipeline();

/**
 * @brief get the binding description for a sprite
 */
VkVertexInputBindingDescription * gf3d_model_get_bind_description();

VkVertexInputAttributeDescription * gf2d_sprite_get_attribute_descriptions(Uint32 *count);

/**
 * @brief called to submit all draw commands to the sprite pipelines
 */
void gf3d_sprite_submit_pipe_commands();

/**
 * @brief queue up a render for the current draw frame
 * @param mesh the mesh to render
 * @param pipe the pipeline to use
 * @param uboData the data to use to draw the mesh
 * @param texture texture data to use
 */
void gf3d_model_queue_render(Model* model, GFC_Matrix4 mat, GFC_Color colorMod);

#endif