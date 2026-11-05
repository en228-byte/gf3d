#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_vector.h"
#include "gfc_list.h"
#include "gfc_text.h"
#include "gfc_matrix.h"
#include "gfc_primitives.h"

#include "gf3d_pipeline.h"

#include "gf3d_model.h"
#include "gf3d_mesh.h"

typedef struct Entity_S
{
	Uint8			_inuse;
	Model			*model;
	GFC_Matrix4		mat;
	GFC_Color		colorMod;
	GFC_Vector3D	position;
	GFC_Vector3D	rotation;
	GFC_Vector3D	scaltion; //scale, but i think its funny that kehoe wanted them all to match
	GFC_Vector3D	velocity;

	int (*think)(struct Entity_S* self);
	int (*update)(struct Entity_S* self);
	int (*draw)(struct Entity_S* self);
	void(*data);
} Entity;

void entity_init(Uint32 maxEnts);

//get a blank entity from the system, null on error out of memory, valid pointer otherwise
Entity* entity_new();

void entity_free(Entity *ent);

//run update on all active entities
void entity_system_update();

//run think function on all active entities
void entity_system_think();

//queue renders on all active entities
void entity_system_draw();

void this_entity_update(Entity* self);

void this_entity_think(Entity* self);

void this_entity_draw(Entity* self);

#endif // !__ENTITY_H__