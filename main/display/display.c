/*
 * Display Driver Stub Implementation
 * TODO: Implement proper ST7789 LCD driver
 */

#include "display.h"
#include "esp_log.h"

static const char *TAG = "display";

void display_init(void)
{
    ESP_LOGI(TAG, "Display initialized (stub)");
}

void display_show_boot_screen(void)
{
    ESP_LOGI(TAG, "Showing boot screen (stub)");
}

void display_show_listening(void)
{
    ESP_LOGI(TAG, "Showing listening screen (stub)");
}

void display_show_processing(void)
{
    ESP_LOGI(TAG, "Showing processing screen (stub)");
}

void display_show_speaking(void)
{
    ESP_LOGI(TAG, "Showing speaking screen (stub)");
}

void display_show_text(const char *text)
{
    if (text == NULL) return;
    
    ESP_LOGI(TAG, "Display text: %s", text);
}
