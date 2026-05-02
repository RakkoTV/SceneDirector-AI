/*
 * Audio Monitor - Monitors audio levels from OBS sources
 * Copyright (c) 2026 Ramiro Silva
 */

#include "audio-monitor.hpp"
#include "scene-director-ai.hpp"

extern "C" {

void update_audio_levels(struct scene_director_data *data)
{
    if (!data) return;

    obs_frontend_source_list *sources = new obs_frontend_source_list();
    obs_frontend_get_global_sources(sources);

    for (size_t i = 0; i < sources->sources.num && i < MAX_AUDIO_MIXES; i++) {
        obs_source_t *source = sources->sources.array[i];

        enum obs_source_type type = obs_source_get_type(source);
        if (type != OBS_SOURCE_TYPE_INPUT && type != OBS_SOURCE_TYPE_SCENE)
            continue;

        uint32_t flags = obs_source_get_output_flags(source);
        if (!(flags & OBS_SOURCE_AUDIO))
            continue;

        // Get audio level
        float *levels = nullptr;
        size_t count = 0;

        if (obs_source_get_audio_levels(source, &levels, &count) && levels) {
            if (count > 0) {
                // Calculate RMS
                float sum = 0.0f;
                for (size_t j = 0; j < count; j++) {
                    sum += levels[j] * levels[j];
                }
                data->audio_levels[i] = sqrtf(sum / count);
            } else {
                data->audio_levels[i] = 0.0f;
            }
            bfree(levels);
        } else {
            data->audio_levels[i] = 0.0f;
        }
    }

    obs_frontend_source_list_free(sources);
}

bool check_audio_condition(struct scene_director_data *data, struct scene_rule *rule)
{
    if (!data || !rule) return false;

    // Find audio source by pattern
    obs_frontend_source_list *sources = new obs_frontend_source_list();
    obs_frontend_get_global_sources(sources);

    bool matched = false;
    float max_level = 0.0f;

    for (size_t i = 0; i < sources->sources.num; i++) {
        obs_source_t *source = sources->sources.array[i];
        const char *name = obs_source_get_name(source);

        if (match_window_pattern(name, rule->pattern)) {
            // Get the audio level for this source
            for (size_t j = 0; j < MAX_AUDIO_MIXES; j++) {
                // Find corresponding level (simplified)
                if (data->audio_levels[j] > max_level)
                    max_level = data->audio_levels[j];
            }
            matched = true;
        }
    }

    obs_frontend_source_list_free(sources);

    if (!matched)
        return false;

    // Convert to dB
    float level_db = 20.0f * log10f(max_level + 1e-10f);

    return level_db >= rule->threshold;
}

}  // extern "C"
