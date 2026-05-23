# Bidin Firmware Implementation Plan

## Status: Skeleton Created ✅

Project structure dah siap. Sekarang tinggal implement modules yang masih kosong.

---

## Phase 1: Audio Driver (CRITICAL - MVP)

**File:** `main/audio/audio.c`

### Tasks:
1. **I2S PDM Microphone Initialization**
   - Configure I2S peripheral for PDM input
   - Set sample rate: 16kHz, 16-bit, mono
   - GPIO: CLK=17, DATA=18

2. **I2S Speaker Initialization**
   - Configure I2S peripheral for standard I2S output
   - Set sample rate: 16kHz or 24kHz, 16-bit, mono
   - GPIO: BCLK=16, LRCK=15, DATA=14, MUTE=22

3. **Recording Functions**
   - `audio_start_recording()` - start I2S capture
   - `audio_read_frame()` - read 20ms frame (320 samples @ 16kHz)
   - `audio_stop_recording()` - stop capture

4. **Playback Functions**
   - `audio_play_frame()` - send samples to I2S DAC
   - Handle speaker mute control

### Reference:
- ESP-IDF I2S docs: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/i2s.html
- Xiaozhi audio code: `78/xiaozhi-esp32/main/audio/`

### Estimated Time: 4-6 hours

---

## Phase 2: Display Driver (CRITICAL - MVP)

**File:** `main/display/display.c`

### Tasks:
1. **SPI Initialization**
   - Configure SPI peripheral (HSPI or VSPI)
   - GPIO: SCK=47, MOSI=48, CS=49, DC=45, RST=21, BLK=15

2. **ST7789 Initialization Sequence**
   - Send init commands (from datasheet or reference)
   - Set resolution: 320x240
   - Enable inversion (if needed)

3. **Basic Primitives**
   - `draw_pixel(x, y, color)`
   - `draw_line(x1, y1, x2, y2, color)`
   - `fill_rect(x, y, w, h, color)`
   - `draw_char(x, y, char, color, bg)`
   - `draw_string(x, y, str, color, bg)`

4. **Status Screens**
   - `display_show_boot_screen()` - Bidin logo + version
   - `display_show_listening()` - Mic icon (recording)
   - `display_show_processing()` - Spinner/thinking
   - `display_show_speaking()` - Speaker icon (playing)
   - `display_show_text()` - Show server response

### Reference:
- ST7789 datasheet
- Xiaozhi display code: `78/xiaozhi-esp32/main/display/`
- LVGL (if we want fancy UI later)

### Estimated Time: 6-8 hours

---

## Phase 3: WiFi Driver (CRITICAL - MVP)

**File:** `main/network/wifi.c`

### Tasks:
1. **WiFi Station Mode**
   - Initialize WiFi in STA mode
   - Scan for networks
   - Connect to SSID from NVS

2. **NVS Credential Storage**
   - Store SSID + password in NVS
   - Support multiple networks (home, office)
   - Auto-reconnect on disconnect

3. **Connection Management**
   - `wifi_is_connected()` - check status
   - `wifi_get_rssi()` - signal strength
   - `wifi_get_ip_address()` - return IP as string
   - Reconnect logic with exponential backoff

### Reference:
- ESP-IDF WiFi docs: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/network/esp_wifi.html
- Xiaozhi WiFi code: `78/xiaozhi-esp32/main/network/`

### Estimated Time: 2-3 hours

---

## Phase 4: WebSocket Polish (DONE - Skeleton Ready)

**File:** `main/protocols/websocket.c`

### Status: ✅ Basic implementation complete

### Remaining Tasks:
1. **Audio Buffer for Playback**
   - Implement ring buffer for incoming TTS audio
   - `websocket_read_audio_frame()` - read from buffer
   - Handle chunked audio streaming

2. **Error Handling**
   - Reconnect on disconnect
   - Timeout handling
   - Error messages to display

### Estimated Time: 2-3 hours

---

## Phase 5: Integration Testing

### Test Scenarios:
1. **Boot Sequence**
   - [ ] Display shows boot screen
   - [ ] WiFi connects
   - [ ] WebSocket connects
   - [ ] Status shows "ready"

2. **Push-to-Talk**
   - [ ] Button press → "listening" screen
   - [ ] Audio streams to server
   - [ ] Button release → "processing" screen
   - [ ] Server response → "speaking" screen
   - [ ] Audio plays through speaker

3. **Error Cases**
   - [ ] WiFi disconnected → show error
   - [ ] Server unreachable → show error
   - [ ] Reconnect automatically

### Test Equipment:
- Freenove ESP32-S3 Display 2.8"
- USB-C cable
- Hermes plugin running on server
- Serial monitor for debugging

---

## Phase 6: Optimization (Optional)

### Size Optimization:
- [ ] Enable compiler optimizations (-Os)
- [ ] Remove unused ESP-IDF components
- [ ] Minimize PSRAM usage
- [ ] Target: <4MB firmware size

### Performance:
- [ ] Reduce audio latency (<200ms round-trip)
- [ ] Optimize display refresh rate
- [ ] Reduce WiFi reconnection time

### Power Management:
- [ ] Deep sleep when idle
- [ ] Dynamic CPU frequency scaling
- [ ] Display auto-off after timeout

---

## Build & Flash Workflow

```bash
# First time setup
cd bidin-firmware
./build.sh  # Will set target + build

# Flash to device
./build.sh flash

# Monitor serial output
./build.sh monitor

# Rebuild from scratch
./build.sh rebuild
```

---

## Dependencies

### ESP-IDF Components Needed:
- `driver/i2s` - Audio
- `driver/spi_master` - Display
- `esp_wifi` - WiFi
- `esp_http_client` - WebSocket
- `nvs_flash` - Non-volatile storage
- `cJSON` - JSON parsing

### External Libraries (if needed):
- OPUS encoding (if server doesn't handle raw PCM)
- Font for display (built-in ESP-IDF fonts should work)

---

## Next Action Items

1. **Install ESP-IDF** (if not already done)
   ```bash
   git clone -b v5.1.2 --recursive https://github.com/espressif/esp-idf.git
   cd esp-idf
   ./install.sh esp32s3
   . ./export.sh
   ```

2. **Verify build environment**
   ```bash
   cd bidin-firmware
   ./build.sh
   # Should fail with "audio.c not found" - that's expected
   ```

3. **Start with Audio Driver** (Phase 1)
   - Create `main/audio/audio.c`
   - Implement I2S init + recording
   - Test with serial monitor

---

## Notes

- **Keep it simple** - Don't add features we don't need
- **Test incrementally** - Build + test each module before moving on
- **Document as we go** - Update README with lessons learned
- **Reference Xiaozhi code** - But don't copy-paste blindly, understand first

---

**Last Updated:** May 23, 2026
**Version:** 1.0.0 (skeleton)
