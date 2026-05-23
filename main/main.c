/*
 * MINIMAL Display Test - EXACT Xiaozhi Code
 * No board_init(), no audio, no wifi - just display
 * Direct copy from: https://github.com/78/xiaozhi-esp32
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

static const char *TAG = "xiaozhi_test";

// EXACT pins from xiaozhi-esp32 for Freenove ESP32-S3 2.8"
#define DISPLAY_MOSI  48
#define DISPLAY_SCLK  47
#define DISPLAY_CS    39
#define DISPLAY_DC    45
#define DISPLAY_RST   46
#define DISPLAY_BL    15

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

static spi_device_handle_t spi_dev = NULL;

static void lcd_write_cmd(uint8_t cmd)
{
    gpio_set_level(DISPLAY_DC, 0);
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 8,
        .tx_data[0] = cmd,
    };
    spi_device_polling_transmit(spi_dev, &t);
}

static void lcd_write_data(uint8_t data)
{
    gpio_set_level(DISPLAY_DC, 1);
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 8,
        .tx_data[0] = data,
    };
    spi_device_polling_transmit(spi_dev, &t);
}

static void lcd_init(void)
{
    ESP_LOGI(TAG, "LCD init sequence (EXACT xiaozhi)...");
    
    lcd_write_cmd(ST7789_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));
    
    lcd_write_cmd(ST7789_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    lcd_write_cmd(0xB2);  // Porch control
    lcd_write_data(0x0C);
    lcd_write_data(0x0C);
    lcd_write_data(0x00);
    lcd_write_data(0x33);
    lcd_write_data(0x33);
    
    lcd_write_cmd(0xB7);  // Gate control
    lcd_write_data(0x35);
    
    lcd_write_cmd(0xBB);  // VCOMS
    lcd_write_data(0x19);
    
    lcd_write_cmd(0xC0);  // Power control 1
    lcd_write_data(0x2C);
    
    lcd_write_cmd(0xC2);  // Power control 2
    lcd_write_data(0x01);
    
    lcd_write_cmd(0xC3);  // Power control 3
    lcd_write_data(0x12);
    
    lcd_write_cmd(0xC4);  // Power control 4
    lcd_write_data(0x20);
    
    lcd_write_cmd(0xC6);  // VCOM control
    lcd_write_data(0x0F);
    
    lcd_write_cmd(0xD0);  // Power control A
    lcd_write_data(0xA4);
    lcd_write_data(0xA1);
    
    lcd_write_cmd(ST7789_MADCTL);  // Memory access control
    lcd_write_data(0x00);
    
    lcd_write_cmd(ST7789_COLMOD);  // Interface pixel format
    lcd_write_data(0x05);
    
    lcd_write_cmd(ST7789_INVON);
    lcd_write_cmd(ST7789_NORON);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    lcd_write_cmd(ST7789_DISPON);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    ESP_LOGI(TAG, "LCD init complete");
}

static void lcd_fill(uint16_t color)
{
    // Swap bytes for little-endian
    uint16_t swapped = ((color & 0xFF) << 8) | ((color >> 8) & 0xFF);
    
    lcd_write_cmd(ST7789_CASET);
    lcd_write_data(0);
    lcd_write_data(0);
    lcd_write_data(1);
    lcd_write_data(31);  // 320
    
    lcd_write_cmd(ST7789_RASET);
    lcd_write_data(0);
    lcd_write_data(0);
    lcd_write_data(0);
    lcd_write_data(239);  // 240
    
    lcd_write_cmd(ST7789_RAMWR);
    
    gpio_set_level(DISPLAY_DC, 1);
    
    uint8_t buffer[256];
    for (int i = 0; i < 256; i += 2) {
        buffer[i] = swapped >> 8;
        buffer[i+1] = swapped & 0xFF;
    }
    
    for (int i = 0; i < 300; i++) {
        spi_transaction_t t = {
            .tx_buffer = buffer,
            .length = 256 * 8,
        };
        spi_device_polling_transmit(spi_dev, &t);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== MINIMAL XIAOZHI DISPLAY TEST ===");
    ESP_LOGI(TAG, "Pins: MOSI=%d, SCLK=%d, CS=%d, DC=%d, RST=%d, BL=%d",
             DISPLAY_MOSI, DISPLAY_SCLK, DISPLAY_CS, DISPLAY_DC, DISPLAY_RST, DISPLAY_BL);
    
    // Configure GPIO - EXACT like xiaozhi
    gpio_reset_pin(DISPLAY_DC);
    gpio_set_direction(DISPLAY_DC, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_DC, 0);
    
    gpio_reset_pin(DISPLAY_BL);
    gpio_set_direction(DISPLAY_BL, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_BL, 0);  // OFF initially
    
    // Initialize SPI - EXACT like xiaozhi
    ESP_LOGI(TAG, "Initializing SPI bus...");
    spi_bus_config_t buscfg = {
        .mosi_io_num = DISPLAY_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = DISPLAY_SCLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = 320 * 240 * sizeof(uint16_t),
    };
    spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_LOGI(TAG, "SPI bus initialized");
    
    // Add device - EXACT like xiaozhi
    ESP_LOGI(TAG, "Adding SPI device...");
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 26 * 1000 * 1000,  // 26 MHz
        .mode = 0,
        .spics_io_num = DISPLAY_CS,
        .queue_size = 7,
        .flags = SPI_DEVICE_NO_DUMMY,
    };
    spi_bus_add_device(SPI_HOST, &devcfg, &spi_dev);
    ESP_LOGI(TAG, "SPI device added");
    
    // Hardware reset - EXACT like xiaozhi (AFTER spi_bus_add_device!)
    ESP_LOGI(TAG, "Hardware reset...");
    gpio_reset_pin(DISPLAY_RST);
    gpio_set_direction(DISPLAY_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(DISPLAY_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));
    ESP_LOGI(TAG, "Reset complete");
    
    // LCD initialization
    lcd_init();
    
    // Turn on backlight
    ESP_LOGI(TAG, "Backlight ON");
    gpio_set_level(DISPLAY_BL, 1);
    
    // Test colors
    ESP_LOGI(TAG, "Filling RED...");
    lcd_fill(0xF800);  // RED
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "Filling GREEN...");
    lcd_fill(0x07E0);  // GREEN
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "Filling BLUE...");
    lcd_fill(0x001F);  // BLUE
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "=== TEST COMPLETE ===");
    
    // Done
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
