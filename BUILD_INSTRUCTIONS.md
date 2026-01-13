# AT-USB LoRa Dongle - Build Instructions

This project supports multiple hardware variants with different RF frequencies and oscillator types.

## Hardware Variants

The project supports 4 hardware configurations:

| Variant | Frequency | Oscillator | Define |
|---------|-----------|------------|--------|
| 868 XTAL | 868 MHz | XTAL | `HW_RF_868_XTAL` |
| 868 TCXO | 868 MHz | TCXO | `HW_RF_868_TCXO` |
| 915 XTAL | 915 MHz | XTAL | `HW_RF_915_XTAL` |
| 915 TCXO | 915 MHz | TCXO | `HW_RF_915_TCXO` |

## Build Methods

### Method 1: CMake Presets (Recommended)

Use CMake presets for easy variant selection:

```bash
# List available presets
cmake --list-presets

# Configure and build a specific variant
cmake --preset 868-tcxo-debug
cmake --build build/868-tcxo-debug

# Or use the preset for building too
cmake --build --preset 868-tcxo-debug
```

Available presets:
- `868-xtal-debug` / `868-xtal-release`
- `868-tcxo-debug` / `868-tcxo-release`
- `915-xtal-debug` / `915-xtal-release`
- `915-tcxo-debug` / `915-tcxo-release`

### Method 2: Build Scripts

#### Windows:
```batch
REM Build single variant
build_hw_variant.bat 915_TCXO Debug

REM Build all variants
build_all_variants.bat
```

#### Linux/Mac:
```bash
# Build single variant
./build_hw_variant.sh 915-tcxo debug

# Make sure script is executable
chmod +x build_hw_variant.sh
```

### Method 3: Manual CMake

```bash
# Configure with specific variant
cmake -B build -DHW_VARIANT=868_TCXO -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build
```

### Method 4: Direct Compiler Flag

Pass the define directly to the compiler via CMakeLists.txt or command line:

```bash
cmake -B build -DHW_RF_868_TCXO=ON
```

## Default Variant

If no hardware variant is specified during build, the default is **915 MHz TCXO**.

The build will show a warning message:
```
"No HW variant defined via build system, using default: HW_RF_915_TCXO"
```

## Output

The firmware will display the selected hardware configuration at startup:

```
==========================================================
              AT-USB LoRa Dongle
==========================================================
  Device Name:  Ropxion AT-USB LoRa_Dongle
  Firmware:     v1.0.0
  Version:      1.0.0
  Hardware:     915 MHz TCXO
==========================================================
```

## VSCode Integration

If using VSCode with CMake Tools extension:

1. Open Command Palette (Ctrl+Shift+P)
2. Select "CMake: Select Configure Preset"
3. Choose your desired hardware variant
4. Build normally (F7 or CMake: Build)

## Adding New Hardware Variants

To add a new hardware variant:

1. Edit `Core/Inc/hw_config.h` and add new defines
2. Update `CMakeLists.txt` to add new variant mapping
3. Add new presets to `CMakePresets.json`
4. Update this documentation

## Troubleshooting

**Problem:** Build fails with "No hardware variant defined" error

**Solution:** Make sure you're specifying the hardware variant using one of the methods above.

**Problem:** Wrong frequency or oscillator configuration

**Solution:** Check that the correct variant is being selected. The firmware output at startup shows the active configuration.
