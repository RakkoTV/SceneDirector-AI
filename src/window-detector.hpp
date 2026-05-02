/*
 * Window Detector Header
 * Copyright (c) 2026 Ramiro Silva
 */

#pragma once

#include "scene-director-ai.hpp"

#ifdef __cplusplus
extern "C" {
#endif

bool detect_windows(struct scene_director_data *data);
bool match_window_pattern(const char *title, const char *pattern);

#ifdef __cplusplus
}
#endif
