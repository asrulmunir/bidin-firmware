/*
 * WebSocket Client for Bidin Hermes Plugin
 * Using esp_websocket_client component
 */

#include "websocket.h"
#include "board_config.h"
#include "audio.h"
#include "esp_log.h"
#include "esp_websocket_client.h"  // Built into ESP-IDF v5.5
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "websocket";

// Server configuration
#define WEBSOCKET_URL "ws://hermes.tetupai.com:8000/bidin/v1/"

// WebSocket state
static bool g_connected = false;
static esp_websocket_client_handle_t g_client = NULL;
static char g_response_text[512] = {0};
static bool g_has_incoming_audio = false;

// WebSocket event handler
static esp_err_t websocket_event_handler(esp_websocket_event_handle_t event)
{
    switch (event->event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WebSocket connected");
            g_connected = true;
            break;
            
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "WebSocket disconnected");
            g_connected = false;
            break;
            
        case WEBSOCKET_EVENT_DATA:
            ESP_LOGI(TAG, "Received data: %.*s", 
                     event->data_len, (char *)event->data);
            
            // Parse incoming message
            if (event->data_len > 0) {
                cJSON *root = cJSON_ParseWithLength((char *)event->data, event->data_len);
                if (root) {
                    cJSON *type = cJSON_GetObjectItem(root, "type");
                    if (type && cJSON_IsString(type)) {
                        if (strcmp(type->valuestring, "audio") == 0) {
                            g_has_incoming_audio = true;
                            // TODO: Extract audio data and buffer it
                        } else if (strcmp(type->valuestring, "response") == 0) {
                            cJSON *text = cJSON_GetObjectItem(root, "text");
                            if (text && cJSON_IsString(text)) {
                                strncpy(g_response_text, text->valuestring, sizeof(g_response_text) - 1);
                                ESP_LOGI(TAG, "Server response: %s", g_response_text);
                            }
                        }
                    }
                    cJSON_Delete(root);
                }
            }
            break;
            
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGE(TAG, "WebSocket error");
            g_connected = false;
            break;
            
        default:
            break;
    }
    return ESP_OK;
}

// Initialize WebSocket client
void websocket_init(void)
{
    ESP_LOGI(TAG, "Initializing WebSocket client...");
    
    // Configure WebSocket client
    esp_websocket_client_config_t config = {
        .uri = WEBSOCKET_URL,
        .buffer_size = 2048,
        .out_buffer_size = 2048,
        .reconnect_timeout_ms = 5000,
        .network_timeout_ms = 10000,
        .pingpong_timeout_sec = 30,
    };
    
    g_client = esp_websocket_client_init(&config);
    if (g_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize WebSocket client");
        return;
    }
    
    // Register event handler
    esp_websocket_register_events(g_client, WEBSOCKET_EVENT_ANY, 
                                   websocket_event_handler, NULL);
    
    ESP_LOGI(TAG, "WebSocket client initialized (URL: %s)", WEBSOCKET_URL);
}

// Connect to Hermes server
void websocket_connect(void)
{
    ESP_LOGI(TAG, "Connecting to %s...", WEBSOCKET_URL);
    
    esp_err_t err = esp_websocket_client_start(g_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WebSocket: %s", esp_err_to_name(err));
        g_connected = false;
        return;
    }
    
    // Wait for connection
    int timeout = 5000; // 5 seconds
    while (!g_connected && timeout > 0) {
        vTaskDelay(pdMS_TO_TICKS(100));
        timeout -= 100;
    }
    
    if (!g_connected) {
        ESP_LOGE(TAG, "Connection timeout");
        return;
    }
    
    ESP_LOGI(TAG, "Connected to Hermes server");
    
    // Send hello message
    websocket_send_hello();
}

// Disconnect from server
void websocket_disconnect(void)
{
    if (g_client != NULL) {
        esp_websocket_client_close(g_client, pdMS_TO_TICKS(1000));
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
    
    esp_websocket_client_send_text(g_client, json_str, strlen(json_str), 
                                    pdMS_TO_TICKS(1000));
    
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
    
    esp_websocket_client_send_text(g_client, json_str, strlen(json_str), 
                                    pdMS_TO_TICKS(1000));
    
    free(json_str);
    cJSON_Delete(start);
}

// Send audio frame
void websocket_send_audio_frame(audio_frame_t *frame)
{
    if (!g_connected || frame == NULL) {
        return;
    }
    
    // Send raw audio data as binary
    esp_websocket_client_send_bin(g_client, (const char *)frame->samples, 
                                   frame->count * sizeof(int16_t), 
                                   pdMS_TO_TICKS(10));
}

// Send audio end (stop recording)
void websocket_send_audio_end(void)
{
    cJSON *end = cJSON_CreateObject();
    cJSON_AddStringToObject(end, "type", "listen");
    cJSON_AddStringToObject(end, "state", "end");
    
    char *json_str = cJSON_PrintUnformatted(end);
    ESP_LOGI(TAG, "Sending audio end: %s", json_str);
    
    esp_websocket_client_send_text(g_client, json_str, strlen(json_str), 
                                    pdMS_TO_TICKS(1000));
    
    free(json_str);
    cJSON_Delete(end);
}

// Check if server has incoming audio
bool websocket_has_incoming_audio(void)
{
    bool has_audio = g_has_incoming_audio;
    g_has_incoming_audio = false; // Reset flag after reading
    return has_audio;
}

// Read audio frame from server
bool websocket_read_audio_frame(audio_frame_t *frame)
{
    if (!g_connected || frame == NULL) {
        return false;
    }
    
    // TODO: Implement proper audio buffering from event handler
    // For now, return false to indicate no data available
    return false;
}

// Get server response text (after ASR)
const char* websocket_get_response_text(void)
{
    return g_response_text;
}
