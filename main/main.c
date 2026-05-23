/*
 * MINIMAL Display Test - Using YOUR Xiaozhi Fork Pinout
 * From: https://github.com/asrulmunir/xiaozhi-esp32
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

static const char *TAG = "display_test";

// YOUR FORK'S PINOUT (from config.h)
#define DISPLAY_MOSI  GPIO_NUM_11
#define DISPLAY_SCLK  GPIO_NUM_12
#define DISPLAY_CS    GPIO_NUM_10
#define DISPLAY_DC    GPIO_NUM_46
#define DISPLAY_RST   GPIO_NUM_NC  // No reset pin
#define DISPLAY_BL    GPIO_NUM_45

#define SPI_HOST SPI3_HOST  // Your fork uses SPI3

// ILI9341 commands (your display uses ILI9341, not ST7789!)
#define ILI9341_SWRESET  0x01
#define ILI9341_SLPOUT   0x11
#define ILI9341_NORON    0x13
#define ILI9341_INVON    0x21
#define ILI9341_DISPON   0x29
#define ILI9341_CASET    0x2A
#define ILI9341_PASET    0x2B
#define ILI9341_RAMWR    0x2C
#define ILI9341_COLMOD   0x3A
#define ILI9341_MADCTL   0x36

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

static void lcd_fill(uint16_t color)
{
    // Swap bytes for little-endian
    uint16_t swapped = ((color & 0xFF) << 8) | ((color >> 8) & 0xFF);
    
    // Set column address (0 to 319 = 0x013F)
    lcd_write_cmd(ILI9341_CASET);
    lcd_write_data(0); lcd_write_data(0);   // Start column: 0
    lcd_write_data(1); lcd_write_data(0x3F); // End column: 319 (0x013F)
    
    // Set page address (0 to 239 = 0x00EF)
    lcd_write_cmd(ILI9341_PASET);
    lcd_write_data(0); lcd_write_data(0);   // Start row: 0
    lcd_write_data(0); lcd_write_data(0xEF); // End row: 239 (0x00EF)
    
    lcd_write_cmd(ILI9341_RAMWR);
    
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
    ESP_LOGI(TAG, "=== YOUR FORK'S PINOUT TEST ===");
    ESP_LOGI(TAG, "MOSI=%d, SCLK=%d, CS=%d, DC=%d, BL=%d",
             DISPLAY_MOSI, DISPLAY_SCLK, DISPLAY_CS, DISPLAY_DC, DISPLAY_BL);
    ESP_LOGI(TAG, "Display: ILI9341 (NOT ST7789!)");
    
    // Configure GPIO
    gpio_reset_pin(DISPLAY_DC);
    gpio_set_direction(DISPLAY_DC, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_DC, 0);
    
    gpio_reset_pin(DISPLAY_BL);
    gpio_set_direction(DISPLAY_BL, GPIO_MODE_OUTPUT);
    gpio_set_level(DISPLAY_BL, 0);
    
    // Initialize SPI
    ESP_LOGI(TAG, "Initializing SPI3...");
    spi_bus_config_t buscfg = {
        .mosi_io_num = DISPLAY_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = DISPLAY_SCLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = 320 * 240 * sizeof(uint16_t),
    };
    spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    
    ESP_LOGI(TAG, "Adding SPI device...");
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 20 * 1000 * 1000,  // 20 MHz (from your config)
        .mode = 0,  // SPI mode 0
        .spics_io_num = DISPLAY_CS,
        .queue_size = 10,  // Your fork uses 10
        .flags = SPI_DEVICE_NO_DUMMY,
    };
    spi_bus_add_device(SPI_HOST, &devcfg, &spi_dev);
    
    vTaskDelay(pdMS_TO_TICKS(200));
    
    // No hardware reset (RST = NC)
    ESP_LOGI(TAG, "No reset pin (NC), skipping reset");
    
    // ILI9341 initialization (from your fork)
    ESP_LOGI(TAG, "Initializing ILI9341...");
    
    lcd_write_cmd(ILI9341_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));
    
    lcd_write_cmd(ILI9341_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // MADCTL: Your fork settings
    lcd_write_cmd(ILI9341_MADCTL);
    lcd_write_data(0x28);  // 0x20 (swap XY) + 0x08 (RGB order) = 0x28
    // Was 0x20 (BGR), now 0x28 (RGB)
    
    // COLMOD: 16-bit
    lcd_write_cmd(ILI9341_COLMOD);
    lcd_write_data(0x05);
    
    // Invert ON (your fork has INVERT_COLOR=true)
    lcd_write_cmd(ILI9341_INVON);
    
    lcd_write_cmd(ILI9341_NORON);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    lcd_write_cmd(ILI9341_DISPON);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Backlight ON
    ESP_LOGI(TAG, "Backlight ON");
    gpio_set_level(DISPLAY_BL, 1);
    
    // Test colors
    ESP_LOGI(TAG, "Filling RED...");
    lcd_fill(0xF800);  // RED (RGB565)
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "Filling GREEN...");
    lcd_fill(0x07E0);  // GREEN
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "Filling BLUE...");
    lcd_fill(0x001F);  // BLUE
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "=== TEST COMPLETE ===");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
