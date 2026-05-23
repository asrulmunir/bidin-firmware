/*
 * Display Driver - ST7789 LCD
 * Minimal implementation for status display
 */

#ifndef DISPLAY_H
#define DISPLAY_H

// Initialize display
void display_init(void);

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
