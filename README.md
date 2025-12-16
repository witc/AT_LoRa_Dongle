# AT_LoRa_Dongle

**LoRa komunikační modul s AT příkazy pro bezdrátový přenos na velké vzdálenosti**

Ovládej LoRa transceiver pomocí jednoduchých sériových AT příkazů – žádné programování, funguje jako modem. Posílej data, nastavuj parametry a přijímej zprávy na dálku přes UART.

**Hlavní vlastnosti:**
- **Ovládání:** Sériové AT příkazy (115200 baud)
- **Hardware:** STM32L071CBT6 + SX1262 LoRa transceiver
- **Frekvence:** 869.525 MHz (optimalizováno), podporuje 150-960 MHz
- **Výkon:** 0-22 dBm (nastavitelné)
- **Dosah:** 15+ km (přímá viditelnost)
- **RTOS:** FreeRTOS, komunikace přes message queues
- **Paměť:** Non-volatile storage pro ukládání paketů

---

## Hardware

| Komponent | Hodnota |
|-----------|---------|
| **MCU** | STM32L071CBT6 (ARM Cortex-M0+) |
| **Flash / RAM** | 176 KB / 20 KB |
| **RF** | SX1262 (Semtech LoRa) |
| **Frekvence** | 150-960 MHz |
| **Optimální frekvence** | **869.525 MHz** (EU ISM band) |
| **TX výkon** | 0 až 22 dBm |
| **UART** | 115200 baud, 8N1 |
| **Programování** | SWD |

**⚠️ Důležité:** Hardware je optimalizován pro **869.525 MHz** s přizpůsobenou impedancí. Jiné frekvence (150-960 MHz) fungují, ale nedosáhnou plného výkonu kvůli neoptimalizovanému RF filtru. Další hw varianty pro 915 MHz jsou v plánu.

---

## Softwarová architektura

```
┌────────────────────────────────────────────────────────────┐
│         UART (115200 baud) ← Serial AT příkazy            │
│              ↓                                              │
│         AT Parser (AT_cmd.c)                               │
│              ↓                                              │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ FreeRTOS Scheduler                                  │   │
│  │ ┌─────────────────────┐      ┌──────────────────┐  │   │
│  │ │ Main Task           │      │ RF Task          │  │   │
│  │ │ - AT příkazy        │◄────►│ - SX1262 driver  │  │   │
│  │ │ - Systém            │      │ - TX/RX          │  │   │
│  │ │ - GPIO/LED          │      │                  │  │   │
│  │ └─────────────────────┘      └──────────────────┘  │   │
│  │          ↓                            ↓             │   │
│  │    ┌──────────────┐          ┌──────────────┐      │   │
│  │    │Message Queues│          │Semaphores    │      │   │
│  │    └──────────────┘          └──────────────┘      │   │
│  └─────────────────────────────────────────────────────┘   │
│              ↓                                              │
│  ┌────────────────────────────────────────────────────┐    │
│  │ Hardware Drivers                                   │    │
│  │ GPIO | SPI | NVMA | SX1262                         │    │
│  └────────────────────────────────────────────────────┘    │
└────────────────────────────────────────────────────────────┘
```

