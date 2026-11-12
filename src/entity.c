#include "simple_logger.h"
#include "entity.h"
#include "player.h"
#include "level.h"

typedef struct
{
	Entity	*entityList;
	Uint32	 maxEnts;
} EntityManager;

static EntityManager entity_manager = { 0 };
void entity_close();

void entity_init(Uint32 maxEnts)
{
	if (!maxEnts) {
		slog("cannot initialize with no maxEnts");
		return;
	}

	if (entity_manager.maxEnts != 0) {
		slog("cannot initialize more than once");
		return;
	}

	entity_manager.entityList = gfc_allocate_array(sizeof(Entity), maxEnts);
	if (!entity_manager.entityList) {
		slog("failed to allocate array space for entityList");
		return;
	}

	entity_manager.maxEnts = maxEnts;
	slog("entity system initialzed");
	atexit(entity_close);
}

void entity_close()
{
	int i;

	for (i = 0; i < entity_manager.maxEnts; i++) {
		if (entity_manager.entityList[i]._inuse) entity_free(&entity_manager.entityList[i]);
	}
	free(&entity_manager);
	memset(&entity_manager, 0, sizeof(EntityManager));
}

//get a blank entity from the system, null on error out of memory, valid pointer otherwise
Entity* entity_new()
{
	int i;
	for (i = 0; i < entity_manager.maxEnts; i++)
	{
		if (entity_manager.entityList[i]._inuse)continue;
		entity_manager.entityList[i]._inuse = 1;

		entity_manager.entityList[i].colorMod = GFC_COLOR_WHITE;
		entity_manager.entityList[i].scaltion = gfc_vector3d(1, 1, 1);

		return &entity_manager.entityList[i];
	}
	slog("could not create new entity");
	return NULL;
}

void entity_free(Entity *ent)
{
	if (!ent || !ent->_inuse) return;
	if (ent->free) {
		if (ent->free(ent) == 1) {
			slog("clean up custom data if its there");
			return;
		}
	}
	gf3d_model_free(ent->model);
	memset(ent, 0, sizeof(Entity));
}

//run update on all active entities
void entity_system_update()
{
	if (!entity_manager.maxEnts) return;
	int i;
	for (i = 0; i < entity_manager.maxEnts; i++)
	{
		if (entity_manager.entityList[i]._inuse){
		this_entity_update(&entity_manager.entityList[i]);
		}
	}
}

//run think function on all active entities
void entity_system_think()
{
	if (!entity_manager.maxEnts) return;
	int i;
	for (i = 0; i < entity_manager.maxEnts; i++)
	{
		if (entity_manager.entityList[i]._inuse) {
		this_entity_think(&entity_manager.entityList[i]);
		}
	
	}
}

//queue renders on all active entities
void entity_system_draw()
{
	if (!entity_manager.maxEnts) return;
	int i;
	for (i = 0; i < entity_manager.maxEnts; i++)
	{
		if (entity_manager.entityList[i]._inuse) {
		this_entity_draw(&entity_manager.entityList[i]);
		}
	}
}

void this_entity_update(Entity* self)
{
	if (!self) return;
	if (!self->_inuse) return;

	if (self->update) {
		if (self->update(self) == 1) {
			return;
		}
	}
	//slog("we made it to the end of updating");
}

void this_entity_think(Entity* self)
{
	if (!self) return;
	if (!self->_inuse) return;

	if (self->think) {
		if (self->think(self) == 1) {
			return;
		}
	}
	if (self->collider) {
		if (entity_collision > 0) slog("reaching out...touching me...touching you");
	}
	slog_sync();
}

void this_entity_draw(Entity* self)
{
	if (!self) return;
	if (!self->_inuse) return;

	if (self->draw) {
		if (self->draw(self) == 1) {
			return;
		}
	}
	else {
		if (!self->model) {
			slog("model issue entity");
			return;
		}
		if (!self->mat) {
			slog("matrix issue entity");
			return;
		}
		gfc_matrix4_from_vectors(self->mat, self->position, self->rotation, self->scaltion);
		gf3d_model_queue_render(self->model, self->mat, self->colorMod);
	}
	//slog("we made it to the end of drawing");
}

Uint8 entity_collision(Entity* self) {
	if (!self || !player_get_the()) return;

	GFC_Box a, b;
	a = self->bounds;
	b = player_get_the()->bounds;
	

	gfc_vector3d_add(a, a, self->position);
	gfc_vector3d_add(b, b, player_get_the()->position);
	return gfc_box_overlap(a, b);
}

int entity_move_check(Entity* self, GFC_Vector3D offset) {
	GFC_Box aabb;

	if (!self) return 0;

	aabb = self->bounds;

	gfc_vector3d_add(aabb, self->bounds, self->position);
	gfc_vector3d_add(aabb, aabb, offset);
	if (!level_bounds_check(level_get_the(), aabb)) {
		slog("made it");
		slog_sync();
		return 1;
	}
	else {
		slog("we aint make jack skippy");
	}
	return 0;
}

/*eol@eof*/