/*
 * Audio Monitor Header
 * Copyright (c) 2026 Ramiro Silva
 */

#pragma once

#include "scene-director-ai.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void update_audio_levels(struct scene_director_data *data);
bool check_audio_condition(struct scene_director_data *data, struct scene_rule *rule);

#ifdef __cplusplus
}
#endif
