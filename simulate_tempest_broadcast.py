import socket
import time
import json
import random

# Network setup for UDP broadcast
UDP_IP = "255.255.255.255"
UDP_PORT = 50222
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

print(f"Broadcasting Fake Tempest Data on Port {UDP_PORT}...")

last_obs_time = 0

try:
    while True:
        current_time = int(time.time())

        # --- Generate Fake Values within your Requested Ranges ---
        temp_f = random.uniform(0, 100)
        humidity = random.uniform(0, 100)
        pressure_mb = random.uniform(960, 1060)
        wind_mph = random.uniform(0, 30)
        precip_chance_simulated = random.uniform(0, 100) # Treating as % for your test, scaling to 0-1 inch equivalent

        # --- Convert to Native Tempest Metric Units ---
        temp_c = (temp_f - 32) * 5.0/9.0
        wind_mps = wind_mph / 2.23694
        # Converting 0-100% "chance" into 0-1 inch (0-25.4 mm) of rain for the dial test
        rain_mm = (precip_chance_simulated / 100.0) * 25.4

        # 1. Broadcast Rapid Wind (Every 3 seconds)
        wind_json = {
            "serial_number": "ST-FAKE123",
            "type": "rapid_wind",
            "hub_sn": "HB-FAKE123",
            "ob": [current_time, round(wind_mps, 2), random.randint(0, 359)]
        }
        sock.sendto(json.dumps(wind_json).encode(), (UDP_IP, UDP_PORT))
        print(f"[rapid_wind] Sent: {wind_mph:.1f} MPH")

        # 2. Broadcast Station Observation (Every 10 seconds for testing)
        if current_time - last_obs_time >= 10:
            obs_json = {
                "serial_number": "ST-FAKE123",
                "type": "obs_st",
                "hub_sn": "HB-FAKE123",
                "obs": [[
                    current_time,
                    0, 0, 0, 0, 3,
                    round(pressure_mb, 1), # Index 6
                    round(temp_c, 1),      # Index 7
                    round(humidity, 1),    # Index 8
                    0, 0, 0,
                    round(rain_mm, 2),     # Index 12
                    0, 0, 0, 0, 1
                ]]
            }
            sock.sendto(json.dumps(obs_json).encode(), (UDP_IP, UDP_PORT))
            print(f"[obs_st] Sent: {temp_f:.1f}F, {humidity:.1f}%, {pressure_mb:.1f}MB, Rain Eqv: {precip_chance_simulated:.0f}%")
            last_obs_time = current_time

        time.sleep(3)

except KeyboardInterrupt:
    print("\nStopping fake broadcast.")