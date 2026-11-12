#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "gfc_vector.h"
#include "gfc_list.h"
#include "gfc_text.h"
#include "gfc_matrix.h"
#include "gfc_primitives.h"

#include "gf3d_pipeline.h"

#include "gf3d_model.h"
#include "gf3d_mesh.h"
#include "entity.h"
#include "gfc_audio.h"

typedef struct
{
    //Model          *terrain;
    //Model          *skybox;
    Entity         *world;
    GFC_Box         bounds; //dont let anything leave
    GFC_TextLine    skyFile;
    GFC_TextLine    terrainFile;
    GFC_TextLine    musicFile;
    SJson          *json;
    Mix_Music      *backgroundMusic;
} Level;

Level* level_get_the();

Level* level_new();

Level* level_load(const char* filename);

void level_free(Level* level);

void level_draw(Level* level);

Uint8 level_bounds_check(Level* level, GFC_Box aabb);

#endif