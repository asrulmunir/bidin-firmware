/*
 * Board initialization for Freenove ESP32-S3 Display 2.8" (FNK0104B)
 * Exact copy from working xiaozhi-esp32 fork
 */

#include <esp_log.h>
#include <driver/i2c_master.h>
#include <driver/spi_master.h>
#include "board_config.h"

static const char *TAG = "board";

void board_init(void)
{
    ESP_LOGI(TAG, "Initializing Freenove ESP32-S3 Display 2.8\" (FNK0104B)");
    
    // Initialize I2C for audio codec
    ESP_LOGI(TAG, "Initializing I2C for audio...");
    i2c_master_bus_config_t i2c_bus_cfg = {
        .i2c_port = AUDIO_CODEC_I2C_NUM,
        .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
        .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = 1,
        },
    };
    i2c_master_bus_handle_t i2c_bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_handle));
    ESP_LOGI(TAG, "I2C initialized");
    
    // Initialize SPI for display
    ESP_LOGI(TAG, "Initializing SPI for display...");
    spi_bus_config_t buscfg = {
        .mosi_io_num = DISPLAY_MOSI_PIN,
        .miso_io_num = DISPLAY_MIS0_PIN,
        .sclk_io_num = DISPLAY_SCK_PIN,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "SPI initialized");
    
    // Configure button
    ESP_LOGI(TAG, "Configuring boot button (GPIO%d)...", BOOT_BUTTON_GPIO);
    gpio_config_t btn_config = {
        .pin_bit_mask = (1ULL << BOOT_BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = true,
        .pull_down_en = false,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&btn_config);
    
    // Configure LED
    ESP_LOGI(TAG, "Configuring builtin LED (GPIO%d)...", BUILTIN_LED_GPIO);
    gpio_reset_pin(BUILTIN_LED_GPIO);
    gpio_set_direction(BUILTIN_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BUILTIN_LED_GPIO, 0);
    
    // Configure backlight
    ESP_LOGI(TAG, "Configuring backlight (GPIO%d)...", DISPLAY_BACKLIGHT_PIN);
    gpio_reset_pin(DISPLAY_BACKLIGHT_PIN);
    gpio_set_direction(DISPLAY_BACKLIGHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_BACKLIGHT_PIN, 0);  // OFF initially
    
    ESP_LOGI(TAG, "✅ Board initialization complete");
}
