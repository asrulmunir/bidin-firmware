/*
 * Display Driver for Freenove ESP32-S3 2.8" (ST7789 LCD)
 * SPI interface, 320x240 resolution
 */

#include "display.h"
#include "board_config.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <math.h>

static const char *TAG = "display";

// Display state
static spi_device_handle_t g_spi_handle = NULL;
static bool g_initialized = false;

// SPI configuration
#define SPI_HOST SPI2_HOST
#define SPI_CLOCK_SPEED_HZ (24 * 1000 * 1000)  // 24 MHz

// ST7789 commands
#define ST7789_NOP      0x00
#define ST7789_SWRESET  0x01
#define ST7789_SLPIN    0x10
#define ST7789_SLPOUT   0x11
#define ST7789_NORON    0x13
#define ST7789_INVOFF   0x20
#define ST7789_INVON    0x21
#define ST7789_DISPOFF  0x28
#define ST7789_DISPON   0x29
#define ST7789_CASET    0x2A
#define ST7789_RASET    0x2B
#define ST7789_RAMWR    0x2C
#define ST7789_COLMOD   0x3A
#define ST7789_MADCTL   0x36
#define ST7789_FRMCTR1  0xB1

// Memory Access Control
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00

// Color definitions
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_GRAY    0x7BEF
#define COLOR_ORANGE  0xFD20

// Font data (simplified 5x7 font)
static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // (space)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    // ... (truncated for brevity - would include full font table)
};

// Helper: Convert RGB565
static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Helper: Send command to display
static void display_command(uint8_t cmd)
{
    gpio_set_level(DISPLAY_DC_PIN, 0);  // Command mode
    gpio_set_level(DISPLAY_SPI_CS_PIN, 0);  // Select
    
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 8,
        .tx_data[0] = cmd,
    };
    spi_device_polling_transmit(g_spi_handle, &t);
    
    gpio_set_level(DISPLAY_SPI_CS_PIN, 1);  // Deselect
}

// Helper: Send data to display
static void display_data(uint8_t data)
{
    gpio_set_level(DISPLAY_DC_PIN, 1);  // Data mode
    gpio_set_level(DISPLAY_SPI_CS_PIN, 0);  // Select
    
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 8,
        .tx_data[0] = data,
    };
    spi_device_polling_transmit(g_spi_handle, &t);
    
    gpio_set_level(DISPLAY_SPI_CS_PIN, 1);  // Deselect
}

// Helper: Set cursor position
static void display_set_cursor(uint16_t x, uint16_t y)
{
    // Column Address Set
    display_command(ST7789_CASET);
    display_data(x >> 8);
    display_data(x & 0xFF);
    display_data((x + 1) >> 8);
    display_data((x + 1) & 0xFF);
    
    // Row Address Set
    display_command(ST7789_RASET);
    display_data(y >> 8);
    display_data(y & 0xFF);
    display_data((y + 1) >> 8);
    display_data((y + 1) & 0xFF);
    
    // Memory Write
    display_command(ST7789_RAMWR);
}

// Initialize display
void display_init(void)
{
    ESP_LOGI(TAG, "Initializing ST7789 display...");
    
    // Configure GPIO pins
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DISPLAY_DC_PIN) | 
                        (1ULL << DISPLAY_BACKLIGHT_PIN) |
                        (1ULL << DISPLAY_SPI_CS_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&io_conf);
    
    // Reset display
    gpio_set_level(DISPLAY_SPI_CS_PIN, 1);
    gpio_set_level(DISPLAY_DC_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Initialize SPI
    spi_bus_config_t bus_config = {
        .mosi_io_num = DISPLAY_SPI_MOSI_PIN,
        .miso_io_num = -1,  // Not used
        .sclk_io_num = DISPLAY_SPI_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * 2,
    };
    
    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = SPI_CLOCK_SPEED_HZ,
        .mode = 3,  // ST7789 uses SPI mode 3
        .spics_io_num = -1,  // Manual CS control
        .queue_size = 1,
    };
    
    esp_err_t ret = spi_bus_initialize(SPI_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return;
    }
    
    ret = spi_bus_add_device(SPI_HOST, &dev_config, &g_spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
        return;
    }
    
    // Hardware reset
    gpio_set_level(DISPLAY_SPI_CS_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(DISPLAY_SPI_CS_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // ST7789 initialization sequence
    display_command(ST7789_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));
    
    display_command(ST7789_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Memory Access Control - RGB, no rotation
    display_command(ST7789_MADCTL);
    display_data(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
    
    // Color Mode: 16-bit (RGB565)
    display_command(ST7789_COLMOD);
    display_data(0x05);  // 16-bit
    
    // Frame Rate Control
    display_command(ST7789_FRMCTR1);
    display_data(0x00);
    display_data(0x14);  // 70 Hz
    
    // Display Inversion ON
    display_command(ST7789_INVON);
    
    // Normal Display Mode ON
    display_command(ST7789_NORON);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Display ON
    display_command(ST7789_DISPON);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Turn on backlight
    gpio_set_level(DISPLAY_BACKLIGHT_PIN, 1);
    
    // Clear screen to black
    display_fill(COLOR_BLACK);
    
    g_initialized = true;
    ESP_LOGI(TAG, "Display initialized successfully");
}

// Fill screen with color
void display_fill(uint16_t color)
{
    if (!g_initialized) return;
    
    gpio_set_level(DISPLAY_DC_PIN, 1);  // Data mode
    gpio_set_level(DISPLAY_SPI_CS_PIN, 0);  // Select
    
    // Set cursor to (0,0) with size (320,240)
    display_set_cursor(0, 0);
    
    // Prepare buffer
    uint16_t buffer[256];
    for (int i = 0; i < 256; i++) {
        buffer[i] = color;
    }
    
    // Send pixels
    for (int i = 0; i < (DISPLAY_WIDTH * DISPLAY_HEIGHT) / 256; i++) {
        spi_transaction_t t = {
            .tx_buffer = buffer,
            .length = 256 * 16,  // 256 pixels * 16 bits
        };
        spi_device_polling_transmit(g_spi_handle, &t);
    }
    
    gpio_set_level(DISPLAY_SPI_CS_PIN, 1);  // Deselect
}

// Draw pixel at (x, y)
void display_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (!g_initialized || x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
    
    display_set_cursor(x, y);
    
    gpio_set_level(DISPLAY_DC_PIN, 1);
    gpio_set_level(DISPLAY_SPI_CS_PIN, 0);
    
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 16,
        .tx_data[0] = color >> 8,
        .tx_data[1] = color & 0xFF,
    };
    spi_device_polling_transmit(g_spi_handle, &t);
    
    gpio_set_level(DISPLAY_SPI_CS_PIN, 1);
}

// Draw character (5x7 font)
static void display_draw_char(uint16_t x, uint16_t y, char c, uint16_t color)
{
    if (!g_initialized) return;
    
    uint8_t char_index = (c >= ' ' && c <= '~') ? (c - ' ') : 0;
    const uint8_t *char_data = font5x7[char_index];
    
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if ((char_data[col] >> row) & 0x01) {
                display_draw_pixel(x + col, y + row, color);
            }
        }
    }
}

