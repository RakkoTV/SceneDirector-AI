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

#include "scene-director-ai.hpp"
#include "window-detector.hpp"
#include "audio-monitor.hpp"
#include "websocket-server.hpp"
#include "scene-predictor.hpp"

#include <cstring>
#include <algorithm>
#include <cmath>

// Localization strings
static const char *sda_strings[] = {
    // English
    "SceneDirector-AI",
    "Smart scene switcher with AI-powered prediction",
    "Switching Mode",
    "Window Focus",
    "Audio Level",
    "Time Schedule",
    "Manual",
    "AI Prediction",
    "Enable Prediction",
    "Prediction Window (seconds)",
    "Confidence Threshold (%)",
    "Switch Delay (ms)",
    "Enable Transitions",
    "Transition",
    "Transition Duration (ms)",
    "Rules",
    "Add Rule",
    "Remove Rule",
    "Scene",
    "Condition",
    "Pattern/Value",
    "Threshold",
    "Priority",
    "Enabled",
    "Next Scene",
    "Previous Scene",
    "WebSocket Server",
    "Enable WebSocket",
    "Port",
    // Spanish
    "SceneDirector-AI",
    "Cambio inteligente de escena con predicción IA",
    "Modo de Cambio",
    "Ventana Activa",
    "Nivel de Audio",
    "Horario",
    "Manual",
    "Predicción IA",
    "Habilitar Predicción",
    "Ventana de Predicción (segundos)",
    "Umbral de Confianza (%)",
    "Retraso de Cambio (ms)",
    "Habilitar Transiciones",
    "Transición",
    "Duración de Transición (ms)",
    "Reglas",
    "Agregar Regla",
    "Eliminar Regla",
    "Escena",
    "Condición",
    "Patrón/Valor",
    "Umbral",
    "Prioridad",
    "Habilitado",
    "Escena Siguiente",
    "Escena Anterior",
    "Servidor WebSocket",
    "Habilitar WebSocket",
    "Puerto",
    nullptr
};

// =============================================================================
// PLUGIN CORE FUNCTIONS
// =============================================================================

const char *sda_get_name(void *type_data)
{
    UNUSED_PARAMETER(type_data);
    return obs_module_text("SceneDirector-AI");
}

void *sda_create(obs_data_t *settings, obs_source_t *source)
{
    struct scene_director_data *data = new scene_director_data();
    memset(data, 0, sizeof(struct scene_director_data));

    data->source = source;
    data->mode = SWITCH_MODE_WINDOW;
    data->enable_prediction = true;
    data->prediction_window = 30;
    data->confidence_threshold = 75.0;
    data->switch_delay = 500;
    data->enable_transitions = true;
    data->transition_duration = 300;
    data->ws_port = 8080;
    data->ws_enabled = false;

    pthread_mutex_init(&data->mutex, nullptr);

    // Initialize history buffer for ML prediction
    circlebuf_init(&data->history_buffer);
    data->pattern_weights = new std::vector<double>();

    sda_update(data, settings);

    blog(LOG_INFO, "[SceneDirector-AI] Plugin created");

    return data;
}

void sda_destroy(void *data)
{
    struct scene_director_data *ctx = (struct scene_director_data *)data;

    if (!ctx)
        return;

    stop_websocket_server(ctx);

    pthread_mutex_destroy(&ctx->mutex);
    circlebuf_free(&ctx->history_buffer);
    delete ctx->pattern_weights;

    delete ctx;

    blog(LOG_INFO, "[SceneDirector-AI] Plugin destroyed");
}

void sda_update(void *data, obs_data_t *settings)
{
    struct scene_director_data *ctx = (struct scene_director_data *)data;

    pthread_mutex_lock(&ctx->mutex);

    // Load settings
    ctx->mode = (switch_mode_t)obs_data_get_int(settings, "mode");
    ctx->enable_prediction = obs_data_get_bool(settings, "enable_prediction");
    ctx->prediction_window = obs_data_get_int(settings, "prediction_window");
    ctx->confidence_threshold = obs_data_get_double(settings, "confidence_threshold");
    ctx->switch_delay = obs_data_get_int(settings, "switch_delay");
    ctx->enable_transitions = obs_data_get_bool(settings, "enable_transitions");
    ctx->ws_enabled = obs_data_get_bool(settings, "ws_enabled");
    ctx->ws_port = obs_data_get_int(settings, "ws_port");

    const char *transition = obs_data_get_string(settings, "transition");
    if (transition)
        strncpy(ctx->transition_name, transition, sizeof(ctx->transition_name) - 1);

    ctx->transition_duration = obs_data_get_int(settings, "transition_duration");

    // Restart WebSocket server if settings changed
    if (ctx->ws_enabled && !ctx->ws_server) {
        start_websocket_server(ctx);
    } else if (!ctx->ws_enabled && ctx->ws_server) {
        stop_websocket_server(ctx);
    }

    pthread_mutex_unlock(&ctx->mutex);
}

