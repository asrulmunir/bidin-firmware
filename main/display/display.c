/*
 * Display driver for Freenove ESP32-S3 2.8" (ILI9341 LCD)
 * Exact copy from working xiaozhi-esp32 fork
 */

#include <esp_log.h>
#include <driver/gpio.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_ili9341.h>
#include "board_config.h"

static const char *TAG = "display";

static esp_lcd_panel_io_handle_t panel_io = NULL;
static esp_lcd_panel_handle_t panel = NULL;
static bool g_initialized = false;

void display_init(void)
{
    ESP_LOGI(TAG, "Initializing ILI9341 display...");
    
    // Configure DC and CS GPIOs
    gpio_reset_pin(DISPLAY_DC_PIN);
    gpio_set_direction(DISPLAY_DC_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_DC_PIN, 0);
    
    // Initialize panel IO
    ESP_LOGI(TAG, "Install panel IO...");
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = DISPLAY_CS_PIN,
        .dc_gpio_num = DISPLAY_DC_PIN,
        .spi_mode = DISPLAY_SPI_MODE,
        .pclk_hz = DISPLAY_SPI_SCLK_HZ,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(LCD_SPI_HOST, &io_config, &panel_io));
    ESP_LOGI(TAG, "Panel IO installed");
    
    // Initialize LCD panel
    ESP_LOGI(TAG, "Install LCD driver ILI9341...");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = DISPLAY_RST_PIN,
        .rgb_ele_order = DISPLAY_RGB_ORDER,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(panel_io, &panel_config, &panel));
    ESP_LOGI(TAG, "LCD driver installed");
    
    // Reset panel
    if (DISPLAY_RST_PIN != GPIO_NUM_NC) {
        ESP_LOGI(TAG, "Reset panel...");
        esp_lcd_panel_reset(panel);
    }
    
    // Initialize panel
    ESP_LOGI(TAG, "Initialize panel...");
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_LOGI(TAG, "Panel initialized");
    
    // Configure display
    ESP_LOGI(TAG, "Configuring display...");
    esp_lcd_panel_invert_color(panel, DISPLAY_INVERT_COLOR);
    esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
    esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
    
    // Turn on display
    ESP_LOGI(TAG, "Turn on display...");
    esp_lcd_panel_disp_on_off(panel, true);
    
    // Turn on backlight
    ESP_LOGI(TAG, "Turn on backlight...");
    gpio_reset_pin(DISPLAY_BACKLIGHT_PIN);
    gpio_set_direction(DISPLAY_BACKLIGHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_BACKLIGHT_PIN, 1);
    
    g_initialized = true;
    ESP_LOGI(TAG, "✅ Display initialized successfully!");
}

void display_fill(uint16_t color)
{
    if (!g_initialized) return;
    esp_lcd_panel_draw_bitmap(panel, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, &color);
}

void display_show_boot_screen(void)
{
    if (!g_initialized) return;
    
    ESP_LOGI(TAG, "=== DISPLAY TEST ===");
    
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
}
