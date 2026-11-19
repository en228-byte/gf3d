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
    self->update = player_update;
    self->think = player_think;
    self->scaltion = gfc_vector3d(2.3, 2.3, 2.3);
    self->colorMod = GFC_COLOR_BLACK;
    self->free = player_free;
    self->position = position;
    self->bounds = gfc_box(self->position.x, self->position.y, self->position.z, 10, 10, 10);
    data->updradeLevel = 0;
    self->data = data;

    thePlayer = self;
    return thePlayer;
}

int player_think(Entity *self)
{
    if (!self || !self->_inuse) return 0;

    //cheats
    if (gfc_input_command_released("nextelement")) {
        Entity* testNPC;
        testNPC = visitor_spawn(gfc_vector3d(self->position.x + 10, self->position.y + 10, -1));

        slog("we doin this");
    }
    if (gfc_input_command_released("panup")) {
        completeTask(self);
    }
    if (gfc_input_command_released("build")) upgradeLibrary(self);

    //movement side to side
    if (gfc_input_command_down("item_left")) {
        if (self->velocity.x > -0.5) {
            self->velocity.x -= 0.001;
        }
    }
    else if (gfc_input_command_down("item_right")) {
        if (self->velocity.x < 0.5) {
            self->velocity.x += 0.001;
        }
    }
    else {
        self->velocity.x = 0;
    }
    //movement forward and back
    if (gfc_input_command_down("item_down")) {
        if (self->velocity.y > -0.5) {
            self->velocity.y -= 0.0025;
        }
    }
    else if (gfc_input_command_down("item_up")) {
        if (self->velocity.y < 0.5) {
            self->velocity.y += 0.0025;
        }
    }
    else {
        self->velocity.y = 0;
    }

    slog_sync();
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

void upgradeLibrary(Entity* self) {
    PlayerEntityData* data = (PlayerEntityData*)self->data;
    if (!data) {
        slog("wrong");
        return;
    }

    GFC_Vector3D position;
    position.x = (data->updradeLevel - 1) * 10;
    position.y = ((data->updradeLevel / 3) * 10) + 20;
    position.z = 0;
        Entity* shelf;
        shelf = entity_new();
        if (!shelf) return;

        shelf->model = gf3d_model_load("models/shelf.model");
        shelf->colorMod = GFC_COLOR_WHITE;
        shelf->position = position;
        shelf->scaltion = gfc_vector3d(10,2,10);
        shelf->bounds = gfc_box(shelf->position.x, shelf->position.y, shelf->position.z, 1,1,1);

        data->updradeLevel += 1;
}

void addTask(Task* task) {
    Entity* player = player_get_the();
    PlayerEntityData* data = (PlayerEntityData*)player->data;

    if (!player || !data) return;

    if (data->taskCount >= 100)return;

    data->tasks[data->taskCount].taskID = task->taskID;
    data->tasks[data->taskCount].completed = task->completed;
    slog("%i", data->tasks[data->taskCount].taskID);
    slog_sync();
    data->taskCount++;
}

void completeTask(int id) {
    PlayerEntityData* data = (PlayerEntityData*)player_get_the()->data;
    int counter;
    counter = 0;

    if (!data) return;

    if (!data->tasks[id].completed) {
            data->tasks[id].completed = true;
            data->taskCount--;
    }
}