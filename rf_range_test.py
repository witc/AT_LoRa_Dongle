#!/usr/bin/env python3
"""
RF Range Test Script for AT LoRa Dongle
Tests various SF, BW combinations between two dongles.

Author: Auto-generated
Date: 2026-01-01
"""

import serial
import serial.tools.list_ports
import time
import sys
from dataclasses import dataclass
from typing import Optional, List, Tuple
import logging

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Test configuration
TEST_PACKET_HEX = "010203"  # 3 bytes
TEST_FREQUENCY = 869525000
BAUD_RATE = 115200
TIMEOUT = 5.0  # seconds

# RF Parameters to test
SF_RANGE = range(5, 13)  # SF5 to SF12
BW_OPTIONS = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]  # All bandwidth options
# BW mapping: 0=7.81kHz, 1=10.42kHz, 2=15.63kHz, 3=20.83kHz, 4=31.25kHz, 
#             5=41.67kHz, 6=62.5kHz, 7=125kHz, 8=250kHz, 9=500kHz

CR_OPTIONS = [45, 46, 47, 48]  # Coding rates: 4/5, 4/6, 4/7, 4/8
IQ_INV_OPTIONS = [0, 1]  # IQ inversion: 0=normal, 1=inverted
HEADER_MODE_OPTIONS = [0, 1]  # Header mode: 0=explicit, 1=implicit
CRC_OPTIONS = [0, 1]  # CRC: 0=disabled, 1=enabled
PREAMBLE_OPTIONS = [8]  # Preamble length in symbols (can add: 8, 16, 32, etc.)

# Fixed parameters (not iterated)
FIXED_POWER = 22  # TX power in dBm
FIXED_LDRO = 2  # LDRO: 0=off, 1=on, 2=auto (always auto)

# Test filtering options
SKIP_SLOW_TESTS = True  # Skip slow combinations: BW <= 5 and SF >= 10

def should_skip_test(sf: int, bw: int) -> bool:
    """Check if test should be skipped based on filter settings"""
    if SKIP_SLOW_TESTS:
        # Skip slow combinations: low bandwidth + high spreading factor
        if bw <= 4 or sf >= 10:
            return True
    return False

CR_NAMES = {
    45: "4/5",
    46: "4/6", 
    47: "4/7",
    48: "4/8"
}

LDRO_NAMES = {
    0: "off",
    1: "on",
    2: "auto"
}

HEADER_MODE_NAMES = {
    0: "Expl",
    1: "Impl"
}

CRC_NAMES = {
    0: "OFF",
    1: "ON"
}

BW_NAMES = {
    0: "7.81 kHz",
    1: "10.42 kHz", 
    2: "15.63 kHz",
    3: "20.83 kHz",
    4: "31.25 kHz",
    5: "41.67 kHz",
    6: "62.5 kHz",
    7: "125 kHz",
    8: "250 kHz",
    9: "500 kHz"
}

CR_NAMES = {
    45: "4/5",
    46: "4/6",
    47: "4/7",
    48: "4/8"
}

LDRO_NAMES = {
    0: "OFF",
    1: "ON",
    2: "AUTO"
}


@dataclass
class TestResult:
    sf: int
    bw: int
    cr: int
    freq: int
    power: int
    iq_inv: int
    ldro: int
    header_mode: int
    crc: int
    preamble: int
    success: bool
    tx_time_ms: float
    rx_time_ms: float
    rssi: int = 0
    error_msg: str = ""


