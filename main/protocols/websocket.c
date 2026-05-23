/*
 * WebSocket Client for Bidin Hermes Plugin
 * Minimal implementation using esp_http_client
 */

#include "websocket.h"
#include "board_config.h"
#include "audio.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "websocket";

// Server configuration - hardcoded for now
#define WEBSOCKET_URL "ws://hermes.tetupai.com:8000/bidin/v1/"

// WebSocket state
static bool g_connected = false;
static esp_http_client_handle_t g_client = NULL;
static char g_response_text[512] = {0};
static bool g_has_incoming_audio = false;

// Initialize WebSocket client
void websocket_init(void)
{
    ESP_LOGI(TAG, "Initializing WebSocket client...");
    
    // Configure HTTP client
    esp_http_client_config_t config = {
        .url = WEBSOCKET_URL,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 10000,
        .buffer_size = 2048,
        .disable_auto_redirect = true,
    };
    
    g_client = esp_http_client_init(&config);
    if (g_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return;
    }
    
    ESP_LOGI(TAG, "WebSocket client initialized (URL: %s)", WEBSOCKET_URL);
}

// Connect to Hermes server
void websocket_connect(void)
{
    ESP_LOGI(TAG, "Connecting to %s...", WEBSOCKET_URL);
    
    esp_err_t err = esp_http_client_open(g_client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open connection: %s", esp_err_to_name(err));
        g_connected = false;
        return;
    }
    
    // Perform WebSocket upgrade
    // TODO: Implement proper WebSocket handshake
    
    g_connected = true;
    ESP_LOGI(TAG, "Connected to Hermes server");
    
    // Send hello message
    websocket_send_hello();
}

// Disconnect from server
void websocket_disconnect(void)
{
    if (g_client != NULL) {
        esp_http_client_close(g_client);
        g_connected = false;
        ESP_LOGI(TAG, "Disconnected from server");
    }
}

// Check if connected
bool websocket_is_connected(void)
{
    return g_connected;
}

// Send hello message (device registration)
void websocket_send_hello(void)
{
    cJSON *hello = cJSON_CreateObject();
    cJSON_AddStringToObject(hello, "type", "hello");
    cJSON_AddStringToObject(hello, "device_id", BOARD_DEVICE_NAME);
    cJSON_AddStringToObject(hello, "version", BOARD_FIRMWARE_VERSION);
    
    char *json_str = cJSON_PrintUnformatted(hello);
    ESP_LOGI(TAG, "Sending hello: %s", json_str);
    
    // TODO: Send via WebSocket when implemented
    
    free(json_str);
    cJSON_Delete(hello);
}

// Send audio start (begin recording)
void websocket_send_audio_start(void)
{
    cJSON *start = cJSON_CreateObject();
    cJSON_AddStringToObject(start, "type", "listen");
    cJSON_AddStringToObject(start, "state", "start");
    
    char *json_str = cJSON_PrintUnformatted(start);
    ESP_LOGI(TAG, "Sending audio start: %s", json_str);
    
    // TODO: Send via WebSocket
    
    free(json_str);
    cJSON_Delete(start);
}

// Send audio frame
void websocket_send_audio_frame(audio_frame_t *frame)
{
    if (!g_connected || frame == NULL) {
        return;
    }
    
    // TODO: Send audio frame via WebSocket
    // For now, just log
    ESP_LOGV(TAG, "Sending audio frame: %zu samples", frame->count);
}

// Send audio end (stop recording)
void websocket_send_audio_end(void)
{
    cJSON *end = cJSON_CreateObject();
    cJSON_AddStringToObject(end, "type", "listen");
    cJSON_AddStringToObject(end, "state", "end");
    
    char *json_str = cJSON_PrintUnformatted(end);
    ESP_LOGI(TAG, "Sending audio end: %s", json_str);
    
    // TODO: Send via WebSocket
    
    free(json_str);
    cJSON_Delete(end);
}

// Check if server has incoming audio
bool websocket_has_incoming_audio(void)
{
    // TODO: Check for incoming data
    return g_has_incoming_audio;
}

// Read audio frame from server
bool websocket_read_audio_frame(audio_frame_t *frame)
{
    if (!g_has_incoming_audio || frame == NULL) {
        return false;
    }
    
    // TODO: Read from buffer and populate frame
    return false;
}

// Get server response text (after ASR)
const char* websocket_get_response_text(void)
{
    return g_response_text;
}
