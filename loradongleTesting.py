


import re
import serial
import time
from serial.tools import list_ports

def find_silicon_labs_ports():
    """Find all Silicon Labs COM ports"""
    silicon_ports = []
    for port in list_ports.comports():
        # Check if manufacturer contains "Silicon Labs" or description contains it
        if port.manufacturer and "Silicon Labs" in port.manufacturer:
            silicon_ports.append(port.device)
        elif "Silicon Labs" in port.description:
            silicon_ports.append(port.device)
    return sorted(silicon_ports)

class ATLoRaDongle:
    
    def __init__(self, port: str, baudrate: int = 115200, timeout: float = 1.0):
            """Initialize the AT LoRa Dongle connection"""
            self.ser = None
            try:
                self.ser = serial.Serial(port, baudrate, timeout=timeout)
                time.sleep(2)  # Wait for the serial connection to initialize
            except serial.SerialException as e:
                print(f"Error opening serial port {port}: {e}")
                self.ser = None

    def close(self):
        """Close the serial connection"""
        if self.ser and self.ser.is_open:
            self.ser.close()
            self.ser = None

    def send_cmd(self, cmd: str, timeout: float = 1.0, wait_all: bool = False):
        """Send a command to the dongle and return the response
        
        Args:
            cmd: Command to send
            timeout: Response timeout in seconds
            wait_all: If True, wait for timeout to collect all responses (for bulk commands)
        """
        if not self.ser:
            return None
        
        self.ser.reset_input_buffer()
        full_cmd = cmd + "\r\n"
        self.ser.write(full_cmd.encode('utf-8'))
        
        end_time = time.time() + timeout
        response_lines = []
        
        while time.time() < end_time:
            line = self.ser.readline().decode('utf-8').strip()
            if line:
                response_lines.append(line)
                # For bulk commands, continue reading until timeout
                if not wait_all and (line == "OK" or line.startswith("ERROR")):
                    break
        
        return "\n".join(response_lines) if response_lines else None
    
    def configure_tx(self, sf: int, bw: int, cr: int, header: int, preamble: int, iq: int, crc: int, freq: int = 869525000, power: int = 14, debug: bool = False) -> bool:
        """Configure TX parameters using bulk SET command"""
        if not self.ser:
            return False
        
        # Use bulk configuration command
        cmd = f"AT+LR_TX_SET=SF:{sf},BW:{bw},CR:{cr},Freq:{freq},IQInv:{iq},HeaderMode:{header},CRC:{crc},Power:{power}"
        response = self.send_cmd(cmd, timeout=1.0, wait_all=True)
        
        if not response or "OK" not in response:
            print(f"Error: TX bulk configuration failed")
            if response:
                print(f"Response: {response}")
            return False
        
        # Set preamble separately
        response = self.send_cmd(f"AT+LR_TX_PREAMBLE_SIZE={preamble}")
        if not response or "OK" not in response:
            print(f"Error: TX preamble configuration failed")
            return False
        
        if debug:
            print("TX configuration successful")
        return True
    def configure_rx(self, sf: int, bw: int, cr: int, header: int, preamble: int, iq: int, crc: int, freq: int = 869525000, debug: bool = False) -> bool:
        """Configure RX parameters using bulk SET command"""
        if not self.ser:
            return False
        
        # Use bulk configuration command
        cmd = f"AT+LR_RX_SET=SF:{sf},BW:{bw},CR:{cr},Freq:{freq},IQInv:{iq},HeaderMode:{header},CRC:{crc}"
        response = self.send_cmd(cmd, timeout=1.0, wait_all=True)
        
        if not response or "OK" not in response:
            print(f"Error: RX bulk configuration failed")
            if response:
                print(f"Response: {response}")
            return False
        
        # Set preamble separately
        response = self.send_cmd(f"AT+LR_RX_PREAMBLE_SIZE={preamble}")
        if not response or "OK" not in response:
            print(f"Error: RX preamble configuration failed")
            return False
        
        if debug:
            print("RX configuration successful")
        return True
    
    def enable_rx_uart(self) -> bool:
        """Enable RX to UART output"""
        response = self.send_cmd("AT+RF_RX_TO_UART=ON")
        if response and "OK" in response:
            time.sleep(0.5)  # Wait for RX mode to stabilize
            return True
        return False
    
    def disable_rx_uart(self) -> bool:
        """Disable RX to UART output"""
        response = self.send_cmd("AT+RF_RX_TO_UART=OFF")
        if response and "OK" in response:
            self.ser.reset_input_buffer()  # Clear any remaining data
            return True
        return False
    
    def send_rf_packet(self, hex_data: str) -> bool:
        """Send packet via RF"""
        return self.send_cmd(f"AT+RF_TX_HEX={hex_data}", timeout=2.0)
    
    def wait_rx_packet(self, timeout: float) -> tuple | None:
        """Wait for RX packet, return (hex_data, rssi) or None"""
        if not self.ser:
            return None
        
        pattern = re.compile(r'\+RF_RX:(\d+),([0-9A-Fa-f]+),RSSI:(-?\d+)')
        start_time = time.time()
        buffer = ""
        
        while time.time() - start_time < timeout:
            if self.ser.in_waiting:
                chunk = self.ser.read(self.ser.in_waiting).decode('utf-8', errors='ignore')
                buffer += chunk
                
                match = pattern.search(buffer)
                if match:
                    data = match.group(2)
                    rssi = int(match.group(3))
                    return (data, rssi)
            
            time.sleep(0.01)
        
        return None
    

