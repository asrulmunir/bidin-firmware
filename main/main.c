/*
 * Bidin Firmware - Main Entry Point
 * Using working display driver from xiaozhi-esp32 fork
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "display/display.h"

static const char *TAG = "bidin";

void app_main(void)
{
    ESP_LOGI(TAG, "Bidin Firmware v1.0.0 starting...");
    ESP_LOGI(TAG, "Target: Freenove ESP32-S3 Display 2.8\" (FNK0104B)");
    
    // Initialize display
    ESP_LOGI(TAG, "Initializing display...");
    display_init();
    
    // Show color test
    ESP_LOGI(TAG, "Running display test...");
    display_show_boot_screen();
    
    ESP_LOGI(TAG, "Display test complete!");
    
    // Main loop (placeholder)
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
