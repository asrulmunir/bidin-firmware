/*
 * EXACT Xiaozhi Fork Implementation
 * Uses ESP-IDF LCD driver API (NOT raw SPI commands)
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
#include "esp_lcd_ili9341.h"

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

void app_main(void)
{
    ESP_LOGI(TAG, "=== EXACT XIAOZHI FORK TEST ===");
    
    // Step 1: Initialize SPI bus (EXACT like xiaozhi)
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
    
    // Step 2: Initialize panel IO (EXACT like xiaozhi)
    ESP_LOGI(TAG, "Install panel IO...");
    esp_lcd_panel_io_spi_config_t io_config = {};
    io_config.cs_gpio_num = DISPLAY_CS;
    io_config.dc_gpio_num = DISPLAY_DC;
    io_config.spi_mode = 0;  // DISPLAY_SPI_MODE
    io_config.pclk_hz = DISPLAY_SPI_SCLK_HZ;
    io_config.trans_queue_depth = 10;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI_HOST, &io_config, &panel_io));
    ESP_LOGI(TAG, "Panel IO installed");
    
    // Step 3: Initialize LCD panel (EXACT like xiaozhi)
    ESP_LOGI(TAG, "Install LCD driver ILI9341...");
    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = DISPLAY_RST;  // NC
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR;  // DISPLAY_RGB_ORDER
    panel_config.bits_per_pixel = 16;
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(panel_io, &panel_config, &panel));
    ESP_LOGI(TAG, "LCD driver installed");
    
    // Step 4: Reset panel (if reset pin exists)
    if (DISPLAY_RST != GPIO_NUM_NC) {
        ESP_LOGI(TAG, "Reset panel...");
        esp_lcd_panel_reset(panel);
    }
    
    // Step 5: Initialize panel
    ESP_LOGI(TAG, "Initialize panel...");
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_LOGI(TAG, "Panel initialized");
    
    // Step 6: Configure display (EXACT like xiaozhi)
    ESP_LOGI(TAG, "Configuring display...");
    esp_lcd_panel_invert_color(panel, DISPLAY_INVERT_COLOR);  // true
    esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);  // true
    esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);  // false, false
    ESP_LOGI(TAG, "Display configured: invert=%d, swap_xy=%d, mirror_x=%d, mirror_y=%d",
             DISPLAY_INVERT_COLOR, DISPLAY_SWAP_XY, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
    
    // Step 7: Turn on display
    ESP_LOGI(TAG, "Turn on display...");
    esp_lcd_panel_disp_on_off(panel, true);
    
    // Step 8: Backlight ON
    ESP_LOGI(TAG, "Turn on backlight...");
    gpio_reset_pin(DISPLAY_BL);
    gpio_set_direction(DISPLAY_BL, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_BL, 1);
    ESP_LOGI(TAG, "Backlight ON");
    
    // Step 9: Test with solid colors
    ESP_LOGI(TAG, "=== COLOR TEST START ===");
    
    // RED
    ESP_LOGI(TAG, "Filling RED...");
    uint16_t red = 0xF800;
    esp_lcd_panel_draw_bitmap(panel, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, &red);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // GREEN
    ESP_LOGI(TAG, "Filling GREEN...");
    uint16_t green = 0x07E0;
    esp_lcd_panel_draw_bitmap(panel, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, &green);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // BLUE
    ESP_LOGI(TAG, "Filling BLUE...");
    uint16_t blue = 0x001F;
    esp_lcd_panel_draw_bitmap(panel, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, &blue);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "=== TEST COMPLETE ===");
    ESP_LOGI(TAG, "If colors are correct, display is working!");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
