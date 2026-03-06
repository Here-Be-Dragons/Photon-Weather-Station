import socket
import time
import json

# Network Configuration
UDP_IP = "255.255.255.255"
UDP_PORT = 50222
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

# Simulation Values
# 4 miles converted to kilometers (4 * 1.60934)
distance_km = 6.44
strike_energy = 3848 # Arbitrary energy value

def fire_strike():
    now = int(time.time())

    # The 'evt_strike' packet format
    # evt index 0: Timestamp
    # evt index 1: Distance in km
    # evt index 2: Energy
    strike_json = {
        "serial_number": "ST-00202318",
        "type": "evt_strike",
        "hub_sn": "HB-00207159",
        "evt": [now, distance_km, strike_energy]
    }

    message = json.dumps(strike_json).encode()
    sock.sendto(message, (UDP_IP, UDP_PORT))

    print(f"⚡ STRIKE SIMULATED: {distance_km} km (~4.0 miles)")
    print(f"Payload: {message.decode()}")

if __name__ == "__main__":
    fire_strike()