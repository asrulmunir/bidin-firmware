/*
 * Display Driver - ST7789 LCD
 * Minimal implementation for status display
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Display dimensions
#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

// Color definitions (RGB565)
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

// Initialize display
void display_init(void);

// Fill screen with color
void display_fill(uint16_t color);

// Draw pixel at (x, y)
void display_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

// Draw string
void display_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color);

// Draw rectangle
void display_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

// Fill rectangle
void display_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
// Show boot screen
void display_show_boot_screen(void);

// Show "listening" state (recording)
void display_show_listening(void);

// Show "processing" state (waiting for response)
void display_show_processing(void);

// Show "speaking" state (playing response)
void display_show_speaking(void);

// Show text message
void display_show_text(const char *text);

// Clear display
void display_clear(void);

// Set brightness (0-100)
void display_set_brightness(int level);

#endif // DISPLAY_H