void sda_activate(void *data)
{
    struct scene_director_data *ctx = (struct scene_director_data *)data;
    pthread_mutex_lock(&ctx->mutex);
    ctx->last_window_check = 0;
    ctx->last_audio_check = 0;
    ctx->last_prediction = 0;
    pthread_mutex_unlock(&ctx->mutex);

    blog(LOG_INFO, "[SceneDirector-AI] Activated");
}

void sda_deactivate(void *data)
{
    blog(LOG_INFO, "[SceneDirector-AI] Deactivated");
}

// =============================================================================
// MAIN TICK FUNCTION - Core Logic
// =============================================================================

void sda_tick(void *data, float seconds)
{
    struct scene_director_data *ctx = (struct scene_director_data *)data;
    if (!ctx) return;

    uint64_t now = os_gettime_ns();
    uint64_t tick_interval = (uint64_t)(seconds * 1000000000ULL);

    pthread_mutex_lock(&ctx->mutex);

    // Update window detection every 500ms
    if (now - ctx->last_window_check > 500000000ULL) {
        detect_windows(ctx);
        ctx->last_window_check = now;
    }

    // Update audio levels every 100ms
    if (now - ctx->last_audio_check > 100000000ULL) {
        update_audio_levels(ctx);
        ctx->last_audio_check = now;
    }

    // Run ML prediction every second if enabled
    if (ctx->enable_prediction && now - ctx->last_prediction > 1000000000ULL) {
        double confidence = predict_scene_switch(ctx);
        ctx->last_prediction = now;

        if (confidence >= ctx->confidence_threshold) {
            // Pending switch based on prediction
            ctx->switch_pending = true;
        }
    }

    // Evaluate rules and switch scenes
    bool switched = false;
    struct scene_rule *best_rule = nullptr;
    int best_priority = -1;

    for (size_t i = 0; i < ctx->rule_count; i++) {
        struct scene_rule *rule = &ctx->rules[i];

        if (!rule->enabled)
            continue;

        if (rule->priority <= best_priority)
            continue;

        bool match = false;

        switch (rule->condition) {
        case CONDITION_WINDOW_TITLE:
            for (size_t j = 0; j < ctx->window_count; j++) {
                if (ctx->windows[j].is_focused &&
                    match_window_pattern(ctx->windows[j].title, rule->pattern)) {
                    match = true;
                    break;
                }
            }
            break;

        case CONDITION_WINDOW_CLASS:
            for (size_t j = 0; j < ctx->window_count; j++) {
                if (ctx->windows[j].is_focused &&
                    match_window_pattern(ctx->windows[j].class_name, rule->pattern)) {
                    match = true;
                    break;
                }
            }
            break;

        case CONDITION_AUDIO_LEVEL:
            match = check_audio_condition(ctx, rule);
            break;

        case CONDITION_TIME_RANGE: {
            time_t current_time = time(nullptr);
            struct tm *tm = localtime(&current_time);
            int current_minutes = tm->tm_hour * 60 + tm->tm_min;

            int start_min = rule->start_time / 60;
            int end_min = rule->end_time / 60;

            if (start_min <= end_min) {
                match = current_minutes >= start_min && current_minutes <= end_min;
            } else {
                // Crosses midnight
                match = current_minutes >= start_min || current_minutes <= end_min;
            }
            break;
        }

        default:
            break;
        }

        if (match) {
            best_rule = rule;
            best_priority = rule->priority;
        }
    }

    if (best_rule && strncmp(ctx->current_scene, best_rule->scene_name, 256) != 0) {
        // Schedule scene switch
        strncpy(ctx->target_scene, best_rule->scene_name, 256);
        ctx->switch_pending = true;
        ctx->last_switch_time = now;
    }

    // Execute pending switch after delay
    if (ctx->switch_pending &&
        (now - ctx->last_switch_time > (uint64_t)(ctx->switch_delay * 1000000ULL))) {

        obs_frontend_source_list *scenes = new obs_frontend_source_list();
        obs_frontend_get_scenes(scenes);

        for (size_t i = 0; i < scenes->sources.num; i++) {
            obs_source_t *scene = scenes->sources.array[i];
            const char *name = obs_source_get_name(scene);

            if (strcmp(name, ctx->target_scene) == 0) {
                // Switch scene
                if (ctx->enable_transitions && strlen(ctx->transition_name) > 0) {
                    obs_frontend_set_transition_duration(ctx->transition_duration);

                    obs_source_t *transition = obs_get_transition_by_name(ctx->transition_name);
                    if (transition) {
                        obs_frontend_set_current_transition(transition);
                        obs_source_release(transition);
                    }
                }

                obs_frontend_set_current_scene(scene);
                strncpy(ctx->current_scene, ctx->target_scene, 256);
                ctx->switch_pending = false;
                break;
            }
        }

        obs_frontend_source_list_free(scenes);
    }

    pthread_mutex_unlock(&ctx->mutex);
}

