# AT_LoRa_Dongle

A simple USB-like device that adds LoRa wireless communication to any computer or microcontroller project. Just connect via serial port and send simple text commands to transmit and receive data over long distances.

## What is this device?

The AT_LoRa_Dongle is a small electronic module that makes it easy to add long-range wireless communication to your projects. Think of it like a WiFi dongle, but instead of connecting to the internet, it sends data directly to other LoRa devices up to several kilometers away.

**Perfect for:**
- Remote sensor monitoring
- IoT projects requiring long-range communication
- Wireless data collection from remote locations
- Testing and prototyping LoRa applications
- Educational purposes and learning LoRa technology

**How it works:**
1. Connect the dongle to your computer or microcontroller via serial cable
2. Send simple text commands (like "AT+RF_TX_TXT=Hello") 
3. The device transmits your data wirelessly using LoRa technology
4. Other LoRa devices can receive your messages from kilometers away
5. Received messages are automatically forwarded back to your computer

No complex programming required - just simple text commands!

## Technical Details

Built with professional components for reliable performance:

- **STM32L071CBT6** ultra-low-power microcontroller
- **SX1262** LoRa transceiver (150 MHz - 960 MHz)
- **FreeRTOS** real-time operating system
- **AT command interface** via UART
- **Comprehensive LoRa parameter control**
- **Periodic transmission capability**
- **Data storage in NVM (Non-Volatile Memory)**
- **Auxiliary GPIO control**
- **Factory mode support**

## Hardware Specifications

- **MCU**: STM32L071CBT6 (ARM Cortex-M0+, 192KB Flash, 20KB RAM)
- **RF Transceiver**: SX1262
- **Frequency Range**: 150 MHz - 960 MHz
- **Optimized Frequency**: 869.525 MHz (full power output with proper impedance matching)
- **Communication Interface**: UART (USART1)
- **Programming Interface**: SWD
- **Note**: Current hardware revision is optimized for 869.525 MHz. Other frequencies are supported but may not achieve full power output due to antenna/RF matching. Future hardware variants for other frequency bands (e.g., 915 MHz) are planned.

## Software Architecture

The firmware is built using:
- **HAL Library**: STM32 Hardware Abstraction Layer
- **FreeRTOS**: Real-time operating system with two main tasks:
  - `TaskMain`: Handles AT commands and system management
  - `TaskRF`: Manages LoRa radio operations
- **CMake**: Build system
- **AT Command Parser**: Custom implementation for command processing

## AT Commands Reference

### Basic Commands

| Command | Description | Parameters |
|---------|-------------|------------|
| `AT` | Basic test command | - |
| `AT+HELP` | List all supported commands | - |
| `AT+IDENTIFY` | Identify the device | - |
| `AT+FACTORY_MODE` | Enable/disable factory mode | `=ON`, `=OFF` |
| `AT+SYS_RESTART` | Restart the system | - |
| `AT+LED_BLUE` | Control blue LED | `=ON`, `=OFF` |

### LoRa Configuration Commands

#### Transmission Parameters
| Command | Description | Parameters |
|---------|-------------|------------|
| `AT+LR_TX_FREQ` | Set TX frequency | `=<frequency_in_Hz>`, `?` |
| `AT+LR_TX_POWER` | Set TX power | `=<power_in_dBm>`, `?` |
| `AT+LR_TX_SF` | Set TX spreading factor | `=5` to `12`, `?` |
| `AT+LR_TX_BW` | Set TX bandwidth | See bandwidth values below |
| `AT+LR_TX_CR` | Set TX coding rate | `=45`, `=46`, `=47`, `=48`, `?` |
| `AT+LR_TX_IQ_INV` | Set TX IQ inversion | `=1`, `=0`, `?` |
| `AT+LR_TX_HEADERMODE` | Set TX header mode | `=1` (implicit), `=0` (explicit), `?` |
| `AT+LR_TX_CRC` | Enable TX CRC | `=1`, `=0`, `?` |
| `AT+LR_TX_PREAMBLE_SIZE` | Set TX preamble size | `=<1 to 65535>`, `?` |

