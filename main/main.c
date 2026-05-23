/*
 * Bidin Firmware - Minimal Voice Assistant
 * Main application entry point
 * 
 * Flow:
 * 1. Initialize hardware (display, audio, buttons)
 * 2. Connect to WiFi
 * 3. Connect to Hermes WebSocket server
 * 4. Listen for audio (push-to-talk or wake word)
 * 5. Stream audio to server, receive response, play back
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "board_config.h"
#include "audio.h"
#include "display.h"
#include "websocket.h"
#include "wifi.h"

static const char *TAG = "bidin";

void app_main(void)
{
    // EARLIEST POSSIBLE LOG - verify we reach app_main
    printf("### APP_MAIN STARTED ###\n");
    fflush(stdout);
    
    ESP_LOGI(TAG, "Bidin Firmware v1.0.0 starting...");
    
    // Initialize NVS (non-volatile storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition truncated, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize board (pins, peripherals)
    ESP_LOGI(TAG, "Initializing board...");
    board_init();
    
    // Initialize display - TEMPORARILY DISABLED FOR DEBUG
    ESP_LOGI(TAG, "Skipping display init (debug mode)...");
    // display_init();
    // display_show_boot_screen();
    
    // TEST: Just log to verify code reaches here
    ESP_LOGI(TAG, "=== CODE RUNNING PAST DISPLAY! ===");
    
    // Initialize audio (I2S mic + speaker)
    ESP_LOGI(TAG, "Initializing audio...");
    audio_init();
    
    // Connect to WiFi
    ESP_LOGI(TAG, "Connecting to WiFi...");
    wifi_init();
    wifi_connect();
    
    // Wait for WiFi connection
    while (!wifi_is_connected()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // Connect to Hermes WebSocket server
    ESP_LOGI(TAG, "Connecting to Hermes server...");
    websocket_init();
    websocket_connect();
    
    // Main loop: handle audio streaming
    ESP_LOGI(TAG, "Starting audio streaming loop...");
    while (1) {
        // Check for button press (push-to-talk)
        if (board_button_pressed()) {
            ESP_LOGI(TAG, "Button pressed - start listening");
            display_show_listening();
            audio_start_recording();
            websocket_send_audio_start();
            
            // Record and stream audio until button released
            while (board_button_pressed()) {
                audio_frame_t frame;
                audio_read_frame(&frame);
                websocket_send_audio_frame(&frame);
                vTaskDelay(pdMS_TO_TICKS(20)); // 50fps, 20ms frames
            }
            
            // Stop recording
            audio_stop_recording();
            websocket_send_audio_end();
            display_show_processing();
            
            // Wait for response
            ESP_LOGI(TAG, "Waiting for server response...");
        }
        
        // Check for incoming audio from server
        if (websocket_has_incoming_audio()) {
            audio_frame_t frame;
            while (websocket_read_audio_frame(&frame)) {
                audio_play_frame(&frame);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
