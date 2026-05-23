/*
 * Display Driver for Freenove ESP32-S3 2.8" (ST7789 LCD)
 * EXACT copy of xiaozhi-esp32 initialization sequence
 * Reference: https://github.com/78/xiaozhi-esp32/blob/main/main/boards/freenove-esp32s3-display-2.8-lcd/lcd.cc
 */

#include "display.h"
#include "board_config.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdint.h>

static const char *TAG = "display";

// Display state
static spi_device_handle_t g_spi_handle = NULL;
static bool g_initialized = false;

// SPI configuration
#define SPI_HOST SPI2_HOST

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

// Color definitions (RGB565 - BIG ENDIAN for SPI)
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

// Helper: Swap bytes for little-endian ESP32
static inline uint16_t swap_bytes(uint16_t val)
{
    return ((val & 0xFF) << 8) | ((val >> 8) & 0xFF);
}

// Helper: Send command to display
static void display_command(uint8_t cmd)
{
    gpio_set_level(DISPLAY_DC_PIN, 0);  // Command mode (DC LOW)
    
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 8,
        .tx_data[0] = cmd,
    };
    spi_device_polling_transmit(g_spi_handle, &t);
}

// Helper: Send data to display
static void display_data(uint8_t data)
{
    gpio_set_level(DISPLAY_DC_PIN, 1);  // Data mode (DC HIGH)
    
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 8,
        .tx_data[0] = data,
    };
    spi_device_polling_transmit(g_spi_handle, &t);
}

// Helper: Set cursor position
static void display_set_cursor(uint16_t x, uint16_t y)
{
    // Column Address Set
    display_command(ST7789_CASET);
    display_data(x >> 8);
    display_data(x & 0xFF);
    display_data(((x + 1) >> 8) & 0xFF);
    display_data((x + 1) & 0xFF);
    
    // Row Address Set
    display_command(ST7789_RASET);
    display_data(y >> 8);
    display_data(y & 0xFF);
    display_data(((y + 1) >> 8) & 0xFF);
    display_data((y + 1) & 0xFF);
    
    // Memory Write
    display_command(ST7789_RAMWR);
}

// Initialize display - EXACT xiaozhi sequence
void display_init(void)
{
    ESP_LOGI(TAG, "=== Initializing ST7789 display (xiaozhi sequence) ===");
    
    // Step 1: Configure GPIO pins (EXCEPT RESET - do that after SPI init)
    ESP_LOGI(TAG, "Configuring DC and CS pins...");
    gpio_reset_pin(DISPLAY_DC_PIN);
    gpio_set_direction(DISPLAY_DC_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_DC_PIN, 0);
    
    // Step 2: Initialize SPI bus FIRST (before reset!)
    ESP_LOGI(TAG, "Initializing SPI bus...");
    spi_bus_config_t buscfg = {
        .mosi_io_num = DISPLAY_SPI_MOSI_PIN,
        .miso_io_num = GPIO_NUM_NC,  // Use GPIO_NUM_NC, not -1!
        .sclk_io_num = DISPLAY_SPI_SCK_PIN,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t),
    };
    
    esp_err_t ret = spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "SPI bus initialized");
    
    // Step 3: Add SPI device with AUTO CS (not manual!)
    ESP_LOGI(TAG, "Adding SPI device with auto CS...");
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 26 * 1000 * 1000,  // 26 MHz (xiaozhi uses 26, not 24!)
        .mode = 0,  // SPI mode 0
        .spics_io_num = DISPLAY_SPI_CS_PIN,  // Use GPIO pin for auto CS!
        .queue_size = 7,  // xiaozhi uses 7
        .flags = SPI_DEVICE_NO_DUMMY,
    };
    
    ret = spi_bus_add_device(SPI_HOST, &devcfg, &g_spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI device add failed: %s", esp_err_to_name(ret));
        spi_bus_free(SPI_HOST);
        return;
    }
    ESP_LOGI(TAG, "SPI device added, handle=%p", (void*)g_spi_handle);
    
    // Step 4: NOW do hardware reset (AFTER spi_bus_add_device!)
    ESP_LOGI(TAG, "Performing hardware reset...");
    gpio_reset_pin(DISPLAY_RESET_PIN);
    gpio_set_direction(DISPLAY_RESET_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_RESET_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));  // 10ms
    gpio_set_level(DISPLAY_RESET_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(120));  // 120ms
    ESP_LOGI(TAG, "Reset complete");
    
    // Step 5: Send initialization commands (EXACT xiaozhi sequence)
    ESP_LOGI(TAG, "Sending initialization commands...");
    
    display_command(ST7789_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));
    
    display_command(ST7789_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Porch control
    display_command(0xB2);
    display_data(0x0C);
    display_data(0x0C);
    display_data(0x00);
    display_data(0x33);
    display_data(0x33);
    
    // Gate control
    display_command(0xB7);
    display_data(0x35);
    
    // VCOMS
    display_command(0xBB);
    display_data(0x19);
    
    // Power control 1
    display_command(0xC0);
    display_data(0x2C);
    
    // Power control 2
    display_command(0xC2);
    display_data(0x01);
    
    // Power control 3
    display_command(0xC3);
    display_data(0x12);
    
    // Power control 4
    display_command(0xC4);
    display_data(0x20);
    
    // VCOM control
    display_command(0xC6);
    display_data(0x0F);
    
    // Power control A
    display_command(0xD0);
    display_data(0xA4);
    display_data(0xA1);
    
    // Memory Access Control
    display_command(ST7789_MADCTL);
    display_data(0x00);  // RGB, no rotation
    
    // Pixel format: 16-bit RGB565
    display_command(ST7789_COLMOD);
    display_data(0x05);
    
    // Display inversion ON
    display_command(ST7789_INVON);
    
    // Normal display mode ON
    display_command(ST7789_NORON);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Display ON
    display_command(ST7789_DISPON);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Step 6: Turn on backlight LAST
    ESP_LOGI(TAG, "Turning on backlight...");
    gpio_reset_pin(DISPLAY_BACKLIGHT_PIN);
    gpio_set_direction(DISPLAY_BACKLIGHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_BACKLIGHT_PIN, 1);
    
    // Clear screen to black
    ESP_LOGI(TAG, "Clearing screen...");
    display_fill(COLOR_BLACK);
    
    g_initialized = true;
    ESP_LOGI(TAG, "✅ Display initialized successfully!");
}

// Fill screen with color - with byte swap for little-endian
void display_fill(uint16_t color)
{
    if (!g_initialized) return;
    
    // Swap bytes for SPI (little-endian → big-endian)
    uint16_t swapped_color = swap_bytes(color);
    
    gpio_set_level(DISPLAY_DC_PIN, 1);  // Data mode
    
    // Set cursor to (0,0)
    display_set_cursor(0, 0);
    
    // Prepare buffer with swapped bytes
    uint16_t buffer[256];
    for (int i = 0; i < 256; i++) {
        buffer[i] = swapped_color;
    }
    
    // Send pixels in chunks
    int total_pixels = DISPLAY_WIDTH * DISPLAY_HEIGHT;
    int chunks = total_pixels / 256;
    
    for (int i = 0; i < chunks; i++) {
        spi_transaction_t t = {
            .tx_buffer = buffer,
            .length = 256 * 16,
        };
        spi_device_polling_transmit(g_spi_handle, &t);
        
        // Yield every 10 chunks
        if (i % 10 == 0) {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

// Draw pixel with byte swap
void display_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (!g_initialized || x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
    
    // Swap bytes for SPI
    uint16_t swapped_color = swap_bytes(color);
    
    display_set_cursor(x, y);
    
    gpio_set_level(DISPLAY_DC_PIN, 1);
    
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 16,
        .tx_data[0] = swapped_color >> 8,
        .tx_data[1] = swapped_color & 0xFF,
    };
    spi_device_polling_transmit(g_spi_handle, &t);
}

// Draw string (simplified - 5x7 font)
void display_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color)
{
    // TODO: Implement proper font rendering
    // For now, just skip to avoid crash
}

// Draw rectangle
void display_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (!g_initialized || w == 0 || h == 0) return;
    display_fill_rect(x, y, w, 1, color);  // Top
    display_fill_rect(x, y + h - 1, w, 1, color);  // Bottom
    display_fill_rect(x, y, 1, h, color);  // Left
    display_fill_rect(x + w - 1, y, 1, h, color);  // Right
}

// Fill rectangle with byte swap
void display_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (!g_initialized || w == 0 || h == 0) return;
    
    // Clamp to display bounds
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
    if (x + w > DISPLAY_WIDTH) w = DISPLAY_WIDTH - x;
    if (y + h > DISPLAY_HEIGHT) h = DISPLAY_HEIGHT - y;
    
    // Swap bytes for SPI
    uint16_t swapped_color = swap_bytes(color);
    
    // Set cursor
    display_set_cursor(x, y);
    
    gpio_set_level(DISPLAY_DC_PIN, 1);
    
    // Prepare buffer
    uint16_t buffer[256];
    for (int i = 0; i < 256; i++) {
        buffer[i] = swapped_color;
    }
    
    // Send pixels
    int pixels_sent = 0;
    int total_pixels = w * h;
    
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            buffer[pixels_sent % 256] = swapped_color;
            pixels_sent++;
            
            if (pixels_sent % 256 == 0 || pixels_sent == total_pixels) {
                int count = (pixels_sent % 256 == 0) ? 256 : (pixels_sent % 256);
                spi_transaction_t t = {
                    .tx_buffer = buffer,
                    .length = count * 16,
                };
                spi_device_polling_transmit(g_spi_handle, &t);
                
                if (pixels_sent % 2560 == 0) {
                    vTaskDelay(pdMS_TO_TICKS(1));
                }
            }
        }
    }
}

// Show boot screen - simple color test
void display_show_boot_screen(void)
{
    if (!g_initialized) return;
    
    ESP_LOGI(TAG, "=== DISPLAY TEST START ===");
    
    // TEST 1: Full screen RED
    ESP_LOGI(TAG, "Filling screen with RED...");
    display_fill(COLOR_RED);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // TEST 2: Full screen GREEN
    ESP_LOGI(TAG, "Filling screen with GREEN...");
    display_fill(COLOR_GREEN);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // TEST 3: Full screen BLUE
    ESP_LOGI(TAG, "Filling screen with BLUE...");
    display_fill(COLOR_BLUE);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "=== DISPLAY TEST END ===");
}

// Show listening screen
void display_show_listening(void)
{
    if (!g_initialized) return;
    display_fill(COLOR_GREEN);
}

// Show processing screen
void display_show_processing(void)
{
    if (!g_initialized) return;
    display_fill(COLOR_YELLOW);
}

// Show speaking screen
void display_show_speaking(void)
{
    if (!g_initialized) return;
    display_fill(COLOR_CYAN);
}

// Show text message
void display_show_text(const char *text)
{
    if (!g_initialized || text == NULL) return;
    // TODO: Implement text display
}