# Automatically find Silicon Labs ports
ports = find_silicon_labs_ports()
if len(ports) < 2:
    print(f"Error: Found only {len(ports)} Silicon Labs port(s), need 2")
    print(f"Available ports: {ports}")
    exit(1)

print(f"Found Silicon Labs ports: {ports}")
print(f"Using {ports[0]} for Dongle A (RX) and {ports[1]} for Dongle B (TX)\n")

dongleA = ATLoRaDongle(port=ports[0])
dongleB = ATLoRaDongle(port=ports[1])

if not dongleA.configure_rx(sf=9, bw=9, cr=45, header=1, preamble=8, iq=1, crc=1, freq=869525000, debug=True):
    print("Dongle A RX configuration failed")
    exit(1)
    
if not dongleB.configure_tx(sf=9, bw=9, cr=45, header=1, preamble=8, iq=1, crc=1, freq=869525000, debug=True):
    print("Dongle B TX configuration failed")
    exit(1)

print("\n=== Starting TX/RX Test ===")
print("Dongle A (COM5) = RX")
print("Dongle B (COM7) = TX\n")

# Enable RX mode on dongle A
if not dongleA.enable_rx_uart():
    print("Failed to enable RX mode on dongle A")
    exit(1)

if not dongleB.enable_rx_uart():
    print("Failed to enable RX mode on dongle B")
    exit(1)

for i in range(5):
    print(f"\n--- Test {i+1}/5 ---")
    
    # Send packet from dongle B
    response = dongleA.send_rf_packet("48656C6C6F20576F726C6421")  # "Hello World!" in hex
    if response and "OK" in response:
        print("TX: Packet sent successfully")
    else:
        print(f"TX: Failed to send packet")
    
    # Wait for reception on dongle A
    received_packet = dongleB.wait_rx_packet(timeout=2.0)
    if received_packet:
        data, rssi = received_packet
        print(f"RX: Received data: {data}, RSSI: {rssi} dBm")
    else:
        print("RX: No packet received within timeout")
    
    time.sleep(0.5)  # Small delay between tests

# Disable RX mode
dongleA.disable_rx_uart()

print("\n=== Test Complete ===")
dongleA.close()
dongleB.close()