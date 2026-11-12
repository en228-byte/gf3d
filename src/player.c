#include "simple_logger.h"
#include "player.h"

static Entity* thePlayer;

Entity* player_get_the() {
    return thePlayer;
}

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
    self->free = player_free;
    self->position = position;
    self->bounds = gfc_box(self->position.x, self->position.y, self->position.z, 10, 10, 10);
    self->data = data;

    thePlayer = self;
    return thePlayer;
}

int player_think(Entity *self)
{
    if (!self || !self->_inuse) return 0;

    //movement side to side
    if (gfc_input_command_down("item_left")) {
        if (self->velocity.x > -0.5) {
            self->velocity.x -= 0.01;
        }
    }
    else if (gfc_input_command_down("item_right")) {
        if (self->velocity.x < 0.5) {
            self->velocity.x += 0.01;
        }
    }
    else {
        self->velocity.x = 0;
    }
    //movement forward and back
    if (gfc_input_command_down("item_down")) {
        if (self->velocity.y > -0.5) {
            self->velocity.y -= 0.01;
        }
    }
    else if (gfc_input_command_down("item_up")) {
        if (self->velocity.y < 0.5) {
            self->velocity.y += 0.01;
        }
    }
    else {
        self->velocity.y = 0;
    }

    slog_sync();
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
    return 1;
}

int player_update(Entity *self)
{
    if (!self || !self->_inuse) return 0;

    if (entity_move_check(self, self->velocity) != 0) {
        gfc_vector3d_add(self->position, self->position, self->velocity);
    }
    return 1;
}

int player_free(Entity* self)
{
    if (!self || !self->data) return 0;
    thePlayer = NULL;
    PlayerEntityData *data = (PlayerEntityData*)self->data;
    free(data);
    self->data = NULL;

    return 1;
}