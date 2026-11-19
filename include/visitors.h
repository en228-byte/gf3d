#ifndef __Visitors_H__
#define __Visitors_H__

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
#include "player.h"

typedef struct {
	char* type;
	char* state;
	int reqNum;
} Visitor;


Entity* visitor_spawn(GFC_Vector3D position);

// the ones that roll over the entity functions

int visitor_think(Entity* ent);

int visitor_update(Entity* ent);

int visitor_free(Entity* ent);

//visitor specific actions

void visitor_request(Entity* self);

char* visitor_rate(Entity* self);

const char setState(const char state);

int isTask(Entity* ent);

int finishedTask(Entity* self);

#endif