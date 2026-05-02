/*
 * SceneDirector-AI for OBS Studio
 * Smart scene switcher with ML-powered prediction
 *
 * Copyright (c) 2026 Ramiro Silva
 * Licensed under GPL-3.0
 *
 * Author: RakkoTV (https://github.com/RakkoTV)
 * Version: 1.0.0
 */

#pragma once

#include <obs-module.h>
#include <util/circlebuf.h>
#include <util/darray.h>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <mutex>

#ifdef __cplusplus
extern "C" {
#endif

// Plugin information
#define PLUGIN_NAME "SceneDirector-AI"
#define PLUGIN_VERSION "1.0.0"
#define PLUGIN_AUTHOR "RakkoTV"

// Maximum number of scenes and rules
#define MAX_SCENES 50
#define MAX_RULES 20
#define MAX_WINDOWS 100

// Switching modes
typedef enum {
    SWITCH_MODE_WINDOW,
    SWITCH_MODE_AUDIO,
    SWITCH_MODE_TIME,
    SWITCH_MODE_MANUAL,
    SWITCH_MODE_PREDICTION
} switch_mode_t;

// Rule conditions
typedef enum {
    CONDITION_WINDOW_TITLE,
    CONDITION_WINDOW_CLASS,
    CONDITION_AUDIO_LEVEL,
    CONDITION_TIME_RANGE,
    CONDITION_VIEWER_COUNT,
    CONDITION_CUSTOM
} condition_type_t;

// Scene rule
struct scene_rule {
    uint64_t id;
    char scene_name[256];
    condition_type_t condition;
    char pattern[512];
    double threshold;
    time_t start_time;
    time_t end_time;
    bool enabled;
    int priority;
};

// Window info
struct window_info {
    char title[256];
    char class_name[256];
    bool is_focused;
    uint64_t last_seen;
};

// Plugin state
struct scene_director_data {
    obs_source_t *source;
    obs_hotkey_id next_scene_hotkey;
    obs_hotkey_id prev_scene_hotkey;

    // Settings
    switch_mode_t mode;
    bool enable_prediction;
    int prediction_window;
    double confidence_threshold;
    int switch_delay;
    bool enable_transitions;
    char transition_name[256];
    int transition_duration;

    // Rules
    struct scene_rule rules[MAX_RULES];
    size_t rule_count;

    // Window detection
    struct window_info windows[MAX_WINDOWS];
    size_t window_count;
    uint64_t last_window_check;

    // Audio monitoring
    float audio_levels[MAX_AUDIO_MIXES];
    uint64_t last_audio_check;

    // ML prediction
    struct circlebuf history_buffer;
    std::vector<double>* pattern_weights;
    time_t last_prediction;

    // WebSocket server
    int ws_port;
    bool ws_enabled;
    void* ws_server;

    // Current state
    char current_scene[256];
    char target_scene[256];
    uint64_t last_switch_time;
    bool switch_pending;

    // Thread safety
    pthread_mutex_t mutex;
};

// Function declarations
extern const char *sda_get_name(void *type_data);
extern void *sda_create(obs_data_t *settings, obs_source_t *source);
extern void sda_destroy(void *data);
extern void sda_update(void *data, obs_data_t *settings);
extern void sda_activate(void *data);
extern void sda_deactivate(void *data);
extern void sda_tick(void *data, float seconds);
extern void sda_save(void *data, obs_data_t *settings);
extern obs_properties_t *sda_properties(void *data);
extern obs_properties_t *sda_get_properties(void *data);
extern void sda_defaults(obs_data_t *settings);

// Window detection
extern bool detect_windows(struct scene_director_data *data);
extern bool match_window_pattern(const char *title, const char *pattern);

// Audio monitoring
extern void update_audio_levels(struct scene_director_data *data);
extern bool check_audio_condition(struct scene_director_data *data, struct scene_rule *rule);

// Scene prediction
extern double predict_scene_switch(struct scene_director_data *data);
extern void train_prediction_model(struct scene_director_data *data);

// WebSocket interface
extern void start_websocket_server(struct scene_director_data *data);
extern void stop_websocket_server(struct scene_director_data *data);
extern void handle_websocket_message(struct scene_director_data *data, const char *message);

#ifdef __cplusplus
}
#endif
