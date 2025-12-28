# AT_LoRa_Dongle

**Professional LoRa wireless communication module with AT command interface**

A hardware module for long-range wireless IoT communication. Control via simple serial AT commands — no complex programming required. Transmit, receive, and configure LoRa parameters just like you would with a modem.

**Core specifications:**
- **Communication:** Serial AT commands (115200 baud)
- **Hardware:** STM32L071CBT6 MCU + SX1262 LoRa transceiver
- **Frequency:** 869.525 MHz (optimized), 150-960 MHz supported
- **Output Power:** 0-22 dBm (configurable)
- **Range:** 15+ km line-of-sight
- **Integration:** FreeRTOS RTOS, message queues for inter-task communication
- **Storage:** Non-volatile memory (NVM) for packet caching

---

## Hardware Specifications

| Component | Value |
|-----------|-------|
| **MCU** | STM32L071CBT6 (ARM Cortex-M0+) |
| **Memory** | 192 KB Flash (176 KB application), 20 KB RAM |
| **RF Transceiver** | SX1262 (Semtech LoRa modem) |
| **Frequency Range** | 150 MHz - 960 MHz |
| **Frequency Optimized** | **869.525 MHz** (EU ISM band, full power) |
| **TX Power Range** | 0 to 22 dBm |
| **Serial Interface** | UART1 (115200 baud, 8N1) |
| **Programming** | SWD (Serial Wire Debug) |
| **Dimensions/Weight** | [hardware data TBD] |

**⚠️ Important:** Current hardware revision is **optimized for 869.525 MHz** with matched RF impedance matching network. Other frequencies (150-960 MHz) are supported via firmware but will not achieve full TX power output due to unmatched antenna/filter circuits. Future hardware variants for 915 MHz and other regional bands are planned.

---

## Software Architecture

```
┌────────────────────────────────────────────────────────────┐
│         Serial Port Interface (USART1 @ 115200 baud)       │
│              ↓                                              │
│         AT Command Parser (Modules/ATInterface/AT_cmd.c)   │
│              ↓                                              │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ FreeRTOS Task Scheduler                             │   │
│  │ ┌─────────────────────┐      ┌──────────────────┐  │   │
│  │ │ Main Task           │      │ RF Task          │  │   │
│  │ │ - AT command proc.  │◄────►│ - SX1262 driver  │  │   │
│  │ │ - System control    │      │ - TX/RX handling │  │   │
│  │ │ - GPIO/LED control  │      │ - RF state mgmt  │  │   │
│  │ └─────────────────────┘      └──────────────────┘  │   │
│  │          ↓                            ↓             │   │
│  │    ┌──────────────┐          ┌──────────────┐      │   │
│  │    │Message Queues│          │Synchronization│    │   │
│  │    │queueMainHandle          │Semaphores     │    │   │
│  │    │queueRadioHandle          │Mutexes        │    │   │
│  │    └──────────────┘          └──────────────┘     │   │
│  └─────────────────────────────────────────────────────┘   │
│              ↓                                              │
│  ┌────────────────────────────────────────────────────┐    │
│  │ Peripheral Drivers & Modules                       │    │
│  │ ┌──────────┐  ┌───────┐  ┌──────────┐  ┌────────┐ │    │
│  │ │GPIO Ctrl │  │SPI    │  │NVMA      │  │ SX1262 │ │    │
│  │ │LED/AUX   │  │ Driver │ │NV Memory │  │ Driver │ │    │
│  │ └──────────┘  └───────┘  └──────────┘  └────────┘ │    │
│  └────────────────────────────────────────────────────┘    │
└────────────────────────────────────────────────────────────┘
```