**Moduly:**
- **ATInterface/** — Parser a dispatcher AT příkazů
- **Tasks/MainTask/** — Hlavní task, systémové příkazy, GPIO
- **Tasks/RFTask/** — Radio operace (TX/RX)
- **RF/SX1262/** — SX1262 driver (Semtech)
- **NVMA/** — Non-volatile paměť
- **Core/** — STM32 HAL inicializace

---

## AT Příkazy – kompletní přehled

### Formát příkazů
- **Case-insensitive:** `AT`, `at`, `At` jsou stejné
- **Nastavit hodnotu:** `AT+COMMAND=value`
- **Zjistit hodnotu:** `AT+COMMAND?`
- **Provést akci:** `AT+COMMAND`

### Systémové příkazy

| Příkaz | Použití | Odpověď | Popis |
|---------|---------|----------|-------|
| `AT` | `AT` | `OK` | Test spojení |
| `AT+HELP` | `AT+HELP` | [seznam příkazů] | Zobrazí všechny příkazy |
| `AT+IDENTIFY` | `AT+IDENTIFY` | Info o zařízení | ID a verze |
| `AT+SYS_RESTART` | `AT+SYS_RESTART` | `OK` → restart | Hardware reset |
| `AT+FACTORY_MODE` | `=ON` / `=OFF` | `OK` | Tovární režim |
| `AT+LED_BLUE` | `=ON` / `=OFF` | `OK` | Ovládání modré LED |

---

### LoRa TX konfigurace

**Jednotlivé parametry:**

| Příkaz | Parametr | Příklad | Poznámka |
|---------|-----------|---------|----------|
| `AT+LR_TX_FREQ` | Frekvence (Hz) | `AT+LR_TX_FREQ=869525000` | 869.525 MHz pro EU |
| `AT+LR_TX_POWER` | Výkon (dBm) | `AT+LR_TX_POWER=14` | 0-22 dBm |
| `AT+LR_TX_SF` | Spreading Factor | `AT+LR_TX_SF=7` | 5-12; vyšší = větší dosah |
| `AT+LR_TX_BW` | Šířka pásma (Hz) | `AT+LR_TX_BW=125000` | 7.81k až 500k Hz |
| `AT+LR_TX_CR` | Coding Rate | `AT+LR_TX_CR=45` | 45, 46, 47, 48 |
| `AT+LR_TX_CRC` | CRC Enable | `AT+LR_TX_CRC=1` | 0=vypnuto, 1=zapnuto |
| `AT+LR_TX_HEADERMODE` | Typ hlavičky | `AT+LR_TX_HEADERMODE=0` | 0=explicit, 1=implicit |
| `AT+LR_TX_IQ_INV` | IQ Invert | `AT+LR_TX_IQ_INV=0` | 0=normální, 1=invertováno |
| `AT+LR_TX_PREAMBLE_SIZE` | Délka preambule | `AT+LR_TX_PREAMBLE_SIZE=8` | 1-65535; min. 8 |

**Dotaz na hodnotu:**
```
AT+LR_TX_FREQ?          # Vrátí: 869525000
AT+LR_TX_POWER?         # Vrátí: 14
```

**Hromadné nastavení TX (vše najednou):**
```
AT+LR_TX_SET=SF:7,BW:7,CR:45,Freq:869525000,IQ:0,Header:0,CRC:1,Power:14
```

---

### LoRa RX konfigurace

**Stejné parametry jako TX, nahraď `TX` za `RX`:**
- `AT+LR_RX_FREQ=<Hz>`
- `AT+LR_RX_SF=<5-12>`
- `AT+LR_RX_BW=<Hz>`
- `AT+LR_RX_CR=<45-48>`
- `AT+LR_RX_CRC=<0|1>`
- `AT+LR_RX_HEADERMODE=<0|1>`
- `AT+LR_RX_IQ_INV=<0|1>`
- `AT+LR_RX_PREAMBLE_SIZE=<1-65535>`

**Hromadné nastavení RX:**
```
AT+LR_RX_SET=SF:7,BW:7,CR:45,Freq:869525000,IQ:0,Header:0,CRC:1
```

---

### Referenční tabulka šířky pásma

| Hz | kHz | Použití |
|----------|-----|------------|
| 7810 | 7.81 | Maximální dosah (pomalé) |
| 10420 | 10.42 | Velmi dlouhý dosah |
| 15630 | 15.63 | Dlouhý dosah |
| 20830 | 20.83 | Střední dosah |
| 31250 | 31.25 | Střední dosah |
| 41670 | 41.67 | Kratší dosah |
| 62500 | 62.5 | Krátký dosah |
| **125000** | **125** | **Standardní (doporučeno)** |
| 250000 | 250 | Krátký dosah, vysoká rychlost |
| 500000 | 500 | Velmi krátký dosah |

---

### Odesílání dat

| Příkaz | Formát | Příklad | Účel |
|---------|--------|---------|---------|
| `AT+RF_TX_TXT` | `=<text>` | `AT+RF_TX_TXT=Hello LoRa` | Odeslat ASCII text |
| `AT+RF_TX_HEX` | `=<hex>` | `AT+RF_TX_HEX=48656C6C6F` | Odeslat binární data (hex) |
| `AT+RF_TX_SAVE_PCKT` | `=<hex>` | `AT+RF_TX_SAVE_PCKT=DEADBEEF` | Uložit paket do NVM |
| `AT+RF_TX_FROM_NVM` | `=1` | `AT+RF_TX_FROM_NVM=1` | Odeslat uložený paket |

**Příklady:**
```
AT+RF_TX_TXT=Teplota: 23.5°C
AT+RF_TX_HEX=010203AABBCC
AT+RF_TX_SAVE_PCKT=48656C6C6F
AT+RF_TX_FROM_NVM=1
```

---

### Periodický přenos

Automatické opakované odesílání dat v pravidelných intervalech:

| Příkaz | Formát | Příklad | Účel |
|---------|--------|---------|---------|
| `AT+RF_TX_PERIOD` | `=<ms>` | `AT+RF_TX_PERIOD=5000` | Nastavit interval (5 sekund) |
| `AT+RF_TX_PERIOD_CTRL` | `=ON` / `=OFF` | `AT+RF_TX_PERIOD_CTRL=ON` | Spustit/zastavit |
| `AT+RF_TX_PERIOD_STATUS` | `?` | `AT+RF_TX_PERIOD_STATUS?` | Zjistit stav |

**Použití:**
```
AT+RF_TX_PERIOD=5000                # Interval 5 sekund
AT+RF_TX_SAVE_PCKT=48656C6C6F       # Uložit payload
AT+RF_TX_PERIOD_CTRL=ON             # Spustit periodický přenos
# Zařízení odesílá každých 5 sekund...
AT+RF_TX_PERIOD_STATUS?             # Ověřit stav
AT+RF_TX_PERIOD_CTRL=OFF            # Zastavit
```

---

### Příjem dat

| Příkaz | Formát | Účel |
|---------|--------|---------|
| `AT+RF_RX_TO_UART` | `=ON` / `=OFF` | Přeposílat přijaté pakety na UART |

**Příklad:**
```
AT+RF_RX_TO_UART=ON
# Zařízení naslouchá a vypisuje přijaté pakety:
# +RX: 48656C6C6F20576F726C64
```

---

### Ovládání GPIO

Přímé ovládání GPIO pinů pro externí zařízení (relé, senzory atd.):

| Příkaz | Formát | Příklad | Účel |
|---------|--------|---------|---------|
| `AT+AUX` | `=<pin>,<ON\|OFF>` | `AT+AUX=1,ON` | Nastavit GPIO high/low |
| `AT+AUX_PULSE` | `=<pin>,<period_ms>,<duty_%>` | `AT+AUX_PULSE=2,1000,50` | PWM výstup |
| `AT+AUX_PULSE_STOP` | `=<pin>` | `AT+AUX_PULSE_STOP=2` | Zastavit PWM |

**Příklady:**
```
AT+AUX=1,ON                 # Pin 1 high (např. aktivovat relé)
AT+AUX=1,OFF                # Pin 1 low
AT+AUX_PULSE=2,1000,75      # PWM na pinu 2: 1 kHz, 75% duty cycle
AT+AUX_PULSE_STOP=2         # Zastavit PWM na pinu 2
```

---

## Rychlý start

### 1. Připojení hardwaru

```
USB TTL Převodník     AT_LoRa_Dongle
────────────────      ──────────────
GND ──────────────► GND
RXD ──────────────► TX (USART1)
TXD ──────────────► RX (USART1)
+5V/+3.3V ────────► VCC
```

Nastavení terminálu: **115200 baud, 8N1**

### 2. Test spojení

```bash
AT                  # Test
# Očekáváno: OK

AT+IDENTIFY         # Info o zařízení
# Očekáváno: AT-LoRa_Dongle v1.0

AT+HELP             # Seznam příkazů
```

### 3. Nastavení LoRa parametrů (EU 869 MHz)

```bash
# TX parametry
AT+LR_TX_FREQ=869525000      # 869.525 MHz
AT+LR_TX_POWER=14            # 14 dBm
AT+LR_TX_SF=7                # SF7
AT+LR_TX_BW=125000           # 125 kHz
AT+LR_TX_CR=45               # CR 4/5
AT+LR_TX_CRC=1               # CRC zapnuto

# RX shodné s TX (pro příjem vlastních zpráv)
AT+LR_RX_FREQ=869525000
AT+LR_RX_SF=7
AT+LR_RX_BW=125000
AT+LR_RX_CR=45
AT+LR_RX_CRC=1
```

**Nebo hromadně:**
```bash
AT+LR_TX_SET=SF:7,BW:7,CR:45,Freq:869525000,IQ:0,Header:0,CRC:1,Power:14
AT+LR_RX_SET=SF:7,BW:7,CR:45,Freq:869525000,IQ:0,Header:0,CRC:1
```

### 4. Odesílání dat

```bash
AT+RF_TX_TXT=Hello World        # Odeslat text
AT+RF_TX_HEX=48656C6C6F         # Odeslat hex

# Nebo uložit a odeslat z NVM
AT+RF_TX_SAVE_PCKT=DEADBEEF
AT+RF_TX_FROM_NVM=1
```

### 5. Příjem dat

```bash
AT+RF_RX_TO_UART=ON
# Zařízení vypisuje přijaté pakety:
# +RX: 48656C6C6F20576F726C64
```

### 6. Periodický přenos (např. data každých 10 sekund)

```bash
AT+RF_TX_SAVE_PCKT=DEADBEEF        # Uložit payload
AT+RF_TX_PERIOD=10000              # 10 sekund
AT+RF_TX_PERIOD_CTRL=ON            # Spustit
AT+RF_TX_PERIOD_STATUS?            # Ověřit

# Zastavení:
AT+RF_TX_PERIOD_CTRL=OFF
```

---

## Build a flashování

### Potřebné nástroje

- **STM32CubeCLT** nebo **STM32CubeIDE**
- **ARM GCC toolchain** (arm-none-eabi-gcc)
- **CMake** 3.22+
- **STM32_Programmer_CLI** pro flashování

### Build

1. **Klonování:**
   ```bash
   git clone https://github.com/witc/AT_LoRa_Dongle.git
   cd AT_LoRa_Dongle
   ```

2. **Konfigurace:**
   ```bash
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
   ```

3. **Kompilace:**
   ```bash
   cmake --build build --config Debug
   ```

   Výstup: `build/Debug/AT_LoRa_Dongle.elf`

### Flashování

**VSCode task** (doporučeno):
1. Připoj zařízení přes SWD
2. Spusť task: `CMake: Flash project (SWD)`

**CLI:**
```bash
STM32_Programmer_CLI --connect port=swd --download build/Debug/AT_LoRa_Dongle.elf -hardRst -rst --start
```

---

## Struktura projektu

```
AT_LoRa_Dongle/
├── Core/                       # STM32 HAL & systém
│   ├── Inc/                    # Hlavičky (FreeRTOSConfig.h, main.h)
│   └── Src/                    # main.c, HAL callbacks
│
├── Modules/                    # Aplikační kód
│   ├── ATInterface/            # AT parser (AT_cmd.c)
│   ├── Tasks/
│   │   ├── MainTask/           # Hlavní task, GPIO
│   │   └── RFTask/             # RF operace
│   ├── RF/SX1262/              # SX1262 driver (Semtech)
│   └── NVMA/                   # Non-volatile paměť
│
├── Drivers/                    # STM32 HAL drivers
├── Middlewares/                # FreeRTOS kernel
└── build/                      # Build výstupy
```

---

## Přidání nového AT příkazu

1. **Definuj enum** v `AT_cmd.h`:
   ```c
   typedef enum {
       // ... existující
       SYS_CMD_MY_NEW_COMMAND = 99,
   } eATCommands;
   ```

2. **Přidej do tabulky** v `AT_cmd.c`:
   ```c
   const AT_Command_Struct AT_Commands[] = {
       {"AT+MY_CMD", NULL, SYS_CMD_MY_NEW_COMMAND, "Popis", "parametry"},
   };
   ```

3. **Implementuj handler** (např. v `general_sys_cmd.c`):
   ```c
   void AT_HandleMyCommand(char *params) {
       // Zpracuj příkaz
       AT_SendStringResponse("OK\r\n");
   }
   ```

---

## Řešení problémů

| Problém | Diagnóza | Řešení |
|---------|-----------|----------|
| **Žádná odpověď na AT** | Zařízení nereaguje | Zkontroluj UART připojení, baud rate (115200), napájení |
| **Build selhává** | Chyby kompilace | Ověř CMake 3.22+, ARM GCC toolchain, závislosti |
| **TX nevysílá** | Žádný RF výstup | Zkontroluj frekvenci (opt. 869.525 MHz), anténu, TX power, SF (5-12) |
| **Nízký výkon na jiných frekvencích** | Slabý signál mimo 869 MHz | Očekávané – HW optimalizován pouze pro 869.525 MHz |
| **RX jen šum** | Nelze dekódovat zprávy | Ověř shodné parametry RX a TX (SF, BW, CR, frekvence, header, CRC) |
| **Periodický TX nefunguje** | Neopakuje se | Ověř uložený paket v NVM, nastavenou periodu, `AT+RF_TX_PERIOD_CTRL=ON` |
| **Zařízení "zbricklé"** | Neodpovídá vůbec | Připoj přes SWD, flashni firmware pomocí STM32CubeProgrammer GUI |

**Factory reset:**
```bash
AT+FACTORY_MODE=ON
AT+SYS_RESTART
```

---

## API Reference

### Main Task (`Main_task.h`)

```c
void main_task(void);                                    // FreeRTOS task
bool MT_SendDataToMainTask(dataQueue_t *data);          // Poslat data do main tasku
bool AT_CustomCommandHandler(char *data, ...);           // Handler vlastních AT příkazů
```

### RF Task (`RF_Task.h`)

```c
void rf_task(void);                                      // RF task (TX/RX operace)
```

### AT Commands (`AT_cmd.h`)

```c
void AT_Init(AT_cmd_t *atCmd);                          // Inicializace AT subsystému
void AT_HandleATCommand(uint16_t size);                 // Zpracování AT příkazu
void AT_SendStringResponse(char *response);             // Odeslat odpověď na UART
```

### NVMA (`NVMA.h`)

```c
void NVMA_SavePacket(uint8_t *data, uint16_t length);   // Uložit paket do flash
void NVMA_LoadPacket(uint8_t *buffer, uint16_t *len);   // Načíst paket z flash
```

---

## Výkonové parametry

| Parametr | Hodnota |
|-----------|---------|
| **Odezva příkazu** | <10 ms |
| **TX latence** | ~100 ms (setup) + čas TX |
| **RX citlivost** (SF7, BW125) | ~-120 dBm |
| **Spotřeba (RX)** | ~80 mA |
| **Spotřeba (TX @ 22 dBm)** | ~500 mA peak |
| **Velikost paketu** | 1-255 bytů |

---

## Licence

- **Firmware:** [Your License]
- **SX1262 Driver:** Semtech
- **FreeRTOS:** Real Time Engineers Ltd
- **STM32 HAL:** STMicroelectronics

---

**Verze:** v1.0  
**Hardware:** Rev 1.0 (869.525 MHz)  
**Aktualizováno:** Prosinec 2024
