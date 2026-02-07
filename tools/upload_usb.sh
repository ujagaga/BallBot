#!/usr/bin/env bash

PROJECT_NAME="ESP32_cam"
ESP_TOOL="$HOME/.arduino15/packages/esp32/tools/esptool_py/5.1.0/esptool"
BOOTLOADER_PATH="$HOME/.arduino15/packages/esp32/hardware/esp32/3.3.6/tools/partitions/boot_app0.bin"

# Get the path of the script as it was called (might be a symlink)
SCRIPT_PATH="$BASH_SOURCE"
# Resolve the symlink, if it is one, to get the actual file path
while [ -h "$SCRIPT_PATH" ]; do
  SCRIPT_PATH=$(readlink "$SCRIPT_PATH")
done
# Get the directory of the resolved script path
SCRIPT_DIR=$(dirname "${SCRIPT_PATH}")

cd "${SCRIPT_DIR}"
echo "Uploading from ../build"

${ESP_TOOL} --chip esp32 --port /dev/ttyUSB0 --baud 460800 \
--before default-reset --after hard-reset write-flash  -z \
--flash-mode keep --flash-freq keep --flash-size keep \
0x1000 ../build/${PROJECT_NAME}.ino.bootloader.bin \
0x8000 ../build/${PROJECT_NAME}.ino.partitions.bin \
0xe000 ${BOOTLOADER_PATH} \
0x10000 ../build/${PROJECT_NAME}.ino.bin 
