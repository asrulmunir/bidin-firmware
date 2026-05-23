/*
 * Audio Driver - I2S Microphone + Speaker
 * Minimal implementation for recording and playback
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int16_t samples[256];
    size_t count;
} audio_frame_t;

// Initialize audio (I2S mic + speaker)
void audio_init(void);

// Start recording from microphone
void audio_start_recording(void);

// Stop recording
void audio_stop_recording(void);

// Read audio frame from mic
audio_frame_t audio_read_frame(void);

// Play audio frame to speaker
void audio_play_frame(audio_frame_t *frame);

// Check if recording is active
bool audio_is_recording(void);

#endif // AUDIO_H