void sda_save(void *data, obs_data_t *settings)
{
    struct scene_director_data *ctx = (struct scene_director_data *)data;

    pthread_mutex_lock(&ctx->mutex);

    obs_data_set_int(settings, "mode", ctx->mode);
    obs_data_set_bool(settings, "enable_prediction", ctx->enable_prediction);
    // ... save other settings

    pthread_mutex_unlock(&ctx->mutex);
}

// =============================================================================
// UI PROPERTIES
// =============================================================================

obs_properties_t *sda_properties(void *data)
{
    obs_properties_t *props = obs_properties_create();

    // Mode selection
    obs_property_t *mode = obs_properties_add_list(props, "mode",
                                    obs_module_text("Switching Mode"),
                                    OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
    obs_property_list_add_int(mode, obs_module_text("Window Focus"), SWITCH_MODE_WINDOW);
    obs_property_list_add_int(mode, obs_module_text("Audio Level"), SWITCH_MODE_AUDIO);
    obs_property_list_add_int(mode, obs_module_text("Time Schedule"), SWITCH_MODE_TIME);
    obs_property_list_add_int(mode, obs_module_text("Manual"), SWITCH_MODE_MANUAL);
    obs_property_list_add_int(mode, obs_module_text("AI Prediction"), SWITCH_MODE_PREDICTION);

    // AI settings
    obs_properties_add_bool(props, "enable_prediction", obs_module_text("Enable Prediction"));
    obs_properties_add_int_slider(props, "prediction_window",
                                    obs_module_text("Prediction Window (seconds)"),
                                    5, 300, 5);
    obs_properties_add_float_slider(props, "confidence_threshold",
                                      obs_module_text("Confidence Threshold (%)"),
                                      0.0, 100.0, 1.0);

    // Transition settings
    obs_properties_add_bool(props, "enable_transitions", obs_module_text("Enable Transitions"));

    obs_property_t *transitions = obs_properties_add_list(props, "transition",
                                             obs_module_text("Transition"),
                                             OBS_COMBO_TYPE_EDITABLE,
                                             OBS_COMBO_FORMAT_STRING);
    obs_frontend_transition_list *trans = new obs_frontend_transition_list();
    obs_frontend_get_transitions(trans);
    for (size_t i = 0; i < trans->transitions.num; i++) {
        const char *name = obs_source_get_name(trans->transitions.array[i]);
        obs_property_list_add_string(transitions, name, name);
    }
    obs_frontend_transition_list_free(trans);

    obs_properties_add_int_slider(props, "transition_duration",
                                    obs_module_text("Transition Duration (ms)"),
                                    50, 2000, 50);

    // WebSocket settings
    obs_properties_add_bool(props, "ws_enabled", obs_module_text("Enable WebSocket"));
    obs_properties_add_int(props, "ws_port", obs_module_text("Port"), 1024, 65535, 1);

    return props;
}

void sda_defaults(obs_data_t *settings)
{
    obs_data_set_default_int(settings, "mode", SWITCH_MODE_WINDOW);
    obs_data_set_default_bool(settings, "enable_prediction", true);
    obs_data_set_default_int(settings, "prediction_window", 30);
    obs_data_set_default_double(settings, "confidence_threshold", 75.0);
    obs_data_set_default_int(settings, "switch_delay", 500);
    obs_data_set_default_bool(settings, "enable_transitions", true);
    obs_data_set_default_int(settings, "transition_duration", 300);
    obs_data_set_default_bool(settings, "ws_enabled", false);
    obs_data_set_default_int(settings, "ws_port", 8080);
}

// =============================================================================
// MODULE REGISTRATION
// =============================================================================

void sda_register()
{
    struct obs_source_info info = {};
    info.id = "scene_director_ai_filter";
    info.type = OBS_SOURCE_TYPE_FILTER;
    info.output_flags = OBS_SOURCE_VIDEO;
    info.get_name = sda_get_name;
    info.create = sda_create;
    info.destroy = sda_destroy;
    info.update = sda_update;
    info.activate = sda_activate;
    info.deactivate = sda_deactivate;
    info.video_tick = sda_tick;
    info.get_properties = sda_properties;
    info.get_defaults = sda_defaults;
    info.save = sda_save;

    obs_register_source(&info);

    blog(LOG_INFO, "[SceneDirector-AI] Plugin registered");
}

bool obs_module_load(void)
{
    obs_register_module(&sda_module);
    sda_register();
    return true;
}

void obs_module_unload(void)
{
    blog(LOG_INFO, "[SceneDirector-AI] Module unloaded");
}

const char *obs_module_name(void)
{
    return "SceneDirector-AI";
}

const char *obs_module_description(void)
{
    return obs_module_text("Smart scene switcher with AI-powered prediction");
}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
