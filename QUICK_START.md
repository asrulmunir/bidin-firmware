# Bidin Firmware - Quick Start Guide

## Apa Yang Dah Siap ✅

Project skeleton untuk **Bidin Firmware** dah ready! Ini yang dah ada:

### Files Created:
```
bidin-firmware/
├── CMakeLists.txt              ✅ Build configuration
├── idf_component.yml           ✅ Dependencies
├── sdkconfig.defaults          ✅ SDK settings (WiFi, audio, display)
├── README.md                   ✅ Full documentation
├── IMPLEMENTATION_PLAN.md      ✅ Step-by-step implementation guide
├── build.sh                    ✅ Build script (build/flash/monitor)
├── .gitignore                  ✅ Git ignore rules
└── main/
    ├── main.c                  ✅ Main app (WiFi → WebSocket → Audio loop)
    ├── boards/
    │   └── freenove-esp32s3-2.8/
    │       ├── board_config.h  ✅ Pin definitions
    │       └── board.c         ✅ Board initialization
    ├── audio/
    │   └── audio.h             ✅ Audio API (TODO: implement audio.c)
    ├── display/
    │   └── display.h           ✅ Display API (TODO: implement display.c)
    ├── network/
    │   └── wifi.h              ✅ WiFi API (TODO: implement wifi.c)
    └── protocols/
        ├── websocket.h         ✅ WebSocket API
        └── websocket.c         ✅ WebSocket client (READY)
```

### Features Yang Dah Ready:
- ✅ **Board configuration** - Freenove ESP32-S3 Display 2.8" pinout complete
- ✅ **WebSocket client** - Connect to Hermes plugin, send/receive audio
- ✅ **Main application flow** - Button → Record → Stream → Play
- ✅ **Build system** - ESP-IDF CMake setup
- ✅ **Documentation** - README + implementation plan

### Yang Masih Kosong (Perlu Implement):
- ⏳ `audio.c` - I2S microphone + speaker driver
- ⏳ `display.c` - ST7789 LCD driver
- ⏳ `wifi.c` - WiFi station mode driver

---

## Nak Mula Macam Mana?

### Step 1: Install ESP-IDF (Jika Belum)

```bash
# Clone ESP-IDF v5.1.2
git clone -b v5.1.2 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf

# Install for ESP32-S3
./install.sh esp32s3

# Load environment
. ./export.sh

# Verify
idf.py --version
```

### Step 2: Build Project

```bash
cd ~/bidin-firmware

# First build (will set target + compile)
./build.sh

# Expected output:
# - Will create sdkconfig
# - Will compile all .c files
# - Will fail on missing audio.c, display.c, wifi.c (EXPECTED!)
```

### Step 3: Implement Modules (ikut IMPLEMENTATION_PLAN.md)

**Phase 1: Audio Driver** (4-6 jam)
```bash
# Create main/audio/audio.c
# Implement:
# - I2S PDM mic recording
# - I2S speaker playback
# - audio_read_frame(), audio_play_frame()
```

**Phase 2: Display Driver** (6-8 jam)
```bash
# Create main/display/display.c
# Implement:
# - SPI initialization
# - ST7789 init sequence
# - draw_pixel(), draw_string(), status screens
```

**Phase 3: WiFi Driver** (2-3 jam)
```bash
# Create main/network/wifi.c
# Implement:
# - WiFi STA mode
# - NVS credential storage
# - wifi_connect(), wifi_is_connected()
```

### Step 4: Test Dengan Hermes Plugin

Pastikan Hermes plugin dah running:
```bash
# Check plugin status
hermes plugins list | grep xiaozhi

# Check port 8000
ss -tlnp | grep 8000
```

Then flash firmware:
```bash
cd ~/bidin-firmware
./build.sh flash
./build.sh monitor
```

### Step 5: Debug & Iterate

