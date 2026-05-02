/*
 * WebSocket Server Header
 * Copyright (c) 2026 Ramiro Silva
 */

#pragma once

#include "scene-director-ai.hpp"

struct lws_context;

struct ws_server_data {
    int port;
    bool running;
    pthread_t thread;
    struct lws_context *context;
};

#ifdef __cplusplus
extern "C" {
#endif

void start_websocket_server(struct scene_director_data *data);
void stop_websocket_server(struct scene_director_data *data);
void handle_websocket_message(struct scene_director_data *data, const char *message);

#ifdef __cplusplus
}
#endif
