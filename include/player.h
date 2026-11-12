#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "gfc_vector.h"
#include "gfc_list.h"
#include "gfc_text.h"
#include "gfc_matrix.h"
#include "gfc_primitives.h"
#include "gfc_input.h"

#include "gf3d_pipeline.h"

#include "gf3d_model.h"
#include "gf3d_mesh.h"
#include "entity.h"

typedef struct {
	float			cameraDistance;
	GFC_Vector2D	cameraRotation;
} PlayerEntityData;

Entity* player_get_the();

Entity *player_spawn(GFC_Vector3D position);

//player think
int player_think(Entity *ent);

//player draw
int player_draw(Entity *ent);

//player update
int player_update(Entity *ent);

int player_free(Entity *ent);

#endif