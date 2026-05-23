/*
 * Test FNK0104A Pinout
 * Different from FNK0104B:
 * - CS: GPIO34 (not GPIO39)
 * - DC: GPIO39 (not GPIO45)
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

static const char *TAG = "pinout_test";

// FNK0104A pins (DIFFERENT from FNK0104B!)
#define DISPLAY_CS      GPIO_NUM_34  // Was GPIO39 for FNK0104B
#define DISPLAY_DC      GPIO_NUM_39  // Was GPIO45 for FNK0104B
#define DISPLAY_RESET   GPIO_NUM_46  // Same
#define DISPLAY_SCK     GPIO_NUM_47  // Same
#define DISPLAY_MOSI    GPIO_NUM_48  // Same
#define DISPLAY_BL      GPIO_NUM_15  // Same

#define SPI_HOST SPI2_HOST
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
    send_command(ST7789_CASET);
    send_data(0); send_data(0); send_data(1); send_data(31);
    send_command(ST7789_RASET);
    send_data(0); send_data(0); send_data(0); send_data(239);
    send_command(ST7789_RAMWR);
    
    uint16_t swapped = ((color & 0xFF) << 8) | ((color >> 8) & 0xFF);
    uint8_t buffer[256];
    for (int i = 0; i < 256; i += 2) {
        buffer[i] = swapped >> 8;
        buffer[i+1] = swapped & 0xFF;
    }
    
    for (int i = 0; i < 300; i++) {
        spi_transaction_t t = {.tx_buffer = buffer, .length = 256 * 8};
        spi_device_polling_transmit(spi_handle, &t);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== Testing FNK0104A Pinout ===");
    ESP_LOGI(TAG, "CS=GPIO34, DC=GPIO39, RESET=GPIO46");
    ESP_LOGI(TAG, "If screen shows RED, your board is FNK0104A!");
    
    // Configure GPIO
    gpio_reset_pin(DISPLAY_DC);
    gpio_set_direction(DISPLAY_DC, GPIO_MODE_OUTPUT);
    gpio_reset_pin(DISPLAY_BL);
    gpio_set_direction(DISPLAY_BL, GPIO_MODE_OUTPUT);
    gpio_reset_pin(DISPLAY_RESET);
    gpio_set_direction(DISPLAY_RESET, GPIO_MODE_OUTPUT);
    
    // Initialize SPI
    spi_bus_config_t buscfg = {
        .mosi_io_num = DISPLAY_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = DISPLAY_SCK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = 320 * 240 * 2,
    };
    spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = DISPLAY_CS,
        .queue_size = 1,
    };
    spi_bus_add_device(SPI_HOST, &devcfg, &spi_handle);
    
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Reset display
    ESP_LOGI(TAG, "Resetting display...");
    gpio_set_level(DISPLAY_RESET, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(DISPLAY_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Init sequence
    ESP_LOGI(TAG, "Initializing display...");
    send_command(ST7789_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));
    send_command(ST7789_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));
    send_command(ST7789_MADCTL);
    send_data(0x00);
    send_command(ST7789_COLMOD);
    send_data(0x05);
    send_command(ST7789_INVON);
    send_command(ST7789_NORON);
    vTaskDelay(pdMS_TO_TICKS(10));
    send_command(ST7789_DISPON);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Backlight ON
    gpio_set_level(DISPLAY_BL, 1);
    ESP_LOGI(TAG, "Backlight ON");
    
    // Fill with RED
    ESP_LOGI(TAG, "Filling screen with RED...");
    fill_screen(0xF800);
    
    ESP_LOGI(TAG, "Screen should be RED now! Wait 5 seconds...");
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // Fill with GREEN
    ESP_LOGI(TAG, "Filling screen with GREEN...");
    fill_screen(0x07E0);
    
    ESP_LOGI(TAG, "Screen should be GREEN now!");
    
    // Done - blink backlight
    while (1) {
        gpio_set_level(DISPLAY_BL, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(DISPLAY_BL, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
