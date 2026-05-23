/*
 * Board Initialization for Freenove ESP32-S3 Display 2.8"
 */

#include "board_config.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "board";

void board_init(void)
{
    ESP_LOGI(TAG, "Initializing Freenove ESP32-S3 Display 2.8\" board");
    
    // Configure button GPIO
    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BOARD_BUTTON_BOOT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&button_config);
    ESP_LOGI(TAG, "Button GPIO configured");
    
    // Configure display backlight ONLY (speaker pins skipped - shared GPIO conflict)
    // NOTE: GPIO15 is shared between DISPLAY_BACKLIGHT and AUDIO_SPEAKER_LRCK
    // We configure it here for display; audio driver must not claim it
    gpio_reset_pin(DISPLAY_BACKLIGHT_PIN);
    gpio_set_direction(DISPLAY_BACKLIGHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_BACKLIGHT_PIN, 0); // Backlight OFF initially
    ESP_LOGI(TAG, "Backlight configured (GPIO%d)", DISPLAY_BACKLIGHT_PIN);
    
    // Speaker mute SKIPPED - GPIO conflict with display
    // gpio_reset_pin(AUDIO_SPEAKER_MUTE_PIN);
    // gpio_set_direction(AUDIO_SPEAKER_MUTE_PIN, GPIO_MODE_OUTPUT);
    // gpio_set_level(AUDIO_SPEAKER_MUTE_PIN, 0);
    ESP_LOGI(TAG, "Speaker mute skipped (shared GPIO)");
    
    // I2C SKIPPED - not needed for basic display/audio
    // i2c_config_t i2c_config = {...}
    // i2c_driver_install(...);
    ESP_LOGI(TAG, "I2C skipped (not needed)");
    
    ESP_LOGI(TAG, "✅ Board initialization complete");
}

bool board_button_pressed(void)
{
    return gpio_get_level(BOARD_BUTTON_BOOT_PIN) == 0; // Active low
}

int board_get_battery_level(void)
{
    // Simple ADC read for battery voltage
    // TODO: Implement ADC reading and voltage-to-percentage conversion
    return -1; // Not implemented yet
}
