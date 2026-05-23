/*
 * WebSocket Client for Bidin Hermes Plugin
 * Minimal implementation using esp_http_client
 */

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

// Must include audio.h first to get audio_frame_t definition
#include "audio.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Initialize WebSocket client
void websocket_init(void);

// Connect to Hermes server
void websocket_connect(void);

// Disconnect from server
void websocket_disconnect(void);

// Check if connected
bool websocket_is_connected(void);

// Send hello message (device registration)
void websocket_send_hello(void);

// Send audio start (begin recording)
void websocket_send_audio_start(void);

// Send audio frame
void websocket_send_audio_frame(audio_frame_t *frame);

// Send audio end (stop recording)
void websocket_send_audio_end(void);

// Check if server has incoming audio
bool websocket_has_incoming_audio(void);

// Read audio frame from server
bool websocket_read_audio_frame(audio_frame_t *frame);

// Get server response text (after ASR)
const char* websocket_get_response_text(void);

#endif // WEBSOCKET_H
