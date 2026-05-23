/*
 * Board Configuration for Bidin Firmware - Freenove ESP32-S3 Display 2.8"
 * Verified pinout - NO GPIO CONFLICTS
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

// GPIO Pin Definitions - NO CONFLICTS!
// Each GPIO used only ONCE

// Display (SPI)
#define DISPLAY_SPI_SCK_PIN     GPIO_NUM_47
#define DISPLAY_SPI_MOSI_PIN    GPIO_NUM_48
#define DISPLAY_SPI_CS_PIN      GPIO_NUM_39  // Unique SPI CS
#define DISPLAY_DC_PIN          GPIO_NUM_45
#define DISPLAY_RESET_PIN       GPIO_NUM_21
#define DISPLAY_BACKLIGHT_PIN   GPIO_NUM_38  // PWM backlight

// Touch (XPT2046 via SPI)
#define TOUCH_SPI_SCK_PIN       DISPLAY_SPI_SCK_PIN  // Shared SCK OK
#define TOUCH_SPI_MOSI_PIN      DISPLAY_SPI_MOSI_PIN  // Shared MOSI OK
#define TOUCH_SPI_MISO_PIN      GPIO_NUM_40  // Touch MISO (input only)
#define TOUCH_SPI_CS_PIN        GPIO_NUM_41  // Unique touch CS
#define TOUCH_IRQ_PIN           GPIO_NUM_42  // Touch interrupt

// Audio (I2S)
// Microphone - I2S PDM
#define AUDIO_MIC_PDM_CLK_PIN   GPIO_NUM_17
#define AUDIO_MIC_PDM_DATA_PIN  GPIO_NUM_18

// Speaker - I2S
#define AUDIO_SPEAKER_BCLK_PIN  GPIO_NUM_16
#define AUDIO_SPEAKER_LRCK_PIN  GPIO_NUM_15
#define AUDIO_SPEAKER_DATA_PIN  GPIO_NUM_14
#define AUDIO_SPEAKER_MUTE_PIN  GPIO_NUM_43  // Unique mute pin

// Board identification
#define BOARD_DEVICE_NAME "Bidin-ESP32S3"
#define BOARD_DEVICE_MODEL "Freenove-ESP32S3-2.8"
#define BOARD_FIRMWARE_VERSION "1.0.0"

// Buttons
#define BOARD_BUTTON_BOOT_PIN   GPIO_NUM_0      // BOOT button (active low)

// I2C (for sensors)
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
