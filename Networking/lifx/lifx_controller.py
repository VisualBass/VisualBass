import socket
import threading
import logging
import time
import signal
import sys
from queue import Queue, Empty
from concurrent.futures import ThreadPoolExecutor
from lifxlan import LifxLAN, Light, WorkflowException

# --- Configuration ---
UDP_IP = "127.0.0.1"
UDP_PORT = 7777
BUFFER_SIZE = 64
QUEUE_TIMEOUT = 0.005

# IMPORTANT: Set this to the total number of lights you own.
# The script will stop scanning the moment it finds this many.
EXPECTED_TOTAL_LIGHTS = 4

# MANUAL SETUP: Add known IPs here to speed up startup
KNOWN_BULBS_CONFIG = [
    ("d0:73:d5:86:b4:1c", "192.168.1.2"),
    ("d0:73:d5:87:00:09", "192.168.1.7"),
]

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - [%(levelname)s] - %(message)s',
    datefmt='%H:%M:%S'
)
logger = logging.getLogger(__name__)

class BulbCommand:
    def __init__(self, glow: float, hue: float, addr):
        self.glow = glow
        self.hue = hue
        self.addr = addr

class LifxController(threading.Thread):
    def __init__(self, command_queue):
        super().__init__()
        self.command_queue = command_queue
        self.running = True
        self.lifx = LifxLAN(verbose=False)
        self.bulbs = []
        self.found_macs = set()
        self.name = "LifxWorker"

    def add_light(self, light, source="AUTO"):
        """Safe method to add a light if it's new."""
        try:
            mac = light.get_mac_addr()
            if mac not in self.found_macs:
                # Fetch label to verify connection
                label = light.get_label()
                self.bulbs.append(light)
                self.found_macs.add(mac)
                logger.info(f" -> [{source}] Added: '{label}' ({light.get_ip_addr()})")
                return True
        except:
            pass
        return False

    def scan_subnet_ip(self, ip):
        """Helper to check a single IP (used in threading)."""
        try:
            # Create temp light to test connection
            light = Light("00:00:00:00:00:00", ip)
            light.get_color() # Trigger connection
            return light
        except:
            return None

    def perform_subnet_scan(self):
        """Scans 192.168.1.1 -> 254 using Threading for speed."""
        base_ip = "192.168.1."
        logger.warning(f"Missing lights ({len(self.bulbs)}/{EXPECTED_TOTAL_LIGHTS}). Starting Subnet Scan...")

        ips = [f"{base_ip}{i}" for i in range(1, 255)]

        with ThreadPoolExecutor(max_workers=50) as executor:
            results = executor.map(self.scan_subnet_ip, ips)

            for light in results:
                if light:
                    self.add_light(light, source="SCAN")

    def discover_devices(self):
        self.bulbs = []
        self.found_macs = set()
        logger.info(f"--- Discovery Started (Target: {EXPECTED_TOTAL_LIGHTS} lights) ---")

        # PHASE 1: Manual Config (Fastest)
        for mac, ip in KNOWN_BULBS_CONFIG:
            try:
                light = Light(mac if mac else "00:00:00:00:00:00", ip)
                self.add_light(light, source="MANUAL")
            except:
                pass

        if len(self.bulbs) >= EXPECTED_TOTAL_LIGHTS:
            logger.info("All lights found via Manual Config.")
            return

        # PHASE 2: Standard Broadcast (Normal)
        logger.info("Phase 2: Broadcasting...")
        try:
            found = self.lifx.get_lights()
            for l in found:
                self.add_light(l, source="AUTO")
        except: pass

        if len(self.bulbs) >= EXPECTED_TOTAL_LIGHTS:
            return

        # PHASE 3: Brute Force Subnet Scan (Fallback)
        self.perform_subnet_scan()

        logger.info(f"--- Total Lights Managed: {len(self.bulbs)}/{EXPECTED_TOTAL_LIGHTS} ---")

    def process_color(self, cmd: BulbCommand):
        if not self.bulbs: return

        try:
            # NO FLOOR: We allow 0 brightness.
            # We just clamp to valid hardware range (0-65535)
            brightness = max(0, min(65535, int(cmd.glow * 65535)))

            lifx_hue = int(cmd.hue * 65535) % 65535
            saturation = 65535
            kelvin = 3500

            color_data = [lifx_hue, saturation, brightness, kelvin]

            for bulb in self.bulbs:
                try:
                    bulb.set_color(color_data, duration=0, rapid=True)
                except Exception:
                    pass

        except Exception as e:
            logger.error(f"Processing error: {e}")

    def run(self):
        self.discover_devices()
        while self.running:
            try:
                cmd = self.command_queue.get(timeout=QUEUE_TIMEOUT)
                self.process_color(cmd)
                self.command_queue.task_done()
            except Empty:
                continue
            except Exception:
                pass

    def stop(self):
        self.running = False

class UdpServer(threading.Thread):
    def __init__(self, host, port, command_queue):
        super().__init__()
        self.host = host
        self.port = port
        self.command_queue = command_queue
        self.running = True
        self.sock = None

    def run(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.sock.bind((self.host, self.port))
            self.sock.settimeout(1.0)
            logger.info(f"UDP Listening on {self.port}")

            while self.running:
                try:
                    data, _ = self.sock.recvfrom(BUFFER_SIZE)
                    if not data: continue
                    msg = data.decode('utf-8').strip().split(',')
                    if len(msg) >= 2:
                        self.command_queue.put(BulbCommand(float(msg[0]), float(msg[1]), None))
                except socket.timeout: continue
                except: pass
        except Exception as e:
            logger.critical(f"UDP Bind Failed: {e}")

    def stop(self):
        self.running = False
        if self.sock: self.sock.close()

def main():
    q = Queue(maxsize=20)
    worker = LifxController(q)
    server = UdpServer(UDP_IP, UDP_PORT, q)

    worker.start()
    server.start()

    try:
        while True:
            time.sleep(1)
            if not worker.is_alive() or not server.is_alive(): break
    except KeyboardInterrupt:
        worker.stop()
        server.stop()

if __name__ == "__main__":
    main()