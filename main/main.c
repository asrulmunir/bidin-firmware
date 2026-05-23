/*
 * GPIO Toggle Test for Freenove Display
 * This will manually toggle display pins to verify connection
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "gpio_test";

// Test ALL possible pin combinations
void app_main(void)
{
    printf("=== GPIO TOGGLE TEST START ===\n");
    
    // Test FNK0104B pins (most common)
    const int test_pins[] = {39, 45, 46, 47, 48, 15};
    const char *pin_names[] = {"CS", "DC", "RESET", "SCK", "MOSI", "BACKLIGHT"};
    
    printf("Testing FNK0104B pinout:\n");
    
    for (int i = 0; i < 6; i++) {
        int pin = test_pins[i];
        printf("  Testing GPIO%d (%s)... ", pin, pin_names[i]);
        
        gpio_reset_pin(pin);
        gpio_set_direction(pin, GPIO_MODE_OUTPUT);
        
        // Toggle 3 times
        for (int j = 0; j < 3; j++) {
            gpio_set_level(pin, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_set_level(pin, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        
        printf("DONE (toggled 3x)\n");
    }
    
    // Special test: Toggle backlight continuously
    printf("\nBacklight test: GPIO15 will blink every 1 second\n");
    printf("If screen lights up, backlight is working!\n");
    
    while (1) {
        gpio_set_level(15, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_set_level(15, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
