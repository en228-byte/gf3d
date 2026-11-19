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
        finishedTask(ent);
        if (gfc_input_command_down("pandown")) {
            slog("DID IT");
            completeTask(data->reqNum);
        }
    }
    if (strcmp(data->state, "IDLE") == 0 || strcmp(data->state, "ANGRY") == 0) {

    if (entity_collision(ent)) {
        data->state = "ANGRY";
        slog("im not touching you");
        if (gfc_input_command_down("panright")) {
            slog("need some help");
            slog_sync();
            data->state = "ASKING";
            visitor_request(ent);
        }
    }
    else {
        data->state = "IDLE";
    }
    slog_sync();
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
    Task* thisTask;

    thisTask = gfc_allocate_array(sizeof(Task), 1);

    if (!self || !self->data) return;

    data = (Visitor*)self->data;
    player = player_get_the();
    if (!data || !player || !player->data) return;

    pd = (PlayerEntityData*)player->data;

    data->reqNum = pd->curTaskNum;

    thisTask->completed = false;
    thisTask->taskID = data->reqNum;

    addTask(thisTask);
    slog_sync();
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
    if (pd->satisfaction) {
        pd->satisfaction += grade;
    }
    else {
        pd->satisfaction = grade;
    }
}

const char setState(const char state)
{
    return state;
}

int isTask(Entity* ent) {
    Visitor* data;

    data = (Visitor*)ent->data;
    
    if (!ent || !data) return 0;

    if (data->reqNum > 0) {
        return 1;
    }

    return 0;
}

int finishedTask(Entity* self) {
    Visitor* data;
    PlayerEntityData* pd;
    Entity* player;

    if (!self || !self->data) return;

    data = (Visitor*)self->data;
    player = player_get_the();
    if (!data || !player || !player->data) return 0;

    pd = (PlayerEntityData*)player->data;

    if (!pd->tasks[data->reqNum].completed) return 0;

    data->reqNum = -1;
    data->state = "IDLE";

    data->state = visitor_rate(self);
    slog("%s", data->state);
}