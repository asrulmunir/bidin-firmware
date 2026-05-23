/*
 * Audio Driver Stub Implementation
 * TODO: Implement proper I2S mic + speaker driver
 */

#include "audio.h"
#include "esp_log.h"

static const char *TAG = "audio";

void audio_init(void)
{
    ESP_LOGI(TAG, "Audio initialized (stub)");
}

void audio_start_recording(void)
{
    ESP_LOGI(TAG, "Start recording (stub)");
}

void audio_stop_recording(void)
{
    ESP_LOGI(TAG, "Stop recording (stub)");
}

void audio_read_frame(audio_frame_t *frame)
{
    if (frame == NULL) return;
    
    // Stub: generate silence
    for (size_t i = 0; i < 256; i++) {
        frame->samples[i] = 0;
    }
    frame->count = 256;
}

void audio_play_frame(audio_frame_t *frame)
{
    if (frame == NULL) return;
    
    // Stub: do nothing
    ESP_LOGV(TAG, "Playing frame: %zu samples", frame->count);
}

bool audio_is_recording(void)
{
    return false;
}