#### Reception Parameters
| Command | Description | Parameters |
|---------|-------------|------------|
| `AT+LR_RX_FREQ` | Set RX frequency | `=<frequency_in_Hz>`, `?` |
| `AT+LR_RX_SF` | Set RX spreading factor | `=5` to `12`, `?` |
| `AT+LR_RX_BW` | Set RX bandwidth | See bandwidth values below |
| `AT+LR_RX_CR` | Set RX coding rate | `=45`, `=46`, `=47`, `=48`, `?` |
| `AT+LR_RX_IQ_INV` | Set RX IQ inversion | `=1`, `=0`, `?` |
| `AT+LR_RX_HEADERMODE` | Set RX header mode | `=1` (implicit), `=0` (explicit), `?` |
| `AT+LR_RX_CRC` | Enable RX CRC | `=1`, `=0`, `?` |
| `AT+LR_RX_PREAMBLE_SIZE` | Set RX preamble size | `=<1 to 65535>`, `?` |

#### Bandwidth Values
The bandwidth parameter accepts the following values:
- `7810` (7.81 kHz) - BW 0
- `10420` (10.42 kHz) - BW 1
- `15630` (15.63 kHz) - BW 2
- `20830` (20.83 kHz) - BW 3
- `31250` (31.25 kHz) - BW 4
- `41670` (41.67 kHz) - BW 5
- `62500` (62.5 kHz) - BW 6
- `125000` (125 kHz) - BW 7
- `250000` (250 kHz) - BW 8
- `500000` (500 kHz) - BW 9

#### Bulk Configuration Commands
| Command | Description |
|---------|-------------|
| `AT+LR_TX_SET` | Set multiple TX parameters in one command |
| `AT+LR_RX_SET` | Set multiple RX parameters in one command |

**Example:**
```
AT+LR_TX_SET=SF:9,BW:7,CR:45,Freq:869525000,IQ:0,Header:0,CRC:1,Power:22
AT+LR_RX_SET=SF:9,BW:7,CR:45,Freq:869525000,IQ:1,Header:0,CRC:1
```

### Data Transmission Commands

| Command | Description | Parameters |
|---------|-------------|------------|
| `AT+RF_TX_HEX` | Transmit data in HEX format | `=<HEX data>` |
| `AT+RF_TX_TXT` | Transmit data in text format | `=<Text data>` |
| `AT+RF_TX_FROM_NVM` | Transmit saved packet from NVM | `=1` |
| `AT+RF_TX_SAVE_PCKT` | Save packet to NVM | `=<HEX data>`, `?` |

### Periodic Transmission

| Command | Description | Parameters |
|---------|-------------|------------|
| `AT+RF_TX_PERIOD` | Set transmission period | `=<period_ms>`, `?` |
| `AT+RF_TX_PERIOD_CTRL` | Start/stop periodic transmission | `=ON`, `=OFF`, `?` |
| `AT+RF_TX_PERIOD_STATUS` | Get periodic transmission status | `?` |

### Reception Control

| Command | Description | Parameters |
|---------|-------------|------------|
| `AT+RF_RX_TO_UART` | Forward received RF data to UART | `=ON`, `=OFF` |

### Auxiliary GPIO Control

| Command | Description | Parameters |
|---------|-------------|------------|
| `AT+AUX` | Set auxiliary pin state | `=<pin>,<ON\|OFF>` |
| `AT+AUX_PULSE` | Generate PWM on auxiliary pin | `=<pin>,<period_ms>,<duty_pct>` |
| `AT+AUX_PULSE_STOP` | Stop PWM on auxiliary pin | `=<pin>` |

## Usage Examples

### Basic Setup

1. **Connect to the device** via UART (typically 115200 baud)
2. **Test connectivity:**
   ```
   AT
   ```
   Response: `OK` or help information

