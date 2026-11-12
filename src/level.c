#include "simple_logger.h"
#include "level.h"

static Level* theLevel;

Level* level_get_the(){
    return theLevel;
}

Level* level_new(){
    Level* level;
    level = gfc_allocate_array(sizeof(Level), 1);

    if (!level) {
        slog("couldn't allocate level array");
        return NULL;
    }
    return level;
}

Level* level_load(const char* filename) {
    
    SJson* json;
    SJson* data;
    const char* str;
    Level* level;
    if (!filename) {
        slog("cannot get level filename");
        return NULL;
    }
    json = sj_load(filename);
    if (!json) {
        slog("failed to load file");
        return NULL;
    }

    data = sj_object_get_value(json, "level");
    if (!data) {
        slog("failed to load data from file");
        return NULL;
    }

    level = level_new();
    if (!level) {
        return NULL;
    }

    level->world = entity_new();
    if (!level->world) {
        slog("couldn't get entity world");
        return;
    }
    level->world->model = gf3d_model_load("models/terrain/world.model");
    if (!level->world->model) {
        slog("world model issue");
        return;
    }
    else {
        slog("world model success");
    }
    level->world->collider = true;
    level->world->colorMod = GFC_COLOR_WHITE;
    level->world->position = gfc_vector3d(-50,-40,-50);
    level->world->scaltion = gfc_vector3d(100, 100, 100);

   
    level->bounds = gfc_box_from_config(sj_object_get_value(data, "bounds"));

    str = sj_object_get_string(data, "string");
    if (str) {
        level->backgroundMusic = gfc_sound_load_music(str);
        if (level->backgroundMusic) gfc_line_cpy(level->musicFile, str);
        Mix_PlayMusic(level->backgroundMusic, -1);
    }
    
    level->json = json;
    theLevel = level;
    return theLevel;
}

void level_free(Level* level) {
    entity_free(level->world);
    Mix_HaltMusic;
    Mix_FreeMusic;

    sj_free(level->json);
    free(level);
}

void level_draw(Level* level) {
    GFC_Matrix4 id;

    if (!level)return;

}

Uint8 level_bounds_check(Level* level, GFC_Box aabb) {
    if (!level) return 0;

    if (aabb.x < level->bounds.x) {
        slog("aabbX < levelX");
        return 1;
    }
    if (aabb.y < level->bounds.y) {
        slog("aabbY < levelY");
        return 1;
    }
    if (aabb.z < level->bounds.z) {
        slog("aabbZ < levelZ");
        return 1;
    }

    if (aabb.x + aabb.w > level->bounds.x + level->bounds.w) {
        slog("aabbXMax > levelXMax");
        return 1;
    }
    if (aabb.y + aabb.h > level->bounds.y + level->bounds.h) {
        slog("aabbYMax > levelYMax");
        return 1;
    }
    if (aabb.z + aabb.d > level->bounds.z + level->bounds.d) {
        slog("aabbZMax > levelZMax");
        return 1;
    }


    slog_sync();

    return 0;
}