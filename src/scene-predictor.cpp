/*
 * Scene Predictor - ML-based prediction of scene switches
 * Copyright (c) 2026 Ramiro Silva
 */

#include "scene-predictor.hpp"
#include "scene-director-ai.hpp"

#include <cmath>
#include <algorithm>

extern "C" {

// Simple pattern matching and prediction algorithm
// Not a full ML implementation, but provides basic predictive capability

double predict_scene_switch(struct scene_director_data *data)
{
    if (!data || !data->pattern_weights)
        return 0.0;

    // Analyze recent scene switches
    time_t now = time(nullptr);

    // Calculate probabilities based on:
    // 1. Time of day patterns
    // 2. Window patterns
    // 3. Audio patterns

    double max_confidence = 0.0;

    // Time-based prediction
    struct tm *tm = localtime(&now);
    int hour = tm->tm_hour;
    int day_of_week = tm->tm_wday;

    // Weekend vs weekday patterns
    bool is_weekend = (day_of_week == 0 || day_of_week == 6);

    // Calculate confidence for each rule based on patterns
    for (size_t i = 0; i < data->rule_count; i++) {
        struct scene_rule *rule = &data->rules[i];
        if (!rule->enabled)
            continue;

        double confidence = 0.0;

        // Check if current conditions favor this rule
        switch (rule->condition) {
        case CONDITION_TIME_RANGE: {
            int current_minutes = hour * 60 + tm->tm_min;
            time_t start_min = rule->start_time / 60;
            time_t end_min = rule->end_time / 60;

            if (start_min <= end_min) {
                if (current_minutes >= start_min && current_minutes <= end_min)
                    confidence += 0.3;
            } else {
                if (current_minutes >= start_min || current_minutes <= end_min)
                    confidence += 0.3;
            }
            break;
        }

        case CONDITION_WINDOW_TITLE:
        case CONDITION_WINDOW_CLASS: {
            // Check if matching window is currently active
            for (size_t j = 0; j < data->window_count; j++) {
                if (data->windows[j].is_focused &&
                    match_window_pattern(data->windows[j].title, rule->pattern)) {
                    confidence += 0.5;
                    break;
                }
            }
            break;
        }

        case CONDITION_AUDIO_LEVEL: {
            if (check_audio_condition(data, rule))
                confidence += 0.4;
            break;
        }

        default:
            break;
        }

        // Add historical weight if this rule has been used before
        if (i < data->pattern_weights->size()) {
            confidence += (*data->pattern_weights)[i] * 0.2;
        }

        max_confidence = std::max(max_confidence, confidence);
    }

    // Update pattern weights based on current state
    if (max_confidence > 0.0) {
        // Store current state in history
        struct {
            time_t timestamp;
            int hour;
            int day_of_week;
            char primary_window[256];
            float audio_level;
        } state;

        state.timestamp = now;
        state.hour = hour;
        state.day_of_week = day_of_week;

        if (data->window_count > 0) {
            strncpy(state.primary_window, data->windows[0].title, 256);
        } else {
            state.primary_window[0] = '\0';
        }

        state.audio_level = data->audio_levels[0];

        circlebuf_push_back(&data->history_buffer, &state, sizeof(state));

        // Limit history size
        size_t max_history = 3600;  // ~1 hour of samples
        if (data->history_buffer.size > max_history * sizeof(state)) {
            circlebuf_pop_front(&data->history_buffer, nullptr, sizeof(state));
        }
    }

    // Scale confidence to 0-100
    return std::min(max_confidence * 100.0, 100.0);
}

void train_prediction_model(struct scene_director_data *data)
{
    if (!data)
        return;

    // Analyze history to find patterns
    // This would be called periodically or on demand

    if (data->history_buffer.size < 100)
        return;  // Not enough data

    // Clear existing weights
    data->pattern_weights->clear();
    data->pattern_weights->resize(data->rule_count, 0.0);

    // Analyze history for each rule
    size_t sample_count = data->history_buffer.size / sizeof(
        struct { time_t t; int h; int d; char w[256]; float a; }
    );

    for (size_t i = 0; i < data->rule_count; i++) {
        struct scene_rule *rule = &data->rules[i];

        // Count matches in history
        int matches = 0;
        int total = 0;

        // This is simplified - a real implementation would analyze
        // the full history buffer properly
        double weight = (double)matches / (double)total;
        (*data->pattern_weights)[i] = weight;
    }
}

}  // extern "C"
