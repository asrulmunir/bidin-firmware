/*
 * WebSocket Client Stub for Bidin Hermes Plugin
 * TODO: Implement proper WebSocket with custom handshake or third-party library
 * For now, this is a placeholder that simulates connection
 */

#include "websocket.h"
#include "board_config.h"
#include "audio.h"
#include "esp_log.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "websocket";

// Server configuration
#define WEBSOCKET_URL "ws://hermes.tetupai.com:8000/bidin/v1/"

// WebSocket state
static bool g_connected = false;
static char g_response_text[512] = {0};
static bool g_has_incoming_audio = false;

// Initialize WebSocket client
void websocket_init(void)
{
    ESP_LOGI(TAG, "Initializing WebSocket client (stub)...");
    ESP_LOGI(TAG, "Server URL: %s", WEBSOCKET_URL);
    ESP_LOGW(TAG, "WARNING: WebSocket not fully implemented yet");
    ESP_LOGW(TAG, "TODO: Implement custom WebSocket handshake or use third-party library");
}

// Connect to Hermes server
void websocket_connect(void)
{
    ESP_LOGI(TAG, "Connecting to %s...", WEBSOCKET_URL);
    
    // Stub: Simulate connection after 2 seconds
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    g_connected = true;
    ESP_LOGI(TAG, "Connected (stub - simulated)");
    
    // Send hello message
    websocket_send_hello();
}

// Disconnect from server
void websocket_disconnect(void)
{
    g_connected = false;
    ESP_LOGI(TAG, "Disconnected (stub)");
}

// Check if connected
bool websocket_is_connected(void)
{
    return g_connected;
}

// Send hello message (device registration)
void websocket_send_hello(void)
{
    if (!g_connected) return;
    
    cJSON *hello = cJSON_CreateObject();
    cJSON_AddStringToObject(hello, "type", "hello");
    cJSON_AddStringToObject(hello, "device_id", BOARD_DEVICE_NAME);
    cJSON_AddStringToObject(hello, "version", BOARD_FIRMWARE_VERSION);
    
    char *json_str = cJSON_PrintUnformatted(hello);
    ESP_LOGI(TAG, "Sending hello: %s", json_str);
    ESP_LOGW(TAG, "WARNING: Message not actually sent (stub)");
    
    // Simulate server response
    vTaskDelay(pdMS_TO_TICKS(500));
    ESP_LOGI(TAG, "Simulated server response: OK");
    
    free(json_str);
    cJSON_Delete(hello);
}

// Send audio start (begin recording)
void websocket_send_audio_start(void)
{
    if (!g_connected) return;
    
    cJSON *start = cJSON_CreateObject();
    cJSON_AddStringToObject(start, "type", "listen");
    cJSON_AddStringToObject(start, "state", "start");
    
    char *json_str = cJSON_PrintUnformatted(start);
    ESP_LOGI(TAG, "Sending audio start: %s", json_str);
    ESP_LOGW(TAG, "WARNING: Message not actually sent (stub)");
    
    free(json_str);
    cJSON_Delete(start);
}

// Send audio frame
void websocket_send_audio_frame(audio_frame_t *frame)
{
    if (!g_connected || frame == NULL) {
        return;
    }
    
    // Stub: Just log, don't actually send
    ESP_LOGV(TAG, "Sending audio frame: %zu samples (stub - not sent)", frame->count);
}

// Send audio end (stop recording)
void websocket_send_audio_end(void)
{
    if (!g_connected) return;
    
    cJSON *end = cJSON_CreateObject();
    cJSON_AddStringToObject(end, "type", "listen");
    cJSON_AddStringToObject(end, "state", "end");
    
    char *json_str = cJSON_PrintUnformatted(end);
    ESP_LOGI(TAG, "Sending audio end: %s", json_str);
    ESP_LOGW(TAG, "WARNING: Message not actually sent (stub)");
    
    free(json_str);
    cJSON_Delete(end);
}

// Check if server has incoming audio
bool websocket_has_incoming_audio(void)
{
    // Stub: Always return false
    return g_has_incoming_audio;
}

// Read audio frame from server
bool websocket_read_audio_frame(audio_frame_t *frame)
{
    if (frame == NULL) {
        return false;
    }
    
    // Stub: No incoming audio
    return false;
}

// Get server response text (after ASR)
const char* websocket_get_response_text(void)
{
    return g_response_text;
}
