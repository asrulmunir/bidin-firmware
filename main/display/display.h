/*
 * Display driver header for ILI9341 LCD
 */

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdint.h>
#include <esp_lcd_panel_ops.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize display
void display_init(void);

// Fill screen with color
void display_fill(uint16_t color);

// Show boot screen (color test)
void display_show_boot_screen(void);

#ifdef __cplusplus
}
#endif

#endif // _DISPLAY_H_