**Module breakdown:**
- **Modules/ATInterface/AT_cmd.c** — AT command parser and dispatcher
- **Modules/Tasks/MainTask/** — Main application task, system commands, GPIO control
- **Modules/Tasks/RFTask/** — Radio operations (TX/RX via SX1262)
- **Modules/RF/SX1262/** — SX1262 driver (Semtech provided)
- **Modules/NVMA/** — Non-volatile memory abstraction layer
- **Core/** — STM32 HAL initialization, peripheral setup

---

## AT Commands — Complete Reference

### Command Format
- **Case-insensitive:** `AT`, `at`, `At` all accepted
- **Set value:** `AT+COMMAND=value` (e.g., `AT+LR_TX_POWER=14`)
- **Query value:** `AT+COMMAND?` (returns current value)
- **Execute action:** `AT+COMMAND` (no params)

### System Control Commands

| Command | Usage | Response | Description |
|---------|-------|----------|-------------|
| `AT` | `AT` | `OK` | Connectivity test |
| `AT+HELP` | `AT+HELP` | [command list] | List all available commands |
| `AT+IDENTIFY` | `AT+IDENTIFY` | Device ID & version | Get device information |
| `AT+SYS_RESTART` | `AT+SYS_RESTART` | `OK` then restart | Restart system (hardware reset) |
| `AT+FACTORY_MODE` | `=ON` / `=OFF` | `OK` | Enable factory mode for testing |
| `AT+LED_BLUE` | `=ON` / `=OFF` | `OK` | Control blue status LED |

---

### LoRa TX Configuration

**Individual parameter commands:**

| Command | Parameter | Example | Notes |
|---------|-----------|---------|-------|
| `AT+LR_TX_FREQ` | Frequency (Hz) | `AT+LR_TX_FREQ=869525000` | 869.525 MHz for EU |
| `AT+LR_TX_POWER` | Power (dBm) | `AT+LR_TX_POWER=14` | 0-22 dBm range |
| `AT+LR_TX_SF` | Spreading Factor | `AT+LR_TX_SF=7` | 5-12; higher = longer range |
| `AT+LR_TX_BW` | Bandwidth (Hz) | `AT+LR_TX_BW=125000` | 7.81k to 500k Hz |
| `AT+LR_TX_CR` | Coding Rate | `AT+LR_TX_CR=45` | 45, 46, 47, 48 |
| `AT+LR_TX_CRC` | CRC Enable | `AT+LR_TX_CRC=1` | 0=off, 1=on |
| `AT+LR_TX_HEADERMODE` | Header Type | `AT+LR_TX_HEADERMODE=0` | 0=explicit, 1=implicit |
| `AT+LR_TX_IQ_INV` | IQ Invert | `AT+LR_TX_IQ_INV=0` | 0=normal, 1=inverted |
| `AT+LR_TX_PREAMBLE_SIZE` | Preamble Len | `AT+LR_TX_PREAMBLE_SIZE=8` | 1-65535; minimum 8 recommended |

**Query current value:**
```
AT+LR_TX_FREQ?          # Returns: 869525000
AT+LR_TX_POWER?         # Returns: 14
```

**Bulk TX configuration (set all at once):**
```
AT+LR_TX_SET=SF:7,BW:7,CR:45,Freq:869525000,IQ:0,Header:0,CRC:1,Power:14
```

---

### LoRa RX Configuration

**Same parameters as TX, replace `TX` with `RX`:**
- `AT+LR_RX_FREQ=<Hz>`
- `AT+LR_RX_POWER` (note: RX doesn't use power setting)
- `AT+LR_RX_SF=<5-12>`
- `AT+LR_RX_BW=<Hz>`
- `AT+LR_RX_CR=<45-48>`
- `AT+LR_RX_CRC=<0|1>`
- `AT+LR_RX_HEADERMODE=<0|1>`
- `AT+LR_RX_IQ_INV=<0|1>`
- `AT+LR_RX_PREAMBLE_SIZE=<1-65535>`

**Bulk RX configuration:**
```
AT+LR_RX_SET=SF:7,BW:7,CR:45,Freq:869525000,IQ:0,Header:0,CRC:1
```

---

### Bandwidth Reference

Standard LoRa bandwidth values (in Hz):

| Hz Value | kHz | Typical Use |
|----------|-----|------------|
| 7810 | 7.81 | Extreme range (slow) |
| 10420 | 10.42 | Very long range |
| 15630 | 15.63 | Long range |
| 20830 | 20.83 | Medium range |
| 31250 | 31.25 | Medium range |
| 41670 | 41.67 | Medium/short range |
| 62500 | 62.5 | Short range |
| **125000** | **125** | **Standard (recommended)** |
| 250000 | 250 | Short range, high data rate |
| 500000 | 500 | Very short range only |

---

### Data Transmission Commands

| Command | Format | Example | Purpose |
|---------|--------|---------|---------|
| `AT+RF_TX_TXT` | `=<text>` | `AT+RF_TX_TXT=Hello LoRa` | Send ASCII text via RF |
| `AT+RF_TX_HEX` | `=<hex>` | `AT+RF_TX_HEX=48656C6C6F` | Send binary data as hex |
| `AT+RF_TX_SAVE_PCKT` | `=<hex>` | `AT+RF_TX_SAVE_PCKT=DEADBEEF` | Save packet to NVM |
| `AT+RF_TX_FROM_NVM` | `=1` | `AT+RF_TX_FROM_NVM=1` | Transmit previously saved packet |

**Examples:**
```
AT+RF_TX_TXT=Sensor reading: 23.5°C
AT+RF_TX_HEX=010203AABBCC
AT+RF_TX_SAVE_PCKT=48656C6C6F
AT+RF_TX_FROM_NVM=1
```

---

### Periodic Transmission

Control automatic repeated transmission of data at fixed intervals:

| Command | Format | Example | Purpose |
|---------|--------|---------|---------|
| `AT+RF_TX_PERIOD` | `=<milliseconds>` | `AT+RF_TX_PERIOD=5000` | Set interval (5 seconds) |
| `AT+RF_TX_PERIOD_CTRL` | `=ON` / `=OFF` | `AT+RF_TX_PERIOD_CTRL=ON` | Start/stop periodic TX |
| `AT+RF_TX_PERIOD_STATUS` | `?` | `AT+RF_TX_PERIOD_STATUS?` | Check if periodic TX enabled |

**Periodic TX workflow:**
```
AT+RF_TX_PERIOD=5000                # Set 5-second interval
AT+RF_TX_SAVE_PCKT=48656C6C6F       # Save payload to NVM
AT+RF_TX_PERIOD_CTRL=ON             # Start periodic transmission
# Device will transmit every 5 seconds...
AT+RF_TX_PERIOD_STATUS?             # Check: enabled?
AT+RF_TX_PERIOD_CTRL=OFF            # Stop periodic transmission
```

---

### Reception Control

| Command | Format | Purpose |
|---------|--------|---------|
| `AT+RF_RX_TO_UART` | `=ON` / `=OFF` | Forward received packets to UART |

**RX mode example:**
```
AT+RF_RX_TO_UART=ON
# Now device listens and prints all received LoRa packets to UART
# Example received: +RX: 48656C6C6F20576F726C64
```

---

### Auxiliary GPIO Control

Directly control GPIO pins for external devices (relays, sensors, etc.):

| Command | Format | Example | Purpose |
|---------|--------|---------|---------|
| `AT+AUX` | `=<pin>,<ON\|OFF>` | `AT+AUX=1,ON` | Set GPIO high/low |
| `AT+AUX_PULSE` | `=<pin>,<period_ms>,<duty_%>` | `AT+AUX_PULSE=2,1000,50` | PWM output |
| `AT+AUX_PULSE_STOP` | `=<pin>` | `AT+AUX_PULSE_STOP=2` | Stop PWM |

**GPIO examples:**
```
AT+AUX=1,ON                 # Set pin 1 high (e.g., activate relay)
AT+AUX=1,OFF                # Set pin 1 low
AT+AUX_PULSE=2,1000,75      # PWM on pin 2: 1 kHz frequency, 75% duty cycle
AT+AUX_PULSE_STOP=2         # Stop PWM on pin 2
```

---

## Quick Start Guide

### 1. Hardware Connection

```
USB TTL Adapter         AT_LoRa_Dongle
─────────────────       ──────────────
GND ────────────────► GND
RXD ────────────────► TX (USART1)
TXD ────────────────► RX (USART1)
+5V/+3.3V ─────────► VCC
```

Configure terminal: **115200 baud, 8 data bits, 1 stop bit, no parity**

### 2. Basic Connectivity Test

```bash
# Test connection
AT
# Expected: OK

# Get device info
AT+IDENTIFY
# Expected: AT-LoRa_Dongle v1.0

# List all commands
AT+HELP
```

### 3. Configure LoRa Parameters (EU 869 MHz Example)

```bash
# Set TX parameters
AT+LR_TX_FREQ=869525000      # 869.525 MHz
AT+LR_TX_POWER=14            # 14 dBm
AT+LR_TX_SF=7                # SF7 (good compromise)
AT+LR_TX_BW=125000           # 125 kHz (standard)
AT+LR_TX_CR=45               # CR 4/5
AT+LR_TX_CRC=1               # Enable CRC

# Set RX to match TX (so you can receive your own messages)
AT+LR_RX_FREQ=869525000
AT+LR_RX_SF=7
AT+LR_RX_BW=125000
AT+LR_RX_CR=45
AT+LR_RX_CRC=1
```

**Or use bulk configuration:**
```bash
AT+LR_TX_SET=SF:7,BW:7,CR:45,Freq:869525000,IQ:0,Header:0,CRC:1,Power:14
AT+LR_RX_SET=SF:7,BW:7,CR:45,Freq:869525000,IQ:0,Header:0,CRC:1
```

### 4. Send Data

```bash
# Send text
AT+RF_TX_TXT=Hello World

# Send hex data
AT+RF_TX_HEX=48656C6C6F

# Or save and transmit from NVM
AT+RF_TX_SAVE_PCKT=DEADBEEF
AT+RF_TX_FROM_NVM=1
```

### 5. Receive Data

```bash
# Enable RX forwarding to UART
AT+RF_RX_TO_UART=ON

# Now device prints all received packets:
# +RX: 48656C6C6F20576F726C64
```

### 6. Periodic Transmission (e.g., sensor data every 10 seconds)

```bash
AT+RF_TX_SAVE_PCKT=DEADBEEF        # Save payload
AT+RF_TX_PERIOD=10000              # 10 seconds
AT+RF_TX_PERIOD_CTRL=ON            # Start periodic TX
AT+RF_TX_PERIOD_STATUS?            # Verify enabled

# To stop:
AT+RF_TX_PERIOD_CTRL=OFF
```

---

## Building and Flashing

### Prerequisites

- **STM32CubeCLT** or **STM32CubeIDE** (recommended)
- **ARM GCC toolchain** (arm-none-eabi-gcc)
- **CMake** 3.22 or later
- **STM32_Programmer_CLI** for flashing
- **Git** for version control

### Build Steps

1. **Clone repository:**
   ```bash
   git clone https://github.com/witc/AT_LoRa_Dongle.git
   cd AT_LoRa_Dongle
   ```

2. **Configure CMake build:**
   ```bash
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
   ```

3. **Build firmware:**
   ```bash
   cmake --build build --config Debug
   ```

   Output: `build/Debug/AT_LoRa_Dongle.elf`

### Flashing to Device

**Using VSCode task** (recommended):
1. Connect device via SWD
2. Run VSCode task: `CMake: Flash project (SWD)`

**Using CLI:**
```bash
STM32_Programmer_CLI --connect port=swd --download build/Debug/AT_LoRa_Dongle.elf -hardRst -rst --start
```

**Verify flash:**
```bash
STM32_Programmer_CLI --list
```

---

## Project Structure

```
AT_LoRa_Dongle/
├── README.md                    # This file
├── CMakeLists.txt              # Build configuration
├── AT_LoRa_Dongle.ioc          # STM32CubeMX project file
│
├── Core/                       # STM32 HAL & system
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32l0xx_hal_conf.h
│   │   ├── FreeRTOSConfig.h    # FreeRTOS settings
│   │   └── ...
│   └── Src/
│       ├── main.c              # Entry point, hardware init
│       ├── stm32l0xx_hal_msp.c # HAL callbacks
│       ├── system_stm32l0xx.c
│       └── ...
│
├── Drivers/                    # STM32 HAL drivers
│   ├── STM32L0xx_HAL_Driver/
│   └── CMSIS/
│
├── Middlewares/                # FreeRTOS kernel
│   └── Third_Party/FreeRTOS/
│
├── Modules/                    # Application code
│   ├── ATInterface/
│   │   ├── AT_cmd.h            # AT command definitions (enum eATCommands)
│   │   ├── AT_cmd.c            # AT command parser & dispatcher
│   │   └── SerialPort/         # UART abstraction layer
│   │
│   ├── Tasks/
│   │   ├── MainTask/
│   │   │   ├── Main_task.h
│   │   │   ├── Main_task.c     # Main FreeRTOS task
│   │   │   ├── general_sys_cmd.c
│   │   │   └── auxPin_logic.c  # GPIO/PWM control
│   │   │
│   │   └── RFTask/
│   │       ├── RF_Task.h
│   │       └── RF_Task.c       # RF transmission/reception task
│   │
│   ├── RF/                     # Radio driver
│   │   ├── Inc/
│   │   ├── Src/
│   │   │   └── radio_user.c
│   │   └── SX1262/             # Semtech SX1262 driver
│   │       ├── Inc/
│   │       ├── Src/
│   │       │   ├── sx126x.c
│   │       │   ├── ral_sx126x.c
│   │       │   └── ...
│   │       └── [Semtech licensed driver]
│   │
│   └── NVMA/                   # Non-volatile memory
│       ├── NVMA.h
│       └── NVMA.c              # Flash-based packet storage
│
├── cmake/                      # Build system
│   ├── gcc-arm-none-eabi.cmake
│   └── stm32cubemx/
│
├── build/                      # Build output (generated)
│   ├── Debug/
│   │   ├── AT_LoRa_Dongle.elf
│   │   ├── AT_LoRa_Dongle.map
│   │   └── compile_commands.json
│   └── Release/
│
└── .vscode/                    # VSCode settings
    ├── launch.json             # Debug configuration
    └── tasks.json              # Build/flash tasks
```

---

## Development Guide

### Adding a New AT Command

1. **Define command enum** in `Modules/ATInterface/AT_cmd.h`:
   ```c
   typedef enum {
       // ... existing commands
       SYS_CMD_MY_NEW_COMMAND = 99,
   } eATCommands;
   ```

2. **Add to command table** in `Modules/ATInterface/AT_cmd.c`:
   ```c
   const AT_Command_Struct AT_Commands[] = {
       // ... existing commands
       {"AT+MY_CMD", NULL, SYS_CMD_MY_NEW_COMMAND, "AT+MY_CMD - My new command", "parameters"},
   };
   ```

3. **Implement handler** in appropriate module (e.g., `Modules/Tasks/MainTask/general_sys_cmd.c`):
   ```c
   void AT_HandleMyCommand(char *params) {
       // Process command
       // Send response
       AT_SendStringResponse("OK\r\n");
   }
   ```

4. **Update README** with new command documentation

### Message Queue Communication

Main Task → RF Task via FreeRTOS queues:
```c
osMessageQueueId_t queueMainHandle;     // Send from MainTask
osMessageQueueId_t queueRadioHandle;    // Send from RFTask

// Send data from MainTask to RFTask
dataQueue_t msg = { ... };
osMessageQueuePut(queueRadioHandle, &msg, 0, 0);

// RF Task receives and processes
```

### Accessing Non-Volatile Memory

Save/load packets from flash:
```c
NVMA_SavePacket(buffer, length);
NVMA_LoadPacket(buffer, &length);
```

---

## Troubleshooting

| Problem | Diagnosis | Solution |
|---------|-----------|----------|
| **Device not responding** | No response to `AT` command | Check UART connection, baud rate (115200), power supply. Try `AT+IDENTIFY` |
| **Compilation errors** | Build fails | Verify CMake 3.22+, ARM GCC toolchain installed, all dependencies present |
| **LoRa TX not transmitting** | No RF output | Check frequency (optimized: 869.525 MHz), antenna connected, TX power set, spreading factor valid (5-12) |
| **Low TX power on other bands** | Reduced RF output on non-869 MHz | Hardware optimized for 869.525 MHz only. Expected on other frequencies. |
| **RX receiving only noise** | Can't decode valid messages | Verify RX parameters match TX settings (SF, BW, CR, frequency, header mode, CRC) |
| **Periodic TX not working** | TX doesn't repeat | Verify packet saved to NVM first, period set (ms), periodic TX enabled with `AT+RF_TX_PERIOD_CTRL=ON` |
| **Flash tool not found** | `STM32_Programmer_CLI` not in PATH | Install STM32CubeCLT, add to system PATH, or use VSCode task |
| **Device bricked** | Device unresponsive | Connect via SWD, flash fresh firmware using STM32CubeProgrammer GUI |

### Factory Reset

```bash
AT+FACTORY_MODE=ON
AT+SYS_RESTART
```

---

## API Reference (For Programmers)

### Main Task API (`Modules/Tasks/MainTask/Main_task.h`)

```c
/**
 * FreeRTOS main task entry point
 * Called by FreeRTOS scheduler
 */
