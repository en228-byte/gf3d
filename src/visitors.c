#include "simple_logger.h"
#include "visitors.h"
#include "string.h"


Entity* visitor_spawn(GFC_Vector3D position)
{
    Visitor* data;
    Entity* self;
    self = entity_new();
    data = gfc_allocate_array(sizeof(Visitor), 1);
    if (!self) return NULL;

    self->model = gf3d_model_load("models/npc/npc.model");
    self->colorMod = GFC_COLOR_BLACK;
    self->update = visitor_update;
    self->think = visitor_think;
    self->free = visitor_free;
    self->position = position;
    self->scaltion = gfc_vector3d(2,2,2);
    self->bounds = gfc_box(self->position.x, self->position.y, self->position.z, 1, 2, 3);
    data->state = "IDLE";
    self->data = data;


    return self;
}

//
// 
// the ones that roll over the entity functions

//visitor think
int visitor_think(Entity* ent)
{
    char* res;
    Visitor* data;

    if (!ent) {
        slog("no entity visitor to think");
        return 0;
    }
    
    data = (Visitor*)ent->data;
    if (!data) {
        data = gfc_allocate_array(sizeof(Visitor), 1);
        if (!data) return 0;

        data->state = "IDLE";
        ent->data = data;
    }
    if (strcmp(data->state, "ASKING") == 0) {
        if (!checkTask(ent)) {
            res = visitor_rate(ent);
            data->state = res;
            return 1;
        }
        else {
            return 1;
        }
    }
    if (strcmp(data->state, "IDLE") == 0 || strcmp(data->state, "ANGRY") == 0) {

    if (entity_collision(ent)) {
        data->state = "ANGRY";
        if (gfc_input_command_down("panright")) {
            slog("need some help");
            data->state = "ASKING";
            visitor_request(ent);
        }
    }
    else {
        data->state = "IDLE";
    }
    }

    return 0;
}

//visitor update
int visitor_update(Entity* ent)
{
    Visitor* visData;
    PlayerEntityData* playerData;
    char* curState;

    if (!ent || !ent->data || !ent->model) return 0;

    visData = (Visitor*)ent->data;
    if (!visData->state) return 0;

    curState = visData->state;

    if (strcmp(curState, "ASKING") == 0) {
        ent->model->texture = gf3d_texture_load("models/npc/teal.png");
    }
    else if (strcmp(curState, "IDLE") == 0 || strcmp(curState, "LEAVING") == 0) {
        ent->model->texture = gf3d_texture_load("models/npc/grey.png");
    }
    else if (strcmp(curState, "ANGRY") == 0) {
        ent->model->texture = gf3d_texture_load("models/npc/red.jpg");
    }
    else if (strcmp(curState, "UPSET") == 0) {
        ent->model->texture = gf3d_texture_load("models/npc/orange.png");
    }
    else if (strcmp(curState, "NEUTRAL") == 0) {
        ent->model->texture = gf3d_texture_load("models/npc/yellow.jpg");
    }
    else if (strcmp(curState, "HAPPY") == 0) {
        ent->model->texture = gf3d_texture_load("models/npc/green.jpg");
    }
    else if (strcmp(curState, "ELATED") == 0) {
        ent->model->texture = gf3d_texture_load("models/npc/pink.jpg");
    }
    else {
        slog("colors dont exist ig");
    }
    return 1;
}

//visitor free
//free visitor specific data
int visitor_free(Entity* self)
{
    if (!self || !self->data) return 0;
    Visitor* data = (Visitor*)self->data;
    free(data);
    self->data = NULL;

    return 1;
}

//
// 
//visitor specific actions

//visitor request from librarian
void visitor_request(Entity* self)
{
    Visitor* data;
    PlayerEntityData* pd;
    Entity* player;

    if (!self || !self->data) return;

    data = (Visitor*)self->data;
    player = player_get_the();
    if (!data || !player || !player->data) return;

    pd = (PlayerEntityData*)player->data;

    data->reqNum = pd->curTaskNum;

    pd->task = "Get me a book by pressing SPACE FIVE TIMES";
}

//visitor rate librarian
char* visitor_rate(Entity* self)
{
    int grade;
    Visitor* data;
    PlayerEntityData* pd;
    Entity* player;

    if (!self || !self->data) return;

    data = (Visitor*)self->data;
    player = player_get_the();
    if (!data || !player || !player->data) return;

    pd = (PlayerEntityData*)player->data;

    grade = 100 - ( (pd->curTaskNum - data->reqNum) * 10 );

    if (grade >= 90) {
        return "ELATED";
    }
    else if (grade >= 80) {
        return "HAPPY";
    }
    else if (grade >= 70) {
        return "NEUTRAL";
    }
    else if (grade >= 60) {
        return "UPSET";
    }
    else {
        return "ANGRY";
    }
    data->reqNum = -1;
}

const char setState(const char state)
{
    return state;
}

int checkTask(Entity* ent) {
    Visitor* data;

    data = (Visitor*)ent->data;
    
    if (!ent || !data) return 0;

    if (data->reqNum > 0) {
        return 1;
    }

    return 0;
}