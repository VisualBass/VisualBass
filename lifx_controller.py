import socket
import threading
import logging
import time
from lifxlan import LifxLAN

PORT = 7777

lifx = LifxLAN()
bulbs = lifx.get_lights()
bulb = bulbs[0] if bulbs else None

logging.basicConfig(level=logging.INFO)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("127.0.0.1", PORT))
sock.settimeout(1.0)  # so it doesn't block forever

def listen_and_echo():
    while True:
        try:
            data, addr = sock.recvfrom(64)
            if not data:
                continue

            try:
                msg = data.decode().strip()
                glow_str, hue_str = msg.split(',')
                glow = float(glow_str)
                hue = float(hue_str)

                # Reply immediately for ping
                sock.sendto(b"pong", addr)

                # Always update bulb on data
                brightness = max(int(glow * 65535), int(0.05 * 65535))
                lifx_hue = int(hue * 65535) % 65535
                bulb.set_color([lifx_hue, 65535, brightness, 3500], rapid=True)

                logging.info(f"[RECV] Glow={glow:.2f} Hue={hue:.2f}")

            except Exception as e:
                logging.warning(f"Failed to process message: {e}")
        except socket.timeout:
            continue  # loop again if no packet

if __name__ == "__main__":
    if bulb is None:
        logging.warning("No LIFX bulbs found.")
    else:
        logging.info(f"Connected to bulb: {bulb.get_label()}")
        listen_and_echo()
