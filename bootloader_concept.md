# Bootloader Concept for AT_LoRa_Dongle

## Architecture

### Memory Layout
```
Flash Memory (192KB total):
0x08000000 - 0x08003FFF : Bootloader (16KB)
0x08004000 - 0x0802FFFF : Application FW (176KB) 
0x08030000 - 0x0802FFFF : Config/Backup area (4KB)

RAM (20KB):
0x20000000 - 0x200003FF : Bootloader variables (1KB)
0x20000400 - 0x20004FFF : Application RAM (19KB)
```

### Boot Process
1. **Power-On/Reset** → Bootloader starts at 0x08000000
2. **Check boot condition:**
   - GPIO pin state (e.g., button pressed during reset)
   - Magic value in RAM (set by application via AT command)
   - Corrupted application detection
3. **Normal boot:** Jump to application at 0x08004000
4. **Bootloader mode:** Stay in bootloader, initialize UART, wait for commands

## AT Commands for Firmware Update

### New bootloader-specific AT commands:
```
AT+BL_ENTER          - Enter bootloader mode (set flag and restart)
AT+BL_STATUS         - Get bootloader info and current mode  
AT+FW_UPDATE_START   - Begin firmware update process
AT+FW_UPDATE_DATA=<hex> - Send firmware data chunk (max 256 bytes)
AT+FW_UPDATE_END     - Finish update and verify
AT+FW_VERIFY         - Verify current application integrity
AT+BL_VERSION        - Get bootloader version
```

### Update Process Flow:
```
1. AT+BL_ENTER → Device restarts in bootloader mode
2. AT+FW_UPDATE_START → Erase application area, prepare for data
3. Multiple AT+FW_UPDATE_DATA=<hex> → Write data chunks to flash
4. AT+FW_UPDATE_END → Verify CRC, set valid flag, restart to new FW
```

## Implementation Details

### Bootloader Features:
- **Size**: ~8-12KB (leaving 4-8KB margin)
- **UART**: Same as application (115200 baud, USART1)
- **Flash writing**: HAL_FLASH_Program()
- **CRC verification**: Hardware CRC32 unit
- **Failsafe**: Keep bootloader always functional

### Bootloader Code Structure:
```c
// bootloader_main.c
int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    if (should_enter_bootloader()) {
        bootloader_mode();  // AT command processing
    } else {
        jump_to_application();
    }
}

bool should_enter_bootloader(void) {
    // Check GPIO pin, RAM flag, or application validity
    return (boot_pin_pressed() || 
            ram_flag_set() || 
            !application_valid());
}

void jump_to_application(void) {
    uint32_t app_stack = *(uint32_t*)APP_START_ADDR;
    uint32_t app_entry = *(uint32_t*)(APP_START_ADDR + 4);
    
    __set_MSP(app_stack);
    ((void(*)())app_entry)();
}
```

### Application Modifications:
```c
// Add to AT_cmd.c command table:
{"AT+BL_ENTER", AT_HandleBootloaderEnter, 0, "AT+BL_ENTER - Enter bootloader mode", ""},

void AT_HandleBootloaderEnter(char *params) {
    // Set magic flag in RAM
    *((uint32_t*)BOOTLOADER_FLAG_ADDR) = BOOTLOADER_MAGIC;
    AT_SendStringResponse("Entering bootloader mode...\r\n");
    HAL_Delay(100);
    NVIC_SystemReset();
}
```

## Update Tools

### Python Update Script:
```python
# fw_update.py
import serial
import time

def update_firmware(port, fw_file):
    ser = serial.Serial(port, 115200)
    
    # Enter bootloader
    ser.write(b'AT+BL_ENTER\r\n')
    time.sleep(2)
    
    # Start update
    ser.write(b'AT+FW_UPDATE_START\r\n')
    
    # Send firmware data
    with open(fw_file, 'rb') as f:
        while True:
            chunk = f.read(128)  # 128 bytes per chunk
            if not chunk:
                break
            hex_data = chunk.hex().upper()
            cmd = f'AT+FW_UPDATE_DATA={hex_data}\r\n'
            ser.write(cmd.encode())
            # Wait for OK response
    
    # Finish update
    ser.write(b'AT+FW_UPDATE_END\r\n')
```

## Build System Integration

### CMakeLists.txt modifications:
```cmake
# Bootloader target
add_executable(bootloader ${BOOTLOADER_SOURCES})
set_target_properties(bootloader PROPERTIES 
    LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/STM32L071CBTx_BOOTLOADER.ld")

# Application target (modified linker script)  
set_target_properties(AT_LoRa_Dongle PROPERTIES
    LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/STM32L071CBTx_APP.ld")

# Combined firmware target
add_custom_target(combined_fw
    COMMAND srec_cat bootloader.bin -binary -offset 0x08000000 
                     AT_LoRa_Dongle.bin -binary -offset 0x08004000 
                     -o combined.bin -binary
    DEPENDS bootloader AT_LoRa_Dongle)
```

## Advantages of This Approach:

1. **Familiar interface**: Uses existing AT command system
2. **Safe**: Bootloader always remains functional
3. **Flexible**: Can update via any device with serial port
4. **Robust**: CRC verification and failsafe mechanisms
5. **Small footprint**: Reuses existing UART/AT infrastructure

## Security Considerations:

- Add simple authentication (password/key)
- Implement encrypted firmware support
- Add rollback capability
- Protect bootloader area from accidental writes
