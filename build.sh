#!/bin/bash
# Bidin Firmware Build Script
# Usage: ./build.sh [flash|monitor|clean]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if ESP-IDF is installed
if [ -z "$IDF_PATH" ]; then
    echo -e "${RED}❌ ESP-IDF not found!${NC}"
    echo ""
    echo "Please install ESP-IDF v5.1 or later:"
    echo "  git clone -b v5.1.2 --recursive https://github.com/espressif/esp-idf.git"
    echo "  cd esp-idf && ./install.sh esp32s3"
    echo "  . ./export.sh"
    echo ""
    exit 1
fi

# Check if target is set
if [ ! -f "sdkconfig" ]; then
    echo -e "${YELLOW}⚙️  First build - setting target to ESP32-S3...${NC}"
    idf.py set-target esp32s3
fi

case "${1:-build}" in
    build)
        echo -e "${GREEN}🔨 Building Bidin Firmware...${NC}"
        idf.py build
        echo -e "${GREEN}✅ Build complete!${NC}"
        echo ""
        echo "To flash: ./build.sh flash"
        echo "To monitor: ./build.sh monitor"
        ;;
    flash)
        echo -e "${GREEN}📦 Flashing to device...${NC}"
        idf.py flash
        ;;
    monitor)
        echo -e "${GREEN}📊 Starting serial monitor...${NC}"
        idf.py monitor
        ;;
    clean)
        echo -e "${YELLOW}🧹 Cleaning build artifacts...${NC}"
        idf.py fullclean
        ;;
    rebuild)
        echo -e "${YELLOW}🔄 Rebuilding from scratch...${NC}"
        idf.py fullclean
        idf.py build
        ;;
    *)
        echo "Bidin Firmware Build Script"
        echo ""
        echo "Usage: ./build.sh [command]"
        echo ""
        echo "Commands:"
        echo "  build    - Build firmware (default)"
        echo "  flash    - Flash to device"
        echo "  monitor  - Open serial monitor"
        echo "  clean    - Remove build artifacts"
        echo "  rebuild  - Clean and rebuild"
        echo ""
        ;;
esac
