#!/bin/bash
# Build script for AT-USB LoRa Dongle - Hardware Variant Builder
# Usage: ./build_hw_variant.sh [variant] [build_type]
#   variant: 868-xtal, 868-tcxo, 915-xtal, 915-tcxo (default: 915-tcxo)
#   build_type: debug, release (default: debug)

set -e

# Set defaults
VARIANT=${1:-915-tcxo}
BUILD_TYPE=${2:-debug}

# Convert to lowercase
VARIANT=$(echo "$VARIANT" | tr '[:upper:]' '[:lower:]')
BUILD_TYPE=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')

# Convert underscores to dashes if needed
VARIANT=${VARIANT//_/-}

PRESET="${VARIANT}-${BUILD_TYPE}"

echo "========================================"
echo "Building AT-USB LoRa Dongle"
echo "Hardware Variant: ${VARIANT}"
echo "Build Type: ${BUILD_TYPE}"
echo "CMake Preset: ${PRESET}"
echo "========================================"
echo

# Configure
echo "Configuring..."
cmake --preset "${PRESET}"

echo
echo "Building..."
cmake --build "build/${PRESET}" -j

echo
echo "========================================"
echo "Build completed successfully!"
echo "Output: build/${PRESET}/CmakeTest.elf"
echo "========================================"
