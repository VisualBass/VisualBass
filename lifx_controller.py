import socket
import struct
import logging
import time

logging.basicConfig(level=logging.INFO)

DISCOVERY_PORT = 56700
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.settimeout(0.5)

bulbs = {}

# Use LIFX LAN discovery (works with all models A19–A22)
from lifxlan import LifxLAN
lifx = LifxLAN()

def discover_bulbs():
    logging.info("Starting robust LIFX discovery...")
    try:
        all_bulbs = lifx.get_lights()  # no timeout param
        for b in all_bulbs:
            ip = b.get_ip_addr()
            mac = b.get_mac_addr()
            bulbs[mac] = b
            h, s, b_, k = b.get_color()
            logging.info(f"{b.get_label()} ({ip}) -> H:{h} S:{s} B:{b_} K:{k}")
    except Exception as e:
        logging.warning(f"Error discovering bulbs: {e}")
    logging.info(f"Total bulbs discovered: {len(bulbs)} -> {[b.get_ip_addr() for b in bulbs.values()]}")

def listen_and_echo(listen_port=7777):
    listener = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    listener.bind(('0.0.0.0', listen_port))
    logging.info("Listening for UDP commands...")

    while True:
        try:
            data, addr = listener.recvfrom(64)
            if not data:
                continue
            glow_str, hue_str = data.decode().strip().split(',')
            glow = float(glow_str)
            hue = float(hue_str)
            listener.sendto(b"pong", addr)

            for bulb in bulbs.values():
                brightness = max(int(glow * 65535), int(0.05 * 65535))
                lifx_hue = int(hue * 65535) % 65535
                bulb.set_color([lifx_hue, 65535, brightness, 3500], rapid=True)

            logging.info(f"[RECV] Glow={glow:.2f} Hue={hue:.2f} -> Updated {len(bulbs)} bulbs")
        except Exception as e:
            logging.warning(f"Failed to process message: {e}")

if __name__ == "__main__":
    discover_bulbs()
    listen_and_echo()