3. **Identify device:**
   ```
   AT+IDENTIFY
   ```
   Response: `AT-LoRa_Dongle v1.0`

### Configure LoRa Parameters

```bash
# Set TX frequency to 869.525 MHz (optimized frequency)
AT+LR_TX_FREQ=869525000

# Set TX power to 14 dBm
AT+LR_TX_POWER=14

# Set spreading factor to 7
AT+LR_TX_SF=7

# Set bandwidth to 125 kHz
AT+LR_TX_BW=125000

# Configure RX to match TX settings
AT+LR_RX_FREQ=869525000
AT+LR_RX_SF=7
AT+LR_RX_BW=125000
```

### Send Data

```bash
# Send text data
AT+RF_TX_TXT=Hello World

# Send hex data
AT+RF_TX_HEX=48656C6C6F20576F726C64

# Save data to NVM and transmit later
AT+RF_TX_SAVE_PCKT=48656C6C6F
AT+RF_TX_FROM_NVM=1
```

### Periodic Transmission

```bash
# Set transmission period to 5 seconds
AT+RF_TX_PERIOD=5000

# Start periodic transmission
AT+RF_TX_PERIOD_CTRL=ON

# Check status
AT+RF_TX_PERIOD_STATUS?

# Stop periodic transmission
AT+RF_TX_PERIOD_CTRL=OFF
```

### Reception Mode

```bash
# Enable RX to UART forwarding
AT+RF_RX_TO_UART=ON

# All received LoRa packets will now be forwarded to UART
```

## Building and Flashing

### Prerequisites

- **STM32CubeCLT** or **STM32CubeIDE**
- **ARM GCC toolchain**
- **CMake** (version 3.22 or higher)
- **STM32_Programmer_CLI** for flashing

### Build Process

1. **Clone the repository**
2. **Configure the build:**
   ```bash
   cmake -B build -S .
   ```

3. **Build the project:**
   ```bash
   cmake --build build
   ```

### Flashing

Using the provided VSCode task:
```bash
# Flash via SWD
STM32_Programmer_CLI --connect port=swd --download build/Debug/AT_LoRa_Dongle.elf -hardRst -rst --start
```

Or use the VSCode task: "CubeProg: Flash project (SWD)"

## Development

### Project Structure

```
AT_LoRa_Dongle/
├── Core/                   # STM32 HAL core files
│   ├── Inc/               # Header files
│   └── Src/               # Source files
├── Drivers/               # STM32 HAL drivers
├── Middlewares/           # FreeRTOS
├── Modules/               # Application modules
│   ├── ATInterface/       # AT command parser
│   ├── NVMA/             # Non-volatile memory access
│   ├── RF/               # LoRa radio driver
│   └── Tasks/            # FreeRTOS tasks
│       ├── MainTask/     # Main application task
│       └── RFTask/       # Radio task
├── cmake/                # CMake configuration
└── build/               # Build output
```

### Adding New AT Commands

1. **Add command enum** in `AT_cmd.h`
2. **Add command entry** in `AT_Commands[]` table in `AT_cmd.c`
3. **Implement command handler** in appropriate module
4. **Update this README** with new command documentation

## Troubleshooting

### Common Issues

1. **Device not responding to AT commands:**
   - Check UART connection and baud rate
   - Verify device power supply
   - Try `AT` command for basic connectivity

2. **LoRa transmission not working:**
   - Verify frequency is within allowed range (150-960 MHz)
   - For optimal performance, use 869.525 MHz (hardware optimized frequency)
   - Check antenna connection
   - Ensure TX power is within limits

3. **Build errors:**
   - Ensure all dependencies are installed
   - Check CMake version compatibility
   - Verify ARM GCC toolchain installation

### Factory Reset

```bash
AT+FACTORY_MODE=ON
AT+SYS_RESTART
```

## License

This project is provided as-is. Check individual file headers for specific license information.

## Support

For issues and questions, please refer to the project documentation or create an issue in the project repository.
