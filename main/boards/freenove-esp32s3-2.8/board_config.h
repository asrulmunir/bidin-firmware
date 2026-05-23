/*
 * Board Configuration for Bidin Firmware - Freenove ESP32-S3 Display 2.8"
 * Based on: https://github.com/78/xiaozhi-esp32/tree/main/main/boards/freenove-esp32s3-display-2.8-lcd
 */

#ifndef BIDIN_BOARD_CONFIG_H
#define BIDIN_BOARD_CONFIG_H

#include <stdbool.h>
#include <driver/gpio.h>

// Board identification
#define BOARD_NAME "bidin-firmware"
#define BOARD_FIRMWARE_VERSION "1.0.0"
#define BOARD_DISPLAY_TYPE "ST7789"
#define BOARD_HAS_TOUCH

// GPIO Pin Definitions (from official Freenove FNK0104B specs)
// Display (SPI)
#define DISPLAY_SPI_SCK_PIN     GPIO_NUM_47
#define DISPLAY_SPI_MOSI_PIN    GPIO_NUM_48
#define DISPLAY_SPI_CS_PIN      GPIO_NUM_49  // Note: Some boards use GPIO_49, check your schematic
#define DISPLAY_DC_PIN          GPIO_NUM_45
#define DISPLAY_RESET_PIN       GPIO_NUM_21
#define DISPLAY_BACKLIGHT_PIN   GPIO_NUM_15

// Touch (XPT2046 via SPI - shared with display)
#define TOUCH_SPI_SCK_PIN       DISPLAY_SPI_SCK_PIN
#define TOUCH_SPI_MOSI_PIN      DISPLAY_SPI_MOSI_PIN
#define TOUCH_SPI_MISO_PIN      GPIO_NUM_46
#define TOUCH_SPI_CS_PIN        GPIO_NUM_46
#define TOUCH_IRQ_PIN           GPIO_NUM_46

// Audio (I2S)
// Microphone - I2S PDM
#define AUDIO_MIC_PDM_CLK_PIN   GPIO_NUM_17
#define AUDIO_MIC_PDM_DATA_PIN  GPIO_NUM_18

// Speaker - I2S
#define AUDIO_SPEAKER_BCLK_PIN  GPIO_NUM_16
#define AUDIO_SPEAKER_LRCK_PIN  GPIO_NUM_15
#define AUDIO_SPEAKER_DATA_PIN  GPIO_NUM_14
// GPIO_NUM_22 not available on all ESP32-S3 boards, use GPIO_47 instead
#define AUDIO_SPEAKER_MUTE_PIN  GPIO_NUM_47

// Buttons
#define BOARD_BUTTON_BOOT_PIN   GPIO_NUM_0      // BOOT button (active low)
#define BOARD_BUTTON_TOUCH_PIN  GPIO_NUM_21     // Touch sensor (if available)

// I2C (for sensors, if any)
#define I2C_SDA_PIN             GPIO_NUM_1
#define I2C_SCL_PIN             GPIO_NUM_2

// Audio Configuration
#define AUDIO_SAMPLE_RATE       16000
#define AUDIO_BITS_PER_SAMPLE   16
#define AUDIO_CHANNELS          1

// Display Configuration
#define DISPLAY_WIDTH           320
#define DISPLAY_HEIGHT          240
#define DISPLAY_INVERT          true

// Power Management
#define BATTERY_ADC_PIN         GPIO_NUM_3      // Battery voltage monitoring
#define BATTERY_ADC_ATTEN       ADC_ATTEN_DB_11

// Board initialization function
void board_init(void);

// Button state
bool board_button_pressed(void);

// Battery level (0-100)
int board_get_battery_level(void);

#endif // BIDIN_BOARD_CONFIG_H
