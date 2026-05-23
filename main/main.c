/*
 * MINIMAL ILI9341 Test - Raw SPI with CORRECT pinout
 * Using YOUR xiaozhi fork pinout
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

static const char *TAG = "ili9341_test";

// YOUR FORK'S PINOUT (from config.h)
#define DISPLAY_MOSI  GPIO_NUM_11
#define DISPLAY_SCLK  GPIO_NUM_12
#define DISPLAY_CS    GPIO_NUM_10
#define DISPLAY_DC    GPIO_NUM_46
#define DISPLAY_BL    GPIO_NUM_45

#define SPI_HOST SPI3_HOST
#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

static spi_device_handle_t spi_dev = NULL;

static void lcd_cmd(uint8_t cmd)
{
    gpio_set_level(DISPLAY_DC, 0);
    spi_transaction_t t = {.flags = SPI_TRANS_USE_TXDATA, .length = 8, .tx_data[0] = cmd};
    spi_device_polling_transmit(spi_dev, &t);
}

static void lcd_data(uint8_t data)
{
    gpio_set_level(DISPLAY_DC, 1);
    spi_transaction_t t = {.flags = SPI_TRANS_USE_TXDATA, .length = 8, .tx_data[0] = data};
    spi_device_polling_transmit(spi_dev, &t);
}

static void lcd_fill(uint16_t color)
{
    // Swap bytes for little-endian
    uint16_t swapped = ((color & 0xFF) << 8) | ((color >> 8) & 0xFF);
    
    // Set column (0-319)
    lcd_cmd(0x2A);
    lcd_data(0); lcd_data(0);
    lcd_data(1); lcd_data(0x3F);
    
    // Set row (0-239)
    lcd_cmd(0x2B);
    lcd_data(0); lcd_data(0);
    lcd_data(0); lcd_data(0xEF);
    
    // Write memory
    lcd_cmd(0x2C);
    
    gpio_set_level(DISPLAY_DC, 1);
    
    // Send pixels
    uint8_t buffer[256];
    for (int i = 0; i < 256; i += 2) {
        buffer[i] = swapped >> 8;
        buffer[i+1] = swapped & 0xFF;
    }
    
    for (int i = 0; i < 300; i++) {
        spi_transaction_t t = {.tx_buffer = buffer, .length = 256 * 8};
        spi_device_polling_transmit(spi_dev, &t);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== ILI9341 RAW SPI TEST ===");
    ESP_LOGI(TAG, "Pins: MOSI=%d, SCLK=%d, CS=%d, DC=%d, BL=%d",
             DISPLAY_MOSI, DISPLAY_SCLK, DISPLAY_CS, DISPLAY_DC, DISPLAY_BL);
    
    // GPIO setup
    gpio_reset_pin(DISPLAY_DC);
    gpio_set_direction(DISPLAY_DC, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_DC, 0);
    
    gpio_reset_pin(DISPLAY_BL);
    gpio_set_direction(DISPLAY_BL, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_BL, 0);
    
    // SPI setup
    spi_bus_config_t buscfg = {
        .mosi_io_num = DISPLAY_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = DISPLAY_SCLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * 2,
    };
    spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 20 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = DISPLAY_CS,
        .queue_size = 10,
    };
    spi_bus_add_device(SPI_HOST, &devcfg, &spi_dev);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // ILI9341 initialization
    ESP_LOGI(TAG, "Initializing ILI9341...");
    
    lcd_cmd(0xEF);  // Soft reset
    lcd_data(0x03); lcd_data(0x80); lcd_data(0x02);
    
    lcd_cmd(0xCF);
    lcd_data(0x00); lcd_data(0xC1); lcd_data(0x30);
    
    lcd_cmd(0xED);
    lcd_data(0x64); lcd_data(0x03); lcd_data(0x12); lcd_data(0x81);
    
    lcd_cmd(0xE8);
    lcd_data(0x85); lcd_data(0x00); lcd_data(0x66);
    
    lcd_cmd(0xCB);
    lcd_data(0x39); lcd_data(0x2C); lcd_data(0x00); lcd_data(0x34); lcd_data(0x02);
    
    lcd_cmd(0xF7);
    lcd_data(0x20);
    
    lcd_cmd(0xEA);
    lcd_data(0x00); lcd_data(0x00);
    
    lcd_cmd(0xC0);  // Power control 1
    lcd_data(0x23);
    
    lcd_cmd(0xC1);  // Power control 2
    lcd_data(0x10);
    
    lcd_cmd(0xC5);  // VCOM
    lcd_data(0x3E); lcd_data(0x28);
    
    lcd_cmd(0xC7);  // VCOM offset
    lcd_data(0x86);
    
    // *** CRITICAL: MADCTL ***
    // Bit 5 (0x20) = Swap XY (true in your config)
    // Bit 3 (0x08) = RGB order (your config says BGR, so set this bit)
    // Result: 0x28
    lcd_cmd(0x36);  // MADCTL
    lcd_data(0x28);  // 0x20 (swap) + 0x08 (BGR)
    
    lcd_cmd(0x3A);  // Pixel format
    lcd_data(0x55);  // 16-bit
    
    lcd_cmd(0xB1);  // Frame rate
    lcd_data(0x00); lcd_data(0x18);
    
    lcd_cmd(0xB6);  // Display function
    lcd_data(0x08); lcd_data(0x82); lcd_data(0x27);
    
    lcd_cmd(0xF2);  // Gamma
    lcd_data(0x00);
    
    lcd_cmd(0x26);  // Gamma curve
    lcd_data(0x01);
    
    // Gamma tables
    lcd_cmd(0xE0);
    lcd_data(0x0F); lcd_data(0x31); lcd_data(0x2B); lcd_data(0x0C); lcd_data(0x0E);
    lcd_data(0x08); lcd_data(0x4E); lcd_data(0xF1); lcd_data(0x37); lcd_data(0x07);
    lcd_data(0x10); lcd_data(0x03); lcd_data(0x0E); lcd_data(0x09); lcd_data(0x00);
    
    lcd_cmd(0xE1);
    lcd_data(0x00); lcd_data(0x0E); lcd_data(0x14); lcd_data(0x03); lcd_data(0x11);
    lcd_data(0x07); lcd_data(0x31); lcd_data(0xC1); lcd_data(0x48); lcd_data(0x08);
    lcd_data(0x0F); lcd_data(0x0C); lcd_data(0x31); lcd_data(0x36); lcd_data(0x0F);
    
    lcd_cmd(0x11);  // Exit sleep
    vTaskDelay(pdMS_TO_TICKS(120));
    
    lcd_cmd(0x29);  // Display on
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Backlight on
    gpio_set_level(DISPLAY_BL, 1);
    ESP_LOGI(TAG, "Backlight ON");
    
    // Color test
    ESP_LOGI(TAG, "RED...");
    lcd_fill(0xF800);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "GREEN...");
    lcd_fill(0x07E0);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "BLUE...");
    lcd_fill(0x001F);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "=== DONE ===");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
