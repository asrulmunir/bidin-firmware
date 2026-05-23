# Bidin Firmware

**Bidin - Voice Assistant Firmware** - Minimal ESP32-S3 Voice Assistant Firmware

## Overview

Firmware minimal untuk ESP32-S3 voice assistant yang connect ke Hermes plugin. Dibina dari kosong dengan ESP-IDF, tak ada legacy code, tak ada features yang tak perlu.

## Features

- ✅ **Minimal** - Hanya apa yang perlu: WiFi, WebSocket, audio I/O, display
- ✅ **Kontrol penuh** - Kita decide latency, buffer size, reconnection logic
- ✅ **Senang debug** - Code kita sendiri, senang trace issue
- ✅ **Hermes integration** - Connect terus ke Hermes plugin (bukan xiaozhi.me server)

## Hardware Support

### Freenove ESP32-S3 Display 2.8" (FNK0104B)

**Specs:**
- MCU: ESP32-S3
- Display: 2.8" TFT LCD (320x240), ST7789 driver
- Touch: XPT2046 resistive touch
- Microphone: I2S PDM MEMS
- Speaker: I2S amplifier
- Button: BOOT button (GPIO0)
- Storage: 8MB Flash, 2MB PSRAM

**Pinout:**
```
Display (SPI):
  SCK:  GPIO47
  MOSI: GPIO48
  CS:   GPIO49
  DC:   GPIO45
  RST:  GPIO21
  BLK:  GPIO15

Touch (SPI):
  MISO: GPIO46
  IRQ:  GPIO46

Audio:
  MIC CLK:  GPIO17
  MIC DATA: GPIO18
  SPK BCLK: GPIO16
  SPK LRCK: GPIO15
  SPK DATA: GPIO14
  SPK MUTE: GPIO22

Buttons:
  BOOT: GPIO0 (active low)

I2C:
  SDA: GPIO1
  SCL: GPIO2

Battery:
  ADC:  GPIO3
```

## Project Structure

```
bidin-firmware/
├── CMakeLists.txt              # ESP-IDF build config
├── idf_component.yml           # Component dependencies
├── sdkconfig.defaults          # Default SDK config
├── main/
│   ├── main.c                  # Application entry point
│   ├── boards/
│   │   └── freenove-esp32s3-2.8/
│   │       ├── board_config.h  # Pin definitions
│   │       └── board.c         # Board initialization
│   ├── audio/
│   │   ├── audio.h             # Audio API
│   │   └── audio.c             # I2S driver (TODO)
│   ├── display/
│   │   ├── display.h           # Display API
│   │   └── display.c           # ST7789 driver (TODO)
│   ├── network/
│   │   ├── wifi.h              # WiFi API
│   │   └── wifi.c              # WiFi driver (TODO)
│   └── protocols/
│       ├── websocket.h         # WebSocket API
│       └── websocket.c         # WebSocket client
```

## Build Instructions

### Prerequisites

1. **Install ESP-IDF** (v5.0 or later):
   ```bash
   # Follow official guide: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/
   git clone -b v5.1.2 --recursive https://github.com/espressif/esp-idf.git
   cd esp-idf
   ./install.sh esp32s3
   . ./export.sh
   ```

2. **Verify installation:**
   ```bash
   idf.py --version
   ```

### Build

```bash
cd bidin-firmware
idf.py set-target esp32s3
idf.py build
```

### Flash

**Freenove FNK0104B has USB-C data support** - flash via USB-C cable:

```bash
idf.py flash
idf.py monitor
```

### Clean Build

```bash
idf.py fullclean
idf.py build
```

## Configuration

Edit `sdkconfig.defaults` untuk customize:

```bash
# WebSocket server URL
CONFIG_Bidin_WEBSOCKET_URL="ws://hermes.tetupai.com:8000/xiaozhi/v1/"

# Audio settings
CONFIG_AUDIO_SAMPLE_RATE=16000
CONFIG_AUDIO_BITS_PER_SAMPLE=16

# Display settings
CONFIG_DISPLAY_WIDTH=320
CONFIG_DISPLAY_HEIGHT=240
```

## WebSocket Protocol

Firmware connect ke Hermes plugin via WebSocket. Message format:

### Device → Server

**Hello (registration):**
```json
{
  "type": "hello",
  "version": "v1",
  "device_id": "freenove-esp32s3-2.8"
}
```

**Audio start:**
```json
{
  "type": "listen",
  "state": "start"
}
```

**Audio frame:** Binary OPUS data (256 bytes)

**Audio end:**
```json
{
  "type": "listen",
  "state": "end"
}
```

### Server → Device

**TTS start:**
```json
{
  "type": "tts",
  "state": "start"
}
```

**TTS audio:** Binary OPUS data

**TTS end:**
```json
{
  "type": "tts",
  "state": "end"
}
```

## TODO / Next Steps

### Critical (MVP)
- [ ] Implement I2S audio driver (`main/audio/audio.c`)
  - [ ] PDM microphone recording
  - [ ] I2S speaker playback
  - [ ] OPUS encoding (if needed)
- [ ] Implement ST7789 display driver (`main/display/display.c`)
  - [ ] SPI initialization
  - [ ] Basic primitives (draw pixel, line, text)
  - [ ] Status screens (listening, processing, speaking)
- [ ] Implement WiFi driver (`main/network/wifi.c`)
  - [ ] Station mode connection
  - [ ] NVS credential storage
  - [ ] Reconnection logic

### Nice to Have
- [ ] Battery level monitoring (ADC)
- [ ] Touch screen support (XPT2046)
- [ ] OTA firmware updates
- [ ] Wake word detection (on-device)
- [ ] Volume control buttons
- [ ] LED / RGB indicator

### Testing
- [ ] Mock Hermes plugin untuk testing
- [ ] Unit tests untuk audio pipeline
- [ ] Integration tests dengan real hardware

## Comparison with Official Xiaozhi Firmware

| Aspect | Official Xiaozhi | Bidin Firmware |
|--------|------------------|---------------|
| Size | ~15MB | ~3MB (target) |
| Board support | 70+ boards | Freenove only |
| Protocols | WebSocket, MQTT, MCP | WebSocket only |
| Display | Complex LVGL | Minimal status |
| Audio | Multiple codecs | I2S only |
| Server | xiaozhi.me | Hermes plugin |
| Features | Wake word, VAD, intents | Push-to-talk |
| Build time | 5-10 min | 1-2 min (target) |

## License

MIT License - Free for personal and commercial use

## Credits

Built with ESP-IDF v5.1.2
Inspired by xiaozhi-esp32 project
