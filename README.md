# AT_LoRa_Dongle

**Long-range wireless communication via simple text commands** — add LoRa to any project with just a serial connection.

Think of it like a WiFi dongle, but for long-distance LoRa communication. Connect → Send AT commands → Done.

**Use cases:** Remote sensors, IoT monitoring, long-range data collection, LoRa prototyping  
**No programming required** — just simple text AT commands

## Hardware

| Component | Specification |
|-----------|---------------|
| **MCU** | STM32L071CBT6 (ARM M0+, 192KB Flash, 20KB RAM) |
| **RF Chip** | SX1262 LoRa transceiver |
| **Freq Range** | 150 MHz - 960 MHz |
| **Optimized** | **869.525 MHz** (full power, impedance matched) |
| **Interface** | UART 115200 baud (USART1) |
| **Programming** | SWD |

**⚠️ Note:** Current revision optimized for **869.525 MHz**. Other frequencies supported but may not achieve full power. Future variants planned (915 MHz, etc.).

## Software Architecture

- **OS:** FreeRTOS (2 main tasks)
  - **TaskMain** — AT command processing & system control
  - **TaskRF** — LoRa radio operations
- **Drivers:** STM32 HAL Library
- **Build:** CMake
- **Communication:** Custom AT command parser

## AT Commands Reference

### Control Commands
| Command | Description |
|---------|------------|
| `AT` | Test connectivity |
| `AT+IDENTIFY` | Device identification |
| `AT+HELP` | List commands |
| `AT+LED_BLUE=ON\|OFF` | Control LED |
| `AT+SYS_RESTART` | Reboot device |

### TX Configuration
| Command | Description | Values |
|---------|------------|--------|
| `AT+LR_TX_FREQ` | Frequency | Hz (869525000 for 869.525 MHz) |
| `AT+LR_TX_POWER` | Output power | dBm |
| `AT+LR_TX_SF` | Spreading factor | 5-12 |
| `AT+LR_TX_BW` | Bandwidth | 7810, 10420, 15630, 20830, 31250, 41670, 62500, **125000**, 250000, 500000 Hz |
| `AT+LR_TX_CR` | Coding rate | 45, 46, 47, 48 |
| `AT+LR_TX_CRC` | CRC | 0=off, 1=on |
| `AT+LR_TX_HEADERMODE` | Header | 0=explicit, 1=implicit |

### RX Configuration
Same as TX (replace `TX` with `RX`)

### Bulk Config (convenience commands)
```
AT+LR_TX_SET=SF:9,BW:7,CR:45,Freq:869525000,Power:22
AT+LR_RX_SET=SF:9,BW:7,CR:45,Freq:869525000
```

### Data Transmission
| Command | Description |
|---------|------------|
| `AT+RF_TX_TXT=<text>` | Send text |
| `AT+RF_TX_HEX=<hex>` | Send hex data |
| `AT+RF_TX_SAVE_PCKT=<hex>` | Save to NVM |
| `AT+RF_TX_FROM_NVM=1` | Transmit from NVM |

### Periodic TX
| Command | Description |
|---------|------------|
| `AT+RF_TX_PERIOD=<ms>` | Set period |
| `AT+RF_TX_PERIOD_CTRL=ON\|OFF` | Start/stop |
| `AT+RF_TX_PERIOD_STATUS?` | Check status |

### Reception
| Command | Description |
|---------|------------|
| `AT+RF_RX_TO_UART=ON\|OFF` | Forward RX to UART |

### GPIO
| Command | Description |
|---------|------------|
| `AT+AUX=<pin>,ON\|OFF` | Set GPIO |
| `AT+AUX_PULSE=<pin>,<period_ms>,<duty_%>` | PWM output |
| `AT+AUX_PULSE_STOP=<pin>` | Stop PWM |

## Quick Start

**Connect:** USB serial adapter at **115200 baud**

**Test:**
```
AT                    # Response: OK or help
AT+IDENTIFY           # Get device info
```

**Configure LoRa (optimized for 869.525 MHz):**
```
AT+LR_TX_FREQ=869525000    # TX frequency
AT+LR_TX_POWER=14          # TX power (dBm)
AT+LR_TX_SF=7              # Spreading factor 5-12
AT+LR_TX_BW=125000         # Bandwidth (Hz)
AT+LR_RX_FREQ=869525000    # RX frequency
```

**Send data:**
```
AT+RF_TX_TXT=Hello World              # Text
AT+RF_TX_HEX=48656C6C6F               # Hex
AT+RF_RX_TO_UART=ON                   # Receive to UART
```

**Periodic transmission:**
```
AT+RF_TX_PERIOD=5000            # Period (ms)
AT+RF_TX_PERIOD_CTRL=ON         # Start
AT+RF_TX_PERIOD_CTRL=OFF        # Stop
```

## Build & Flash

**Prerequisites:**
- STM32CubeCLT or STM32CubeIDE
- ARM GCC toolchain
- CMake 3.22+
- STM32_Programmer_CLI

**Build:**
```bash
cmake -B build -S .
cmake --build build
```

**Flash (SWD):**
```bash
STM32_Programmer_CLI --connect port=swd --download build/Debug/AT_LoRa_Dongle.elf -hardRst -rst --start
```
Or use VSCode task: "CubeProg: Flash project (SWD)"

## Project Structure

```
├── Core/              # STM32 HAL initialization & system
├── Drivers/           # STM32 HAL peripheral drivers
├── Middlewares/       # FreeRTOS kernel
├── Modules/
│   ├── ATInterface/   # AT command parser
│   ├── NVMA/         # Non-volatile memory management
│   ├── RF/           # LoRa driver (SX1262)
│   └── Tasks/        # FreeRTOS tasks (MainTask, RFTask)
├── cmake/            # Build configuration
└── build/            # Compiled output
```

**Adding new AT commands:**
1. Define command in `AT_cmd.h`
2. Add entry in `AT_Commands[]` (AT_cmd.c)
3. Implement handler
4. Update README

## Troubleshooting

| Issue | Solution |
|-------|----------|
| **No response to AT commands** | Check UART connection, baud rate 115200, power supply. Try `AT` |
| **LoRa TX not working** | Verify frequency 150-960 MHz (best: **869.525 MHz**). Check antenna. Verify power limits. |
| **Build errors** | Install dependencies, check CMake 3.22+, verify ARM GCC. |
| **Factory reset** | `AT+FACTORY_MODE=ON` → `AT+SYS_RESTART` |

## License & Support

License: See individual file headers  
Support: Check documentation or create an issue
