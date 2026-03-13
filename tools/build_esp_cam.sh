#!/usr/bin/env bash

# Get the path of the script as it was called (might be a symlink)
SCRIPT_PATH="$BASH_SOURCE"
# Resolve the symlink, if it is one, to get the actual file path
while [ -h "$SCRIPT_PATH" ]; do
  SCRIPT_PATH=$(readlink "$SCRIPT_PATH")
done
# Get the directory of the resolved script path
SCRIPT_DIR=$(dirname "$SCRIPT_PATH")

cd $SCRIPT_DIR
rm -rf ../build

# Copy custom partition table to Arduino package directory
PARTITION_DIR="$HOME/.arduino15/packages/esp32/hardware/esp32/3.3.7/tools/partitions"
cp partitions.csv "$PARTITION_DIR/custom.csv"

echo "Building ESP32_cam"
/usr/local/bin/arduino-cli compile --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashMode=qio,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi \
--build-path ../build \
--build-property "build.partitions=app3M_fat9M_16MB" ../ESP32_cam