void main_task(void);

/**
 * Send data from external source to main task
 * @param data: Message to send (dataQueue_t structure)
 * @return: true if successful, false if queue full
 */
bool MT_SendDataToMainTask(dataQueue_t *data);

/**
 * Handle custom AT commands
 * Called by AT parser when command matches
 * @param data: Command string
 * @param atCmd: Command enum value
 * @param size: Command length
 * @return: true if handled
 */
bool AT_CustomCommandHandler(char *data, eATCommands atCmd, uint16_t size);
```

### RF Task API (`Modules/Tasks/RFTask/RF_Task.h`)

```c
/**
 * FreeRTOS RF task entry point
 * Handles all LoRa radio operations
 */
void rf_task(void);
```

### AT Command API (`Modules/ATInterface/AT_cmd.h`)

```c
/**
 * Initialize AT command subsystem
 * @param atCmd: AT context with UART handle and buffers
 */
void AT_Init(AT_cmd_t *atCmd);

/**
 * Process incoming AT command (called from UART ISR)
 * @param size: Size of received data
 */
void AT_HandleATCommand(uint16_t size);

/**
 * Send string response to UART
 * @param response: Null-terminated response string
 */
void AT_SendStringResponse(char *response);
```

### NVMA API (`Modules/NVMA/NVMA.h`)

```c
/**
 * Save packet to non-volatile memory
 */
void NVMA_SavePacket(uint8_t *data, uint16_t length);

/**
 * Load packet from non-volatile memory
 */
void NVMA_LoadPacket(uint8_t *buffer, uint16_t *length);
```

---

## Performance Specifications

| Parameter | Value |
|-----------|-------|
| **Command Response Time** | <10 ms |
| **TX Latency** | ~100 ms (setup) + LoRa TX time |
| **RX Sensitivity** (SF7, BW125) | ~-120 dBm |
| **Current Consumption (RX)** | ~80 mA |
| **Current Consumption (TX @ 22 dBm)** | ~500 mA peak |
| **Packet Size** | 1-255 bytes |

---

## License & Attribution

- **Firmware:** [Your License Here]
- **SX1262 Driver:** Semtech (see `Modules/RF/SX1262/LICENSE`)
- **FreeRTOS:** Real Time Engineers Ltd
- **STM32 HAL:** STMicroelectronics

---

## Support & Contact

For issues, questions, or contributions:
- Create an issue on GitHub
- Check existing documentation
- Review code comments in relevant module

**Last Updated:** November 2024
**Firmware Version:** v1.0
**Hardware Revision:** Rev 1.0 (869.525 MHz)
