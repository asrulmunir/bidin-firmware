/*
 * EXACT Xiaozhi Fork Implementation
 * Uses ESP-IDF LCD driver API with generic initialization
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"

static const char *TAG = "xiaozhi_test";

// YOUR FORK'S PINOUT
#define DISPLAY_MOSI  GPIO_NUM_11
#define DISPLAY_SCLK  GPIO_NUM_12
#define DISPLAY_CS    GPIO_NUM_10
#define DISPLAY_DC    GPIO_NUM_46
#define DISPLAY_RST   GPIO_NUM_NC
#define DISPLAY_BL    GPIO_NUM_45

#define SPI_HOST SPI3_HOST

// Config from your fork
#define DISPLAY_WIDTH         320
#define DISPLAY_HEIGHT        240
#define DISPLAY_SWAP_XY       true
#define DISPLAY_MIRROR_X      false
#define DISPLAY_MIRROR_Y      false
#define DISPLAY_INVERT_COLOR  true
#define DISPLAY_SPI_SCLK_HZ   (20 * 1000 * 1000)

static esp_lcd_panel_io_handle_t panel_io = NULL;
static esp_lcd_panel_handle_t panel = NULL;

// ILI9341 initialization commands (from ESP-IDF examples)
static const uint8_t lcd_init_cmds[] = {
    0xEF, 0, 3, 0x03, 0x80, 0x02,
    0xCF, 0, 3, 0x00, 0xC1, 0x30,
    0xED, 0, 4, 0x64, 0x03, 0x12, 0x81,
    0xE8, 0, 3, 0x85, 0x00, 0x66,
    0xCB, 0, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
    0xF7, 0, 1, 0x20,
    0xEA, 0, 2, 0x00, 0x00,
    0xC0, 0, 1, 0x23,  // Power control 1
    0xC1, 0, 1, 0x10,  // Power control 2
    0xC5, 0, 2, 0x3E, 0x28,  // VCOM control 1
    0xC7, 0, 1, 0x86,  // VCOM control 2
    0x36, 1, 1, 0x28,  // MADCTL - THIS IS THE KEY! 0x28 = BGR + swap XY
    0x3A, 1, 1, 0x55,  // Pixel format: 16-bit
    0xB1, 0, 2, 0x00, 0x18,  // Frame rate
    0xB6, 0, 3, 0x08, 0x82, 0x27,  // Display function
    0xF2, 0, 1, 0x00,  // Gamma function disable
    0x26, 0, 1, 0x01,  // Gamma curve selected
    0xE0, 0, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
    0xE1, 0, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
    0x11, 0, 0x80,  // Exit sleep mode (delay 128ms)
    0x29, 0, 0x80,  // Display on (delay 128ms)
};

void app_main(void)
{
    ESP_LOGI(TAG, "=== XIAOZHI FORK LCD TEST ===");
    
    // Step 1: Initialize SPI bus
    ESP_LOGI(TAG, "Initialize SPI bus...");
    spi_bus_config_t buscfg = {
        .mosi_io_num = DISPLAY_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = DISPLAY_SCLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "SPI bus initialized");
    
    // Step 2: Initialize panel IO
    ESP_LOGI(TAG, "Install panel IO...");
    esp_lcd_panel_io_spi_config_t io_config = {};
    io_config.cs_gpio_num = DISPLAY_CS;
    io_config.dc_gpio_num = DISPLAY_DC;
    io_config.spi_mode = 0;
    io_config.pclk_hz = DISPLAY_SPI_SCLK_HZ;
    io_config.trans_queue_depth = 10;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI_HOST, &io_config, &panel_io));
    ESP_LOGI(TAG, "Panel IO installed");
    
    // Step 3: Create LCD panel with vendor-specific init
    ESP_LOGI(TAG, "Create LCD panel...");
    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = DISPLAY_RST;
    panel_config.bits_per_pixel = 16;
    panel_config.flags = {
        .reset_active_high = 1,
    };
    
    // Use generic vendor panel with custom init commands
    ESP_ERROR_CHECK(esp_lcd_new_panel_spi_vendor(panel_io, &panel_config, 
        (const esp_lcd_vendor_init_cmd_t *)lcd_init_cmds, 
        sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]), 
        &panel));
    ESP_LOGI(TAG, "LCD panel created");
    
    // Step 4: Initialize panel
    ESP_LOGI(TAG, "Initialize panel...");
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_LOGI(TAG, "Panel initialized");
    
    // Step 5: Configure display
    ESP_LOGI(TAG, "Configuring display...");
    esp_lcd_panel_invert_color(panel, DISPLAY_INVERT_COLOR);
    esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
    esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
    
    // Step 6: Turn on display
    ESP_LOGI(TAG, "Turn on display...");
    esp_lcd_panel_disp_on_off(panel, true);
    
    // Step 7: Backlight ON
    ESP_LOGI(TAG, "Turn on backlight...");
    gpio_reset_pin(DISPLAY_BL);
    gpio_set_direction(DISPLAY_BL, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_BL, 1);
    ESP_LOGI(TAG, "Backlight ON");
    
    // Step 8: Test colors
    ESP_LOGI(TAG, "=== COLOR TEST START ===");
    
    // Create color buffer
    uint16_t *buffer = malloc(DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t));
    if (!buffer) {
        ESP_LOGE(TAG, "Failed to allocate buffer!");
        return;
    }
    
    // RED
    ESP_LOGI(TAG, "Filling RED...");
    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        buffer[i] = 0xF800;  // RED (RGB565)
    }
    esp_lcd_panel_draw_bitmap(panel, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, buffer);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // GREEN
    ESP_LOGI(TAG, "Filling GREEN...");
    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        buffer[i] = 0x07E0;  // GREEN
    }
    esp_lcd_panel_draw_bitmap(panel, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, buffer);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // BLUE
    ESP_LOGI(TAG, "Filling BLUE...");
    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        buffer[i] = 0x001F;  // BLUE
    }
    esp_lcd_panel_draw_bitmap(panel, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, buffer);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    free(buffer);
    
    ESP_LOGI(TAG, "=== TEST COMPLETE ===");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
