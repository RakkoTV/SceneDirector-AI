/*
 * Scene Predictor Header
 * Copyright (c) 2026 Ramiro Silva
 */

#pragma once

#include "scene-director-ai.hpp"

#ifdef __cplusplus
extern "C" {
#endif

double predict_scene_switch(struct scene_director_data *data);
void train_prediction_model(struct scene_director_data *data);

#ifdef __cplusplus
}
#endif
