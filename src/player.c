#include "simple_logger.h"
#include "player.h"

Entity* player_spawn(GFC_Vector3D position)
{
    PlayerEntityData* data;
    Entity* self;
    self = entity_new();
    data = gfc_allocate_array(sizeof(PlayerEntityData),1);
    if (!self) return NULL;

    self->model = gf3d_model_load("models/dino/dino.model");
    self->draw = player_draw;
    self->update = player_update;
    self->think = player_think;
    self->position = position;
    self->data = data;

    return self;
}

int player_think(Entity *self)
{
    if (!self || !self->_inuse) return 0;

    self->position.z = sin(SDL_GetTicks());
    return 1;
}

int player_draw(Entity *self)
{
    if (!self || !self->_inuse) return 0;
    if (!self->model) {
        slog("model issue player");
        return 0;
    }
    if (!self->mat) {
        slog("matrix issue player");
        return 0;
    }
    gfc_matrix4_from_vectors(self->mat, self->position, self->rotation, self->scaltion);
    gf3d_model_queue_render(self->model, self->mat, self->colorMod);
    slog("we're so up drawing");
    return 1;
}

int player_update(Entity *self)
{
    if (!self || !self->_inuse) return 0;
    slog("we're so up updating");
    return 1;
}

void player_free(Entity* self)
{
    if (!self || !self->data) return;
    PlayerEntityData *data = (PlayerEntityData*)self->data;
    free(data);
    self->data = NULL;
}