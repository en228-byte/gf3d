#include "simple_logger.h"
#include "entity.h"

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
			slog("we're trying to update personally");
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
			slog("we're trying to think personally");
			return;
		}
	}
	//slog("we made it to the end of drawing");
}

void this_entity_draw(Entity* self)
{
	if (!self) return;
	if (!self->_inuse) return;

	if (self->draw) {
		if (self->draw(self) == 1) {
			slog("we're trying to draw personally");
			return;
		}
	}
	//slog("we made it to the end of drawing");
}
/*eol@eof*/