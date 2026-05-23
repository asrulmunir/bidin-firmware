/*
 * WebSocket Client Implementation
 * Uses ESP-IDF's transport_ws and esp_http_client
 */

#include "websocket.h"
#include "board_config.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "websocket";

// Server configuration
#define WEBSOCKET_URL CONFIG_Bidin_WEBSOCKET_URL

// WebSocket state
static bool g_connected = false;
static esp_websocket_client_handle_t g_client = NULL;
static char g_response_text[512] = {0};
static bool g_has_audio = false;

// Event handler
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_handle_t event = (esp_websocket_event_handle_t)event_data;
    
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WebSocket connected");
            g_connected = true;
            websocket_send_hello();
            break;
            
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "WebSocket disconnected");
            g_connected = false;
            break;
            
        case WEBSOCKET_EVENT_DATA:
            ESP_LOGD(TAG, "Received data");
            
            if (event->data_len > 0) {
                // Check if binary (audio) or text (JSON)
                if (event->op_code == WS_TRANSPORT_OPCODES_BINARY) {
                    // Binary audio data from TTS
                    ESP_LOGD(TAG, "Received audio chunk (%d bytes)", event->data_len);
                    // TODO: Store in audio buffer for playback
                    g_has_audio = true;
                } else {
                    // Text JSON message
                    char *json = (char *)event->data;
                    ESP_LOGD(TAG, "Received JSON: %.*s", event->data_len, json);
                    
                    // Parse JSON
                    cJSON *root = cJSON_Parse(json);
                    if (root) {
                        cJSON *type = cJSON_GetObjectItem(root, "type");
                        if (type && strcmp(type->valuestring, "tts") == 0) {
                            cJSON *state = cJSON_GetObjectItem(root, "state");
                            if (state) {
                                if (strcmp(state->valuestring, "start") == 0) {
                                    ESP_LOGI(TAG, "TTS start");
                                } else if (strcmp(state->valuestring, "end") == 0) {
                                    ESP_LOGI(TAG, "TTS end");
                                    g_has_audio = false;
                                }
                            }
                        }
                        cJSON_Delete(root);
                    }
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
}

void websocket_init(void)
{
    ESP_LOGI(TAG, "Initializing WebSocket client");
    
    esp_websocket_client_config_t config = {
        .uri = WEBSOCKET_URL,
        .task_prio = 8,
        .task_stack = 4096,
        .buffer_size = 2048,
        .reconnect_timeout_ms = 5000,
        .network_timeout_ms = 5000,
        .ping_interval_sec = 30,
    };
    
    g_client = esp_websocket_client_init(&config);
    esp_websocket_register_events(g_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, NULL);
}

void websocket_connect(void)
{
    ESP_LOGI(TAG, "Connecting to %s", WEBSOCKET_URL);
    esp_websocket_client_start(g_client);
}

void websocket_disconnect(void)
{
    if (g_client) {
        esp_websocket_client_close(g_client, pdMS_TO_TICKS(1000));
        g_connected = false;
    }
}

bool websocket_is_connected(void)
{
    return g_connected;
}

void websocket_send_hello(void)
{
    if (!g_connected) return;
    
    // Send hello message (device registration)
    cJSON *hello = cJSON_CreateObject();
    cJSON_AddStringToObject(hello, "type", "hello");
    cJSON_AddStringToObject(hello, "version", "v1");
    cJSON_AddStringToObject(hello, "device_id", BOARD_NAME);
    
    char *json = cJSON_PrintUnformatted(hello);
    esp_websocket_client_send_text(g_client, json, strlen(json), pdMS_TO_TICKS(1000));
    
    cJSON_Delete(hello);
    free(json);
    
    ESP_LOGI(TAG, "Hello sent");
}

void websocket_send_audio_start(void)
{
    if (!g_connected) return;
    
    cJSON *msg = cJSON_CreateObject();
    cJSON_AddStringToObject(msg, "type", "listen");
    cJSON_AddStringToObject(msg, "state", "start");
    
    char *json = cJSON_PrintUnformatted(msg);
    esp_websocket_client_send_text(g_client, json, strlen(json), pdMS_TO_TICKS(1000));
    
    cJSON_Delete(msg);
    free(json);
}

void websocket_send_audio_frame(audio_frame_t *frame)
{
    if (!g_connected) return;
    
    esp_websocket_client_send_bin(g_client, frame->data, frame->size, pdMS_TO_TICKS(1000));
}

void websocket_send_audio_end(void)
{
    if (!g_connected) return;
    
    cJSON *msg = cJSON_CreateObject();
    cJSON_AddStringToObject(msg, "type", "listen");
    cJSON_AddStringToObject(msg, "state", "end");
    
    char *json = cJSON_PrintUnformatted(msg);
    esp_websocket_client_send_text(g_client, json, strlen(json), pdMS_TO_TICKS(1000));
    
    cJSON_Delete(msg);
    free(json);
}

bool websocket_has_incoming_audio(void)
{
    return g_has_audio;
}

bool websocket_read_audio_frame(audio_frame_t *frame)
{
    // TODO: Implement audio buffer read
    // For now, return false
    return false;
}

const char* websocket_get_response_text(void)
{
    return g_response_text;
}