// Draw string
void display_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color)
{
    if (!g_initialized || str == NULL) return;
    
    uint16_t cursor_x = x;
    while (*str) {
        display_draw_char(cursor_x, y, *str, color);
        cursor_x += 6;  // 5 pixels + 1 spacing
        str++;
    }
}

// Draw rectangle
void display_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (!g_initialized) return;
    
    // Top and bottom lines
    for (uint16_t i = 0; i < w; i++) {
        display_draw_pixel(x + i, y, color);
        display_draw_pixel(x + i, y + h - 1, color);
    }
    
    // Left and right lines
    for (uint16_t i = 0; i < h; i++) {
        display_draw_pixel(x, y + i, color);
        display_draw_pixel(x + w - 1, y + i, color);
    }
}

// Fill rectangle
void display_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (!g_initialized) return;
    
    for (uint16_t i = 0; i < h; i++) {
        for (uint16_t j = 0; j < w; j++) {
            display_draw_pixel(x + j, y + i, color);
        }
    }
}

// Show boot screen
void display_show_boot_screen(void)
{
    if (!g_initialized) return;
    
    ESP_LOGI(TAG, "Showing boot screen");
    
    // Clear to blue
    display_fill(COLOR_BLUE);
    
    // White box for title
    display_fill_rect(20, 80, 280, 60, COLOR_WHITE);
    display_draw_rect(20, 80, 280, 60, COLOR_WHITE);
    
    // Title text
    display_draw_string(60, 100, "Bidin", COLOR_BLUE);
    display_draw_string(40, 120, "Voice Assistant", COLOR_BLUE);
    
    // Version
    display_draw_string(120, 200, "v1.0.0", COLOR_WHITE);
}

// Show listening screen
void display_show_listening(void)
{
    if (!g_initialized) return;
    
    ESP_LOGI(TAG, "Showing listening screen");
    
    // Clear to green
    display_fill(COLOR_GREEN);
    
    // Microphone icon (simple circle)
    display_fill_rect(120, 80, 80, 100, COLOR_WHITE);
    display_fill_rect(130, 70, 60, 20, COLOR_WHITE);
    
    // Text
    display_draw_string(100, 200, "Listening...", COLOR_BLACK);
}

// Show processing screen
void display_show_processing(void)
{
    if (!g_initialized) return;
    
    ESP_LOGI(TAG, "Showing processing screen");
    
    // Clear to yellow
    display_fill(COLOR_YELLOW);
    
    // Spinner or dots
    display_fill_rect(140, 100, 40, 40, COLOR_BLACK);
    
    // Text
    display_draw_string(100, 200, "Processing...", COLOR_BLACK);
}

// Show speaking screen
void display_show_speaking(void)
{
    if (!g_initialized) return;
    
    ESP_LOGI(TAG, "Showing speaking screen");
    
    // Clear to cyan
    display_fill(COLOR_CYAN);
    
    // Speaker icon
    display_fill_rect(120, 90, 80, 60, COLOR_WHITE);
    
    // Text
    display_draw_string(110, 200, "Speaking...", COLOR_BLACK);
}

// Show text message
void display_show_text(const char *text)
{
    if (!g_initialized || text == NULL) return;
    
    ESP_LOGI(TAG, "Displaying text: %s", text);
    
    // Clear to black
    display_fill(COLOR_BLACK);
    
    // Draw text in white, centered
    uint16_t text_width = strlen(text) * 6;
    uint16_t x = (DISPLAY_WIDTH - text_width) / 2;
    display_draw_string(x, 110, text, COLOR_WHITE);
}
