/*
 * WebSocket Client for Bidin Hermes Plugin
 * Minimal implementation using esp_http_client with WebSocket transport
 */

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Forward reference - audio_frame_t is defined in audio.h
// Include audio.h in websocket.c to get the actual definition
struct audio_frame;

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
void websocket_send_audio_frame(struct audio_frame *frame);

// Send audio end (stop recording)
void websocket_send_audio_end(void);

// Check if server has incoming audio
bool websocket_has_incoming_audio(void);

// Read audio frame from server
bool websocket_read_audio_frame(struct audio_frame *frame);

// Get server response text (after ASR)
const char* websocket_get_response_text(void);

#endif // WEBSOCKET_H