class ATLoraDongle:
    """Class to communicate with AT LoRa Dongle"""
    
    def __init__(self, port: str, name: str = "Dongle"):
        self.port = port
        self.name = name
        self.serial: Optional[serial.Serial] = None
        
    def connect(self) -> bool:
        """Connect to the dongle"""
        try:
            self.serial = serial.Serial(
                port=self.port,
                baudrate=BAUD_RATE,
                timeout=TIMEOUT,
                write_timeout=TIMEOUT
            )
            time.sleep(0.5)  # Wait for connection to stabilize
            self.serial.reset_input_buffer()
            self.serial.reset_output_buffer()
            logger.info(f"{self.name}: Connected to {self.port}")
            return True
        except Exception as e:
            logger.error(f"{self.name}: Failed to connect to {self.port}: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from the dongle"""
        if self.serial and self.serial.is_open:
            self.serial.close()
            logger.info(f"{self.name}: Disconnected from {self.port}")
    
    def send_command(self, cmd: str, wait_response: bool = True, timeout: float = None, 
                     expected_ok_count: int = 1, stop_on_pattern: str = None) -> Tuple[bool, str]:
        """Send AT command and wait for response
        
        Args:
            cmd: AT command to send
            wait_response: Whether to wait for response
            timeout: Custom timeout in seconds
            expected_ok_count: Number of OK responses to wait for (for multi-param commands)
            stop_on_pattern: Stop reading when this pattern is found in response (e.g. 'TOA:')
        """
        if not self.serial or not self.serial.is_open:
            return False, "Not connected"
        
        original_timeout = self.serial.timeout
        if timeout:
            self.serial.timeout = timeout
            
        try:
            # Clear buffers
            self.serial.reset_input_buffer()
            
            # Send command
            full_cmd = f"{cmd}\r\n"
            self.serial.write(full_cmd.encode())
            logger.debug(f"{self.name} TX: {cmd}")
            
            if not wait_response:
                return True, ""
            
            # Read response
            response_lines = []
            ok_count = 0
            error_count = 0
            start_time = time.time()
            
            while True:
                if time.time() - start_time > (timeout or TIMEOUT):
                    break
                    
                if self.serial.in_waiting > 0:
                    line = self.serial.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        response_lines.append(line)
                        logger.debug(f"{self.name} RX: {line}")
                        
                        # Check for custom stop pattern first
                        if stop_on_pattern and stop_on_pattern in line:
                            break
                        
                        # Count OK and ERROR responses
                        if line == "OK":
                            ok_count += 1
                            if ok_count >= expected_ok_count:
                                break
                        elif "ERROR" in line:
                            error_count += 1
                            break  # Stop on first error
                else:
                    time.sleep(0.01)
            
            response = "\n".join(response_lines)
            success = ok_count >= expected_ok_count and error_count == 0
            
            return success, response
            
        except Exception as e:
            logger.error(f"{self.name}: Command error: {e}")
            return False, str(e)
        finally:
            self.serial.timeout = original_timeout
    
    def configure_tx(self, sf: int, bw: int, freq: int, cr: int = 45, power: int = 22, 
                     iq_inv: int = 0, ldro: int = 2, header_mode: int = 0, crc: int = 1, preamble: int = 8) -> bool:
        """Configure TX parameters"""
        cmd = f"AT+LR_TX_SET=SF:{sf},BW:{bw},CR:{cr},Freq:{freq},IQInv:{iq_inv},HeaderMode:{header_mode},CRC:{crc},Preamble:{preamble},Power:{power},LDRO:{ldro}"
        # 10 parameters = 10 OK responses expected
        success, response = self.send_command(cmd, expected_ok_count=10)
        if success:
            ldro_name = {0: 'off', 1: 'on', 2: 'auto'}.get(ldro, str(ldro))
            logger.info(f"{self.name}: TX configured - SF{sf}, BW{bw}, CR{cr}, Pwr:{power}dBm, LDRO:{ldro_name}")
        else:
            logger.error(f"{self.name}: TX config failed: {response}")
        return success
    
    def configure_rx(self, sf: int, bw: int, freq: int, cr: int = 45, 
                     iq_inv: int = 0, ldro: int = 2, header_mode: int = 0, crc: int = 1, preamble: int = 8,
                     rx_payload_len: int = 0) -> bool:
        """Configure RX parameters
        
        Args:
            rx_payload_len: Expected payload length for implicit header mode (0=auto/max buffer)
        """
        # For implicit header mode, set expected payload length first
        if header_mode == 1 and rx_payload_len > 0:
            pldlen_cmd = f"AT+LR_RX_PLDLEN={rx_payload_len}"
            success, response = self.send_command(pldlen_cmd)
            if not success:
                logger.error(f"{self.name}: RX payload length config failed: {response}")
                return False
            logger.debug(f"{self.name}: RX payload length set to {rx_payload_len} for implicit mode")
        elif header_mode == 0:
            # For explicit mode, set to 0 (use max buffer)
            pldlen_cmd = "AT+LR_RX_PLDLEN=0"
            self.send_command(pldlen_cmd)
        
        cmd = f"AT+LR_RX_SET=SF:{sf},BW:{bw},CR:{cr},Freq:{freq},IQInv:{iq_inv},HeaderMode:{header_mode},CRC:{crc},Preamble:{preamble},LDRO:{ldro}"
        # 9 parameters = 9 OK responses expected
        success, response = self.send_command(cmd, expected_ok_count=9)
        if success:
            ldro_name = {0: 'off', 1: 'on', 2: 'auto'}.get(ldro, str(ldro))
            logger.info(f"{self.name}: RX configured - SF{sf}, BW{bw}, CR{cr}, LDRO:{ldro_name}, Hdr:{header_mode}, CRC:{crc}")
        else:
            logger.error(f"{self.name}: RX config failed: {response}")
        return success
    
    def enable_rx_to_uart(self, enable: bool = True) -> bool:
        """Enable/disable RX to UART output"""
        cmd = f"AT+RF_RX_TO_UART={'ON' if enable else 'OFF'}"
        success, _ = self.send_command(cmd)
        return success
    
    def get_toa(self, packet_size: int) -> int:
        """Get Time on Air for a packet of given size in ms
        
        Args:
            packet_size: Packet size in bytes
            
        Returns:
            TOA in milliseconds, or 0 if failed
        """
        cmd = f"AT+RF_GET_TOA={packet_size}"
        _, response = self.send_command(cmd, timeout=2.0, stop_on_pattern="ms")
        # Response format: "TOA: X ms" - doesn't include OK, stop when we see "ms"
        if "TOA:" in response:
            # Parse "TOA: X ms" from response
            import re
            match = re.search(r'TOA:\s*(\d+)\s*ms', response)
            if match:
                toa = int(match.group(1))
                logger.debug(f"{self.name}: TOA for {packet_size}B = {toa} ms")
                return toa
        logger.warning(f"{self.name}: Failed to get TOA: {response}")
        return 0
    
    def send_packet(self, hex_data: str) -> bool:
        """Send a packet via RF"""
        cmd = f"AT+RF_TX_HEX={hex_data}"
        success, response = self.send_command(cmd)
        return success
    
    def wait_for_rx_packet(self, timeout: float = 10.0) -> Tuple[bool, str, int]:
        """Wait for incoming RX packet
        
        Returns:
            Tuple of (success, hex_data, rssi)
        """
        if not self.serial or not self.serial.is_open:
            return False, "Not connected", 0
        
        start_time = time.time()
        buffer = ""
        
        # Pattern for RX packet: +RF_RX:<len>,<hex_data>,RSSI:<rssi>
        import re
        pattern = re.compile(r'\+RF_RX:(\d+),([0-9A-Fa-f]+),RSSI:(-?\d+)')
        
        try:
            while time.time() - start_time < timeout:
                if self.serial.in_waiting > 0:
                    chunk = self.serial.read(self.serial.in_waiting).decode('utf-8', errors='ignore')
                    buffer += chunk
                    logger.debug(f"{self.name} RX buffer: {chunk.strip()}")
                    
                    match = pattern.search(buffer)
                    if match:
                        data_len = int(match.group(1))
                        hex_data = match.group(2).upper()
                        rssi = int(match.group(3))
                        logger.info(f"{self.name}: Received {data_len} bytes, RSSI: {rssi} dBm")
                        return True, hex_data, rssi
                else:
                    time.sleep(0.01)
            
            return False, "Timeout waiting for packet", 0
            
        except Exception as e:
            return False, str(e), 0


def test_eeprom_storage(tx_dongle: ATLoraDongle, rx_dongle: ATLoraDongle) -> bool:
    """Test EEPROM storage for all configuration parameters
    
    Writes various configurations and reads them back to verify correct EEPROM storage.
    Returns True if all tests pass, False otherwise.
    """
    
    logger.info("\n" + "="*70)
    logger.info("EEPROM STORAGE VERIFICATION TEST")
    logger.info("="*70)
    
    all_passed = True
    failed_tests = 0
    
    # Individual parameter tests - write value then read back with ?
    # BW uses index 0-9, not Hz values!
    param_tests = [
        # TX parameters
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_SF=7", "get_cmd": "AT+LR_TX_SF?", "expected": "7", "name": "TX SF"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_SF=12", "get_cmd": "AT+LR_TX_SF?", "expected": "12", "name": "TX SF (high)"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_BW=7", "get_cmd": "AT+LR_TX_BW?", "expected": "7", "name": "TX BW (125kHz)"},  # BW7 = 125kHz
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_BW=9", "get_cmd": "AT+LR_TX_BW?", "expected": "9", "name": "TX BW (500kHz)"},  # BW9 = 500kHz
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_CR=45", "get_cmd": "AT+LR_TX_CR?", "expected": "45", "name": "TX CR"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_CR=48", "get_cmd": "AT+LR_TX_CR?", "expected": "48", "name": "TX CR (high)"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_POWER=10", "get_cmd": "AT+LR_TX_POWER?", "expected": "10", "name": "TX Power"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_POWER=22", "get_cmd": "AT+LR_TX_POWER?", "expected": "22", "name": "TX Power (high)"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_IQ_INV=0", "get_cmd": "AT+LR_TX_IQ_INV?", "expected": "0", "name": "TX IQ Inv (off)"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_IQ_INV=1", "get_cmd": "AT+LR_TX_IQ_INV?", "expected": "1", "name": "TX IQ Inv (on)"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_HEADERMODE=0", "get_cmd": "AT+LR_TX_HEADERMODE?", "expected": "0", "name": "TX Header (explicit)"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_HEADERMODE=1", "get_cmd": "AT+LR_TX_HEADERMODE?", "expected": "1", "name": "TX Header (implicit)"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_CRC=0", "get_cmd": "AT+LR_TX_CRC?", "expected": "0", "name": "TX CRC (off)"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_CRC=1", "get_cmd": "AT+LR_TX_CRC?", "expected": "1", "name": "TX CRC (on)"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_PREAMBLE_SIZE=8", "get_cmd": "AT+LR_TX_PREAMBLE_SIZE?", "expected": "8", "name": "TX Preamble"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_PREAMBLE_SIZE=16", "get_cmd": "AT+LR_TX_PREAMBLE_SIZE?", "expected": "16", "name": "TX Preamble (high)"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_LDRO=0", "get_cmd": "AT+LR_TX_LDRO?", "expected": "0", "name": "TX LDRO (off)"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_LDRO=1", "get_cmd": "AT+LR_TX_LDRO?", "expected": "1", "name": "TX LDRO (on)"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_LDRO=2", "get_cmd": "AT+LR_TX_LDRO?", "expected": "2", "name": "TX LDRO (auto)"},
        
        # RX parameters
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_SF=7", "get_cmd": "AT+LR_RX_SF?", "expected": "7", "name": "RX SF"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_SF=12", "get_cmd": "AT+LR_RX_SF?", "expected": "12", "name": "RX SF (high)"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_BW=7", "get_cmd": "AT+LR_RX_BW?", "expected": "7", "name": "RX BW (125kHz)"},  # BW7 = 125kHz
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_BW=9", "get_cmd": "AT+LR_RX_BW?", "expected": "9", "name": "RX BW (500kHz)"},  # BW9 = 500kHz
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_CR=45", "get_cmd": "AT+LR_RX_CR?", "expected": "45", "name": "RX CR"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_CR=48", "get_cmd": "AT+LR_RX_CR?", "expected": "48", "name": "RX CR (high)"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_IQ_INV=0", "get_cmd": "AT+LR_RX_IQ_INV?", "expected": "0", "name": "RX IQ Inv (off)"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_IQ_INV=1", "get_cmd": "AT+LR_RX_IQ_INV?", "expected": "1", "name": "RX IQ Inv (on)"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_HEADERMODE=0", "get_cmd": "AT+LR_RX_HEADERMODE?", "expected": "0", "name": "RX Header (explicit)"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_HEADERMODE=1", "get_cmd": "AT+LR_RX_HEADERMODE?", "expected": "1", "name": "RX Header (implicit)"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_CRC=0", "get_cmd": "AT+LR_RX_CRC?", "expected": "0", "name": "RX CRC (off)"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_CRC=1", "get_cmd": "AT+LR_RX_CRC?", "expected": "1", "name": "RX CRC (on)"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_PREAMBLE_SIZE=8", "get_cmd": "AT+LR_RX_PREAMBLE_SIZE?", "expected": "8", "name": "RX Preamble"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_PREAMBLE_SIZE=16", "get_cmd": "AT+LR_RX_PREAMBLE_SIZE?", "expected": "16", "name": "RX Preamble (high)"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_LDRO=0", "get_cmd": "AT+LR_RX_LDRO?", "expected": "0", "name": "RX LDRO (off)"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_LDRO=1", "get_cmd": "AT+LR_RX_LDRO?", "expected": "1", "name": "RX LDRO (on)"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_LDRO=2", "get_cmd": "AT+LR_RX_LDRO?", "expected": "2", "name": "RX LDRO (auto)"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_PLDLEN=64", "get_cmd": "AT+LR_RX_PLDLEN?", "expected": "64", "name": "RX PldLen"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_PLDLEN=255", "get_cmd": "AT+LR_RX_PLDLEN?", "expected": "255", "name": "RX PldLen (max)"},
        
        # Frequency tests (32-bit values)
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_FREQ=868000000", "get_cmd": "AT+LR_TX_FREQ?", "expected": "868000000", "name": "TX Freq"},
        {"dongle": tx_dongle, "set_cmd": "AT+LR_TX_FREQ=915000000", "get_cmd": "AT+LR_TX_FREQ?", "expected": "915000000", "name": "TX Freq (high)"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_FREQ=868000000", "get_cmd": "AT+LR_RX_FREQ?", "expected": "868000000", "name": "RX Freq"},
        {"dongle": rx_dongle, "set_cmd": "AT+LR_RX_FREQ=915000000", "get_cmd": "AT+LR_RX_FREQ?", "expected": "915000000", "name": "RX Freq (high)"},
        
        # Saved packet test (hex data stored in EEPROM)
        {"dongle": tx_dongle, "set_cmd": "AT+RF_SAVE_PACKET=AABBCC", "get_cmd": "AT+RF_SAVE_PACKET?", "expected": "AABBCC", "name": "Saved Packet", "is_hex": True},
        {"dongle": tx_dongle, "set_cmd": "AT+RF_SAVE_PACKET=112233445566", "get_cmd": "AT+RF_SAVE_PACKET?", "expected": "112233445566", "name": "Saved Packet (long)", "is_hex": True},
        
        # TX period (32-bit value in ms)
        {"dongle": tx_dongle, "set_cmd": "AT+RF_TX_NVM_PERIOD=1000", "get_cmd": "AT+RF_TX_NVM_PERIOD?", "expected": "1000", "name": "TX Period"},
        {"dongle": tx_dongle, "set_cmd": "AT+RF_TX_NVM_PERIOD=5000", "get_cmd": "AT+RF_TX_NVM_PERIOD?", "expected": "5000", "name": "TX Period (high)"},
    ]
    
    import re
    
    for test in param_tests:
        dongle = test["dongle"]
        
        # Clear any pending data in buffer before SET
        dongle.serial.reset_input_buffer()
        
        # Set the parameter - expects OK response
        success, response = dongle.send_command(test["set_cmd"], timeout=2.0)
        if not success:
            logger.error(f"  ✗ {test['name']}: Set command failed - {response}")
            failed_tests += 1
            all_passed = False
            continue
        
        # Read back the parameter - query returns just value, no OK
        dongle.serial.reset_input_buffer()
        dongle.serial.write(f"{test['get_cmd']}\r\n".encode())
        
        # Wait for response with timeout
        start_time = time.time()
        response = ""
        while time.time() - start_time < 1.0:
            if dongle.serial.in_waiting > 0:
                response = dongle.serial.read(dongle.serial.in_waiting).decode('utf-8', errors='ignore').strip()
                if response:
                    break
            time.sleep(0.01)
        
        # Parse response - look for the value (number in response)
        # For hex data, look for hex string pattern
        if test.get("is_hex"):
            # Look for hex string like AABBCC or 112233445566
            hex_match = re.search(r'([0-9A-Fa-f]{4,})', response)
            if hex_match:
                read_value = hex_match.group(1).upper()
                expected = test["expected"].upper()
                if read_value == expected:
                    logger.info(f"  ✓ {test['name']}: {read_value} == {expected}")
                else:
                    logger.error(f"  ✗ {test['name']}: Got {read_value}, expected {expected} - MISMATCH!")
                    failed_tests += 1
                    all_passed = False
            else:
                logger.error(f"  ✗ {test['name']}: Could not parse hex response: '{response[:50]}'")
                failed_tests += 1
                all_passed = False
        else:
            value_match = re.search(r'(\d+)', response)
            if value_match:
                read_value = value_match.group(1)
                if read_value == test["expected"]:
                    logger.info(f"  ✓ {test['name']}: {read_value} == {test['expected']}")
                else:
                    logger.error(f"  ✗ {test['name']}: Got {read_value}, expected {test['expected']} - MISMATCH!")
                    failed_tests += 1
                    all_passed = False
            else:
                logger.error(f"  ✗ {test['name']}: Could not parse response: '{response[:50]}'")
                failed_tests += 1
                all_passed = False
    
    logger.info("\n" + "="*70)
    if all_passed:
        logger.info(f"✓ ALL EEPROM TESTS PASSED ({len(param_tests)} tests)")
    else:
        logger.error(f"✗ {failed_tests}/{len(param_tests)} EEPROM TESTS FAILED - Check NVMA implementation")
    logger.info("="*70 + "\n")
    
    return all_passed


def find_silicon_labs_ports() -> List[str]:
    """Find COM ports with Silicon Labs devices"""
    ports = []
    for port in serial.tools.list_ports.comports():
        if "Silicon Labs" in port.description or "CP210" in port.description:
            ports.append(port.device)
            logger.info(f"Found Silicon Labs device: {port.device} - {port.description}")
    return ports


def run_single_test(tx_dongle: ATLoraDongle, rx_dongle: ATLoraDongle, 
                    sf: int, bw: int, freq: int, cr: int = 45, power: int = 22,
                    iq_inv: int = 0, ldro: int = 2, header_mode: int = 0, crc: int = 1, preamble: int = 8) -> TestResult:
    """Run a single RF test with specified parameters"""
    
    cr_name = CR_NAMES.get(cr, str(cr))
    iq_name = "Inv" if iq_inv else "Norm"
    hdr_name = HEADER_MODE_NAMES.get(header_mode, str(header_mode))
    crc_name = CRC_NAMES.get(crc, str(crc))
    
    logger.info(f"\n{'='*60}")
    logger.info(f"Testing: SF{sf}, BW{bw} ({BW_NAMES.get(bw, 'Unknown')}), CR:{cr_name}")
    logger.info(f"         IQ:{iq_name}, Hdr:{hdr_name}, CRC:{crc_name}")
    logger.info(f"{'='*60}")
    
    result = TestResult(sf=sf, bw=bw, cr=cr, freq=freq, power=power, iq_inv=iq_inv, 
                        ldro=ldro, header_mode=header_mode, crc=crc, preamble=preamble, 
                        success=False, tx_time_ms=0, rx_time_ms=0)
    
    # Calculate expected packet size for implicit header mode
    packet_size_bytes = len(TEST_PACKET_HEX) // 2
    
    # Configure RX first (pass payload length for implicit header mode)
    if not rx_dongle.configure_rx(sf, bw, freq, cr=cr, iq_inv=iq_inv, ldro=ldro, 
                                   header_mode=header_mode, crc=crc, preamble=preamble,
                                   rx_payload_len=packet_size_bytes):
        result.error_msg = "RX configuration failed"
        return result
    
    # Small delay for RX to start listening
    time.sleep(0.2)
    
    # Configure TX
    if not tx_dongle.configure_tx(sf, bw, freq, cr=cr, power=power, iq_inv=iq_inv, ldro=ldro,
                                   header_mode=header_mode, crc=crc, preamble=preamble):
        result.error_msg = "TX configuration failed"
        return result
    
    # Get accurate TOA from firmware for timeout calculation
    toa_ms = tx_dongle.get_toa(packet_size_bytes)
    if toa_ms == 0:
        # Fallback: rough estimate
        toa_ms = 100 * (2 ** (sf - 5)) / (BW_OPTIONS.index(bw) + 1) if bw in BW_OPTIONS else 1000
        logger.warning(f"Using estimated TOA: {toa_ms:.0f} ms")
    else:
        logger.info(f"TOA: {toa_ms} ms (from firmware)")
    
    # Timeout = TOA + 25% margin (minimum 100ms for command processing)
    rx_timeout_ms = max(100, toa_ms * 1.25)
    rx_timeout_s = rx_timeout_ms / 1000
    logger.info(f"RX timeout set to: {rx_timeout_ms:.0f} ms (TOA {toa_ms} ms + 25%)")
    
    # Clear RX buffer before sending
    rx_dongle.serial.reset_input_buffer()
    
    # Start timing from TX command - this is when packet transmission begins
    tx_start = time.time()
    if not tx_dongle.send_packet(TEST_PACKET_HEX):
        result.error_msg = "TX send failed"
        return result
    result.tx_time_ms = (time.time() - tx_start) * 1000
    
    # Wait for RX packet (timeout starts from TX command)
    success, received, rssi = rx_dongle.wait_for_rx_packet(timeout=rx_timeout_s)
    actual_wait_time = time.time() - tx_start
    result.rx_time_ms = actual_wait_time  # Total time from TX start
    result.rssi = rssi
    
    if success:
        # Verify packet content
        expected = TEST_PACKET_HEX.upper()
        received = received.upper()
        
        if received == expected:
            result.success = True
            logger.info(f"✓ SUCCESS: Packet received correctly ({len(TEST_PACKET_HEX)//2} bytes), RSSI: {rssi} dBm, Real wait: {actual_wait_time*1000:.0f} ms")
        else:
            result.error_msg = f"Data mismatch: expected {expected}, got {received}"
            logger.warning(f"✗ FAIL: {result.error_msg}, Real wait: {actual_wait_time*1000:.0f} ms")
    else:
        result.error_msg = received  # Contains error message
        logger.warning(f"✗ FAIL: {result.error_msg}, Real wait: {actual_wait_time*1000:.0f} ms")
    
    return result


def run_full_test_suite(tx_dongle: ATLoraDongle, rx_dongle: ATLoraDongle) -> List[TestResult]:
    """Run complete test suite with all parameter combinations"""
    
    results = []
    
    # Calculate total tests (accounting for skipped combinations)
    total_tests = 0
    for sf in SF_RANGE:
        for bw in BW_OPTIONS:
            if not should_skip_test(sf, bw):
                total_tests += len(CR_OPTIONS) * len(IQ_INV_OPTIONS) * len(HEADER_MODE_OPTIONS) * len(CRC_OPTIONS) * len(PREAMBLE_OPTIONS)
    
    skipped_combos = (len(SF_RANGE) * len(BW_OPTIONS) - sum(1 for sf in SF_RANGE for bw in BW_OPTIONS if not should_skip_test(sf, bw)))
    current_test = 0
    
    logger.info(f"\n{'#'*60}")
    logger.info(f"Starting RF Range Test Suite")
    logger.info(f"Total tests: {total_tests}")
    if SKIP_SLOW_TESTS:
        logger.info(f"Skipping slow tests (BW<=5 & SF>=10): {skipped_combos} SF/BW combinations excluded")
    logger.info(f"Test packet size: {len(TEST_PACKET_HEX)//2} bytes")
    logger.info(f"Parameters: SF={list(SF_RANGE)}, BW={BW_OPTIONS}, CR={CR_OPTIONS}")
    logger.info(f"            IQ={IQ_INV_OPTIONS}, Header={HEADER_MODE_OPTIONS}, CRC={CRC_OPTIONS}")
    logger.info(f"Fixed: Power={FIXED_POWER}dBm, LDRO=auto")
    logger.info(f"{'#'*60}\n")
    
    # Enable RX to UART on receiver
    rx_dongle.enable_rx_to_uart(True)
    
    for cr in CR_OPTIONS:
        for iq_inv in IQ_INV_OPTIONS:
            for header_mode in HEADER_MODE_OPTIONS:
                for crc in CRC_OPTIONS:
                    for preamble in PREAMBLE_OPTIONS:
                        for sf in SF_RANGE:
                            for bw in BW_OPTIONS:
                                # Skip slow test combinations if configured
                                if should_skip_test(sf, bw):
                                    continue
                                    
                                current_test += 1
                                logger.info(f"\nTest {current_test}/{total_tests}")
                                
                                result = run_single_test(tx_dongle, rx_dongle, sf, bw, TEST_FREQUENCY,
                                                        cr=cr, power=FIXED_POWER, iq_inv=iq_inv, 
                                                        ldro=FIXED_LDRO, header_mode=header_mode,
                                                        crc=crc, preamble=preamble)
                                results.append(result)
    
    return results


def print_results_summary(results: List[TestResult]):
    """Print a summary table of all test results"""
    
    print("="*140)
    print("TEST RESULTS SUMMARY")
    print("="*140)
    print(f"{'SF':<4} {'BW':<10} {'CR':<5} {'IQ':<5} {'Hdr':<5} {'CRC':<4} {'Result':<10} {'RSSI':<10} {'Wait(ms)':<10} {'Error'}")
    print("-"*140)
    
    success_count = 0
    fail_count = 0
    
    for r in results:
        status = "✓ PASS" if r.success else "✗ FAIL"
        if r.success:
            success_count += 1
        else:
            fail_count += 1
            
        bw_name = BW_NAMES.get(r.bw, f"BW{r.bw}")
        cr_name = CR_NAMES.get(r.cr, str(r.cr))
        iq_name = "Inv" if r.iq_inv else "Norm"
        hdr_name = HEADER_MODE_NAMES.get(r.header_mode, str(r.header_mode))
        crc_name = CRC_NAMES.get(r.crc, str(r.crc))
        rssi_str = f"{r.rssi} dBm" if r.success else "-"
        print(f"{r.sf:<4} {bw_name:<10} {cr_name:<5} {iq_name:<5} {hdr_name:<5} {crc_name:<4} {status:<10} {rssi_str:<10} {r.rx_time_ms*1000:<10.0f} {r.error_msg[:25]}")
    
    print("-"*140)
    print(f"Total: {len(results)} tests | Passed: {success_count} | Failed: {fail_count} | Success rate: {success_count/len(results)*100:.1f}%")
    print("="*140)
    
    # Print matrix view
    print("\nRESULTS MATRIX (SF vs BW):")
    print("-"*60)
    
    # Header
    print(f"{'SF':<4}", end="")
    for bw in BW_OPTIONS:
        print(f"{bw:<6}", end="")
    print()
    
    # Data rows
    for sf in SF_RANGE:
        print(f"{sf:<4}", end="")
        for bw in BW_OPTIONS:
            # Find result for this SF/BW combination
            result = next((r for r in results if r.sf == sf and r.bw == bw), None)
            if result:
                symbol = "✓" if result.success else "✗"
            else:
                symbol = "-"
            print(f"{symbol:<6}", end="")
        print()
    
    print("-"*60)
    print("BW Legend:", ", ".join([f"{k}={v}" for k, v in BW_NAMES.items()]))


def main():
    """Main entry point"""
    
    print("\n" + "#"*60)
    print("  AT LoRa Dongle RF Range Test")
    print("#"*60 + "\n")
    
    # Find Silicon Labs COM ports
    ports = find_silicon_labs_ports()
    
    if len(ports) < 2:
        logger.error(f"Need 2 Silicon Labs devices, found {len(ports)}")
        logger.error("Please connect both AT LoRa Dongles")
        sys.exit(1)
    
    if len(ports) > 2:
        logger.warning(f"Found {len(ports)} devices, using first two: {ports[0]}, {ports[1]}")
    
    # Create dongle instances
    tx_dongle = ATLoraDongle(ports[0], "TX_Dongle")
    rx_dongle = ATLoraDongle(ports[1], "RX_Dongle")
    
    try:
        # Connect to both dongles
        if not tx_dongle.connect():
            logger.error("Failed to connect to TX dongle")
            sys.exit(1)
            
        if not rx_dongle.connect():
            logger.error("Failed to connect to RX dongle")
            tx_dongle.disconnect()
            sys.exit(1)
        
        # Verify communication with basic AT command
        logger.info("\nVerifying communication...")
        
        # AT command returns help text, so just check we get a response
        success, response = tx_dongle.send_command("AT", timeout=3.0)
        if not response or len(response) < 10:
            logger.error(f"TX dongle not responding")
            sys.exit(1)
        logger.info(f"TX dongle OK (received {len(response)} bytes)")
        
        success, response = rx_dongle.send_command("AT", timeout=3.0)
        if not response or len(response) < 10:
            logger.error(f"RX dongle not responding")
            sys.exit(1)
        logger.info(f"RX dongle OK (received {len(response)} bytes)")
        
        # Run EEPROM storage verification test first
        logger.info("\n" + "*"*70)
        logger.info("* PHASE 1: EEPROM STORAGE VERIFICATION")
        logger.info("*"*70)
        eeprom_test_passed = test_eeprom_storage(tx_dongle, tx_dongle)
        
        if not eeprom_test_passed:
            logger.error("\n⚠ EEPROM verification failed - check your AT firmware and NVMA implementation")
            logger.error("Fix EEPROM issues before running RF range tests")
            tx_dongle.disconnect()
            rx_dongle.disconnect()
            sys.exit(1)
        
        # Run test suite
        logger.info("\n" + "*"*70)
        logger.info("* PHASE 2: RF RANGE TESTS")
        logger.info("*"*70)
        results = run_full_test_suite(tx_dongle, rx_dongle)
        
        # Print summary
        print_results_summary(results)
        
    except KeyboardInterrupt:
        logger.info("\nTest interrupted by user")
    except Exception as e:
        logger.error(f"Test error: {e}")
        raise
    finally:
        # Cleanup
        tx_dongle.disconnect()
        rx_dongle.disconnect()
        logger.info("\nTest complete")


if __name__ == "__main__":
    main()
