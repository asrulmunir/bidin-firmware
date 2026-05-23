/*
 * Audio Driver - I2S Microphone + Speaker
 * Minimal implementation for recording and playback
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Audio frame structure (raw PCM samples)
typedef struct {
    int16_t samples[256];  // PCM samples
    size_t count;          // Number of valid samples
} audio_frame_t;

// Initialize audio (I2S mic + speaker)
void audio_init(void);

// Start recording from microphone
void audio_start_recording(void);

// Stop recording
void audio_stop_recording(void);

// Read audio frame from mic (fills the frame)
void audio_read_frame(audio_frame_t *frame);

// Play audio frame to speaker
void audio_play_frame(audio_frame_t *frame);

// Check if recording is active
bool audio_is_recording(void);

#endif // AUDIO_H
