/*
 * WebSocket Server - Allows external control of scene switching
 * Copyright (c) 2026 Ramiro Silva
 */

#include "websocket-server.hpp"
#include "scene-director-ai.hpp"

#include <cstring>

// Simple WebSocket implementation using libwebsockets would go here
// For now, stub implementation

extern "C" {

struct ws_server_data {
    int port;
    bool running;
    pthread_t thread;
    struct lws_context *context;
};

void *ws_thread_func(void *param)
{
    struct scene_director_data *data = (struct scene_director_data *)param;

    // WebSocket server loop would go here
    // For now, just sleep to simulate running server
    while (data->ws_server) {
        usleep(100000);  // 100ms
    }

    return nullptr;
}

void start_websocket_server(struct scene_director_data *data)
{
    if (!data || data->ws_server)
        return;

    data->ws_server = new ws_server_data();
    data->ws_server->port = data->ws_port;
    data->ws_server->running = true;

    // Create thread for WebSocket server
    pthread_create(&data->ws_server->thread, nullptr, ws_thread_func, data);

    blog(LOG_INFO, "[SceneDirector-AI] WebSocket server started on port %d",
         data->ws_port);
}

void stop_websocket_server(struct scene_director_data *data)
{
    if (!data || !data->ws_server)
        return;

    data->ws_server->running = false;

    // Wait for thread to finish
    pthread_join(data->ws_server->thread, nullptr);

    delete data->ws_server;
    data->ws_server = nullptr;

    blog(LOG_INFO, "[SceneDirector-AI] WebSocket server stopped");
}

void handle_websocket_message(struct scene_director_data *data, const char *message)
{
    if (!data || !message)
        return;

    // Parse JSON message
    // Expected format: {"action": "switch", "scene": "Scene Name"}

    if (strstr(message, "\"action\"") && strstr(message, "\"switch\"")) {
        const char *scene_start = strstr(message, "\"scene\"");
        if (scene_start) {
            scene_start = strchr(scene_start, ':');
            if (scene_start) {
                scene_start++;  // Skip colon
                while (*scene_start == ' ' || *scene_start == '"')
                    scene_start++;

                char scene_name[256];
                const char *scene_end = strchr(scene_start, '"');
                if (scene_end) {
                    size_t len = scene_end - scene_start;
                    if (len < sizeof(scene_name)) {
                        memcpy(scene_name, scene_start, len);
                        scene_name[len] = '\0';

                        strncpy(data->target_scene, scene_name, 256);
                        data->switch_pending = true;

                        blog(LOG_INFO, "[SceneDirector-AI] WebSocket: Switch to scene '%s'",
                             scene_name);
                    }
                }
            }
        }
    }
}

}  // extern "C"