Guna serial monitor untuk debug:
```bash
# Monitor akan show:
# - Boot sequence
# - WiFi connection status
# - WebSocket connection
# - Audio streaming events
# - Errors (if any)
```

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│  Bidin Firmware (ESP32-S3)                           │
├─────────────────────────────────────────────────────┤
│                                                     │
│  [Button Press]                                     │
│       ↓                                             │
│  [Audio Recording] → I2S PDM Mic (16kHz)           │
│       ↓                                             │
│  [WebSocket Client] → Binary OPUS frames           │
│       ↓                                             │
│  [WiFi STA Mode] → ws://hermes.tetupai.com:8000   │
│                                                     │
└─────────────────────────────────────────────────────┘
                          ↓
                          ↓ WebSocket
                          ↓
┌─────────────────────────────────────────────────────┐
│  Hermes Plugin (xiaozhi-esp32)                      │
├─────────────────────────────────────────────────────┤
│                                                     │
│  [WebSocket Server] ← Receive audio frames         │
│       ↓                                             │
│  [VAD] → Detect speech end                          │
│       ↓                                             │
│  [ASR] → Whisper → Text                            │
│       ↓                                             │
│  [Hermes Agent] → LLM response                     │
│       ↓                                             │
│  [TTS] → Edge TTS → Audio                          │
│       ↓                                             │
│  [WebSocket] → Send audio back to ESP32            │
│                                                     │
└─────────────────────────────────────────────────────┘
```

---

## Comparison: Bidin vs Official Xiaozhi

| Aspect | Official Xiaozhi | Bidin Firmware |
|--------|------------------|---------------|
| **Code Size** | ~15MB | ~3MB (target) |
| **Lines of Code** | ~50,000 | ~2,000 (target) |
| **Board Support** | 70+ boards | Freenove only |
| **Display** | LVGL complex | Minimal status |
| **Protocols** | WebSocket, MQTT, MCP | WebSocket only |
| **Audio** | Multiple codecs | I2S PCM only |
| **Features** | Wake word, VAD, intents | Push-to-talk |
| **Server** | xiaozhi.me | Hermes plugin |
| **Build Time** | 5-10 min | 1-2 min (target) |
| **Complexity** | High | Minimal |

**Key Insight:** Kita buang semua benda yang tak perlu:
- ❌ MCP server (kita tak perlu device control)
- ❌ Multiple board configs (satu board je)
- ❌ Wake word detection (guna button je)
- ❌ Complex UI (status icons je cukup)
- ❌ Multiple protocols (WebSocket only)

Result: **Firmware yang simple, senang debug, easy maintain.**

---

## Troubleshooting

### Build Fails
```bash
# Clean and rebuild
./build.sh rebuild

# Check ESP-IDF version
idf.py --version  # Should be 5.1.x
```

### Flash Fails
```bash
# Check USB connection
lsusb | grep ESP32

# Try different USB port
# Some ports are charge-only
```

### WebSocket Won't Connect
```bash
# Check Hermes plugin is running
hermes gateway status

# Check port 8000
ss -tlnp | grep 8000

# Check firewall
sudo ufw status | grep 8000
```

### Audio Not Working
```bash
# Check I2S pin configuration
# See main/boards/freenove-esp32s3-2.8/board_config.h

# Verify with oscilloscope/logic analyzer
# Check I2S signals on pins 14-18
```

---

## Next Steps

1. **Install ESP-IDF** (if not done)
2. **Verify build works** (`./build.sh`)
3. **Implement audio.c** (Phase 1)
4. **Test recording** with serial monitor
5. **Implement display.c** (Phase 2)
6. **Implement wifi.c** (Phase 3)
7. **Full integration test** with Hermes plugin

---

## Resources

- **ESP-IDF Docs:** https://docs.espressif.com/projects/esp-idf/
- **Freenove Datasheet:** See skill `xiaozhi-esp32-server` → `references/freenove-fnk0104b-specs.md`
- **Xiaozhi Reference:** https://github.com/78/xiaozhi-esp32
- **Hermes Plugin:** `~/.hermes/plugins/xiaozhi-esp32/`

---

**Last Updated:** May 23, 2026  
**Version:** 1.0.0 (skeleton)  
**Status:** Ready for implementation
