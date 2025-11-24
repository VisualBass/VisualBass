import socket
import threading
import logging
import time
import signal
import sys
from queue import Queue, Empty
from lifxlan import LifxLAN, Light, WorkflowException

# --- Configuration ---
UDP_IP = "127.0.0.1"
UDP_PORT = 7777
BUFFER_SIZE = 64
QUEUE_TIMEOUT = 0.005

# Manual Configuration: Add your specific lights here
KNOWN_BULBS_CONFIG = [
    ("d0:73:d5:86:b4:1c", "192.168.1.2"),
    ("d0:73:d5:87:00:09", "192.168.1.7"),
]

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - [%(levelname)s] - %(threadName)s - %(message)s',
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
        self.name = "LifxWorker"

    def discover_devices(self):
        """
        Combines manual IP connection (Unicast) with auto-discovery (Broadcast).
        """
        self.bulbs = []
        logger.info("Starting Device Discovery...")

        # 1. Try Manual Connections (Unicast) - Reliable
        for mac, ip in KNOWN_BULBS_CONFIG:
            try:
                logger.info(f"Attempting direct connection to {ip}...")
                light = Light(mac, ip)
                # We try to get the label to verify the connection works
                label = light.get_label()
                self.bulbs.append(light)
                logger.info(f" -> Success! Added manual light: '{label}' ({ip})")
            except Exception as e:
                logger.warning(f" -> Failed to connect to manual light {ip}: {e}")

        # 2. Try Auto Discovery (Broadcast) - For any others
        logger.info("Scanning network for additional lights...")
        try:
            found_lights = self.lifx.get_lights()
            
            for light in found_lights:
                # --- FIX: Use get_mac_addr() instead of get_mac_addr_str() ---
                # This returns the raw MAC (usually a tuple or int) for comparison
                current_macs = [b.get_mac_addr() for b in self.bulbs]
                
                if light.get_mac_addr() not in current_macs:
                    self.bulbs.append(light)
                    logger.info(f" -> Auto-discovered light: '{light.get_label()}'")
                    
        except Exception as e:
            logger.error(f"Auto-discovery failed (ignoring): {e}")

        logger.info(f"Total Lights Managed: {len(self.bulbs)}")
        
        # If we have 0 lights, we must retry. If we have partial lights, we continue.
        if len(self.bulbs) == 0:
            logger.warning("No lights found via Config OR Discovery. Retrying in 5s...")
            time.sleep(5)
            self.discover_devices() # Recursive retry

    def process_color(self, cmd: BulbCommand):
        if not self.bulbs:
            return

        try:
            # Brightness floor of 5% to prevent "off" state disconnection issues
            brightness = max(int(cmd.glow * 65535), int(0.05 * 65535))
            lifx_hue = int(cmd.hue * 65535) % 65535
            kelvin = 3500
            saturation = 65535
            
            color_data = [lifx_hue, saturation, brightness, kelvin]

            for bulb in self.bulbs:
                try:
                    # rapid=True avoids waiting for packet acknowledgement
                    bulb.set_color(color_data, duration=0, rapid=True)
                except WorkflowException:
                    pass 
                except OSError:
                    pass 

        except Exception as e:
            logger.error(f"Batch processing error: {e}")

    def run(self):
        self.discover_devices()

        while self.running:
            try:
                cmd = self.command_queue.get(timeout=QUEUE_TIMEOUT)
                self.process_color(cmd)
                self.command_queue.task_done()
            except Empty:
                continue
            except Exception as e:
                logger.error(f"Worker thread error: {e}")

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
        self.name = "UdpListener"

    def setup_socket(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.sock.bind((self.host, self.port))
            self.sock.settimeout(1.0)
            logger.info(f"UDP Server listening on {self.host}:{self.port}")
        except OSError as e:
            logger.critical(f"Failed to bind port {self.port}: {e}")
            self.running = False

    def run(self):
        self.setup_socket()
        while self.running and self.sock:
            try:
                data, addr = self.sock.recvfrom(BUFFER_SIZE)
                if not data: continue

                msg = data.decode('utf-8').strip()
                parts = msg.split(',')
                
                if len(parts) == 2:
                    glow = float(parts[0])
                    hue = float(parts[1])
                    self.sock.sendto(b"pong", addr)
                    self.command_queue.put(BulbCommand(glow, hue, addr))

            except socket.timeout:
                continue 
            except Exception as e:
                logger.error(f"Socket error: {e}")

    def stop(self):
        self.running = False
        if self.sock:
            self.sock.close()

def main():
    cmd_queue = Queue(maxsize=20)
    lifx_worker = LifxController(cmd_queue)
    udp_server = UdpServer(UDP_IP, UDP_PORT, cmd_queue)

    lifx_worker.start()
    udp_server.start()

    def signal_handler(sig, frame):
        print("\nRequesting shutdown...")
        udp_server.stop()
        lifx_worker.stop()
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)
    logger.info("System running. Press Ctrl+C to stop.")
    
    try:
        while True:
            time.sleep(1)
            if not lifx_worker.is_alive() or not udp_server.is_alive():
                logger.critical("Thread died. Exiting.")
                break
    except KeyboardInterrupt:
        signal_handler(None, None)

if __name__ == "__main__":
    main()
    