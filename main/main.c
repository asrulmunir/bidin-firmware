/*
 * Display Probe Test - Try ALL possible configurations
 * This will test different SPI modes and MADCTL values to find what works
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

static const char *TAG = "display_probe";

// Pin definitions for FNK0104B
#define DISPLAY_CS      GPIO_NUM_39
#define DISPLAY_DC      GPIO_NUM_45
#define DISPLAY_RESET   GPIO_NUM_46
#define DISPLAY_SCK     GPIO_NUM_47
#define DISPLAY_MOSI    GPIO_NUM_48
#define DISPLAY_BL      GPIO_NUM_15

#define SPI_HOST SPI2_HOST

// ST7789 commands
#define ST7789_SWRESET  0x01
#define ST7789_SLPOUT   0x11
#define ST7789_NORON    0x13
#define ST7789_INVON    0x21
#define ST7789_DISPON   0x29
#define ST7789_CASET    0x2A
#define ST7789_RASET    0x2B
#define ST7789_RAMWR    0x2C
#define ST7789_COLMOD   0x3A
#define ST7789_MADCTL   0x36

static spi_device_handle_t spi_handle = NULL;

static void send_command(uint8_t cmd)
{
    gpio_set_level(DISPLAY_DC, 0);
    spi_transaction_t t = {.flags = SPI_TRANS_USE_TXDATA, .length = 8, .tx_data[0] = cmd};
    spi_device_polling_transmit(spi_handle, &t);
}

static void send_data(uint8_t data)
{
    gpio_set_level(DISPLAY_DC, 1);
    spi_transaction_t t = {.flags = SPI_TRANS_USE_TXDATA, .length = 8, .tx_data[0] = data};
    spi_device_polling_transmit(spi_handle, &t);
}

static void fill_screen(uint16_t color)
{
    gpio_set_level(DISPLAY_DC, 1);
    
    // Set cursor to 0,0
    send_command(ST7789_CASET);
    send_data(0); send_data(0); send_data(1); send_data(31);  // 320 pixels
    send_command(ST7789_RASET);
    send_data(0); send_data(0); send_data(0); send_data(239);  // 240 pixels
    send_command(ST7789_RAMWR);
    
    // Send color data (swap bytes for little-endian)
    uint16_t swapped = ((color & 0xFF) << 8) | ((color >> 8) & 0xFF);
    uint8_t buffer[256];
    for (int i = 0; i < 256; i += 2) {
        buffer[i] = swapped >> 8;
        buffer[i+1] = swapped & 0xFF;
    }
    
    for (int i = 0; i < 300; i++) {  // 300 * 256 = 76800 pixels (more than screen)
        spi_transaction_t t = {.tx_buffer = buffer, .length = 256 * 8};
        spi_device_polling_transmit(spi_handle, &t);
    }
}

static bool test_display_config(int spi_mode, uint8_t madctl, const char *config_name)
{
    ESP_LOGI(TAG, "Testing: %s (SPI mode %d, MADCTL 0x%02X)", config_name, spi_mode, madctl);
    
    // Reinitialize SPI with new mode
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,  // Slow speed for testing
        .mode = spi_mode,
        .spics_io_num = DISPLAY_CS,
        .queue_size = 1,
    };
    
    if (spi_handle) spi_bus_remove_device(spi_handle);
    spi_bus_add_device(SPI_HOST, &devcfg, &spi_handle);
    
    // Reset display
    gpio_set_level(DISPLAY_RESET, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(DISPLAY_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Init sequence
    send_command(ST7789_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));
    
    send_command(ST7789_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    send_command(ST7789_MADCTL);
    send_data(madctl);
    
    send_command(ST7789_COLMOD);
    send_data(0x05);  // 16-bit
    
    send_command(ST7789_INVON);
    send_command(ST7789_NORON);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    send_command(ST7789_DISPON);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Turn on backlight
    gpio_set_level(DISPLAY_BL, 1);
    
    // Fill with RED
    fill_screen(0xF800);  // RED
    
    ESP_LOGI(TAG, "Waiting 3 seconds - check if screen is RED...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Fill with BLACK
    fill_screen(0x0000);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    return true;
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== DISPLAY PROBE TEST ===");
    ESP_LOGI(TAG, "Testing ALL SPI modes and MADCTL combinations");
    ESP_LOGI(TAG, "Watch the screen for RED color!");
    
    // Configure GPIO
    gpio_reset_pin(DISPLAY_DC);
    gpio_set_direction(DISPLAY_DC, GPIO_MODE_OUTPUT);
    gpio_reset_pin(DISPLAY_BL);
    gpio_set_direction(DISPLAY_BL, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_BL, 0);
    gpio_reset_pin(DISPLAY_RESET);
    gpio_set_direction(DISPLAY_RESET, GPIO_MODE_OUTPUT);
    
    // Initialize SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = DISPLAY_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = DISPLAY_SCK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = 320 * 240 * 2,
    };
    spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Test matrix: SPI mode × MADCTL
    // MADCTL values to try:
    // 0x00 = RGB, no rotation
    // 0x20 = RGB, 180° rotation  
    // 0x40 = BGR, no rotation
    // 0x60 = BGR, 180° rotation
    // 0x80 = RGB, vertical
    // 0xA0 = RGB, vertical flip
    // 0xC0 = BGR, vertical
    // 0xE0 = BGR, vertical flip
    
    test_display_config(0, 0x00, "Mode0-MADCTL0x00-RGB");
    test_display_config(0, 0x20, "Mode0-MADCTL0x20-RGB-180");
    test_display_config(0, 0x40, "Mode0-MADCTL0x40-BGR");
    test_display_config(0, 0x60, "Mode0-MADCTL0x60-BGR-180");
    
    test_display_config(3, 0x00, "Mode3-MADCTL0x00-RGB");
    test_display_config(3, 0x20, "Mode3-MADCTL0x20-RGB-180");
    test_display_config(3, 0x40, "Mode3-MADCTL0x40-BGR");
    test_display_config(3, 0x60, "Mode3-MADCTL0x60-BGR-180");
    
    ESP_LOGI(TAG, "=== TEST COMPLETE ===");
    ESP_LOGI(TAG, "If screen NEVER showed RED, issue is:");
    ESP_LOGI(TAG, "  1. Wrong pinout (check board variant)");
    ESP_LOGI(TAG, "  2. Display not connected properly");
    ESP_LOGI(TAG, "  3. Display hardware fault");
    
    // Blink backlight to signal end
    while (1) {
        gpio_set_level(DISPLAY_BL, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(DISPLAY_BL, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
