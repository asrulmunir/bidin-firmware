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
    
    // Configure display backlight
    gpio_reset_pin(DISPLAY_BACKLIGHT_PIN);
    gpio_set_direction(DISPLAY_BACKLIGHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_BACKLIGHT_PIN, 1); // Backlight ON
    
    // Configure speaker mute
    gpio_reset_pin(AUDIO_SPEAKER_MUTE_PIN);
    gpio_set_direction(AUDIO_SPEAKER_MUTE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(AUDIO_SPEAKER_MUTE_PIN, 0); // Speaker UNMUTED
    
    // Initialize I2C (for sensors)
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_NUM_0, &i2c_config);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    
    ESP_LOGI(TAG, "Board initialization complete");
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
