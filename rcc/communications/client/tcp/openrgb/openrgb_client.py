import os
import re
import socket
import subprocess
import sys
from typing import Optional
import threading
import time

from rcc.communications.client.tcp.openrgb.client.orgb import Device, OpenRGBClient
from rcc.communications.client.tcp.openrgb.client.utils import RGBColor

from rcc.models.rgb_brightness import RgbBrightness
from rcc.models.usb_identifier import UsbIdentifier
from rcc.utils.constants import ORGB_PATH, UDEV_PATH
from rcc.communications.client.tcp.openrgb.effects.base.abstract_effect import AbstractEffect
from rcc.communications.client.tcp.openrgb.effects.breathing import BREATHING_EFFECT
from rcc.communications.client.tcp.openrgb.effects.dance_floor import DANCE_FLOOR_EFFECT
from rcc.communications.client.tcp.openrgb.effects.digital_rain import DIGITAL_RAIN_EFFECT
from rcc.communications.client.tcp.openrgb.effects.drops import DROPS_EFFECT
from rcc.communications.client.tcp.openrgb.effects.rainbow_wave import RAINBOW_WAVE_EFFECT
from rcc.communications.client.tcp.openrgb.effects.spectrum_cycle import SPECTRUM_CYCLE_EFFECT
from rcc.communications.client.tcp.openrgb.effects.starry_night import STARRY_NIGHT_EFFECT
from rcc.communications.client.tcp.openrgb.effects.static import STATIC_EFFECT
from rcc.utils.shell import SHELL
from rcc.utils.beans import EVENT_BUS
from framework.logger import Logger
from framework.singleton import singleton


@singleton
class OpenRgbClient:
    """Client to communicate with OpenRGB Server"""

    def __init__(self):
        self.logger = Logger()
        EVENT_BUS.on("stop", self.stop)
        self._available_devices: list[Device] = []
        self._client = None
        self._orgb_thread = None
        self._orgb_process = None
        self._port = 6472
        self._compatible_devices = None
        self._available_effects: list[AbstractEffect] = [
            BREATHING_EFFECT,
            DANCE_FLOOR_EFFECT,
            DIGITAL_RAIN_EFFECT,
            DROPS_EFFECT,
            RAINBOW_WAVE_EFFECT,
            SPECTRUM_CYCLE_EFFECT,
            STARRY_NIGHT_EFFECT,
            STATIC_EFFECT,
        ]
        self.start()

    def start(self):
        """Initialize server and client"""
        self.logger.info("Initializing OpenRgbClient")
        SHELL.run_command("asusctl aura static -c 000000")
        self.logger.add_tab()
        self._start_orgb_process()
        self._find_compatible_devices()
        self._start_client()
        self._get_available_devices()
        self._client.set_color(RGBColor(0, 0, 0), True)
        self.logger.rem_tab()

    def stop(self, _=None):
        """Stop server and client"""
        self.logger.info("Stopping OpenRgbClient")
        self.logger.add_tab()
        for mode in self._available_effects:
            mode.stop()
        self._client.set_color(RGBColor(0, 0, 0), True)
        time.sleep(0.2)
        self._stop_orgb_process()
        self.logger.rem_tab()

    def _start_client(self):
        """Initialize client"""
        self.logger.info("Connecting to server")
        self.logger.add_tab()
        self._client = OpenRGBClient(port=self._port, name="RogControlCenter")
        self._client.connect()
        self.logger.info("Connected")
        self.logger.rem_tab()

    def _stop_client(self):
        """Stop client"""
        self._client.disconnect()
        self._client = None

    def _get_available_devices(self):
        self.logger.info("Getting available devices")
        self.logger.add_tab()
        self._available_devices = self._client.ee_devices
        for dev in self._available_devices:
            self.logger.info(dev.name)
        self.logger.rem_tab()

    def _start_orgb_process(self):
        self.logger.info("Starting OpenRgb server")
        self.logger.add_tab()
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind(("", 0))
            self._port = s.getsockname()[1]

        self._orgb_thread = threading.Thread(name="ORGBServer", target=self._runner)
        self._orgb_thread.start()

        self._wait_for_server()
        self.logger.info("OpenRgb server ready")
        self.logger.rem_tab()

    @property
    def compatible_devices(self) -> list[UsbIdentifier]:
        """Getter for compatible devices"""
        return self._compatible_devices

    def _stop_orgb_process(self):
        self._orgb_process.kill()

    def _runner(self):
        command = [ORGB_PATH, "--server-host", "localhost", "--server-port", str(self._port)]
        if "--orgb-gui" in sys.argv:
            command.append("--gui")
        self._orgb_process = subprocess.Popen(  # pylint: disable=R1732,
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        for _ in self._orgb_process.stdout:
            pass

        for _ in self._orgb_process.stderr:
            pass

        self._orgb_process.wait()
        self.logger.info(f"OpenRgb finished with code {self._orgb_process.returncode}")
        self._orgb_process = None

    def _wait_for_server(self) -> bool:
        while True:
            try:
                with socket.create_connection(("localhost", self._port), timeout=1):
                    break
            except Exception:
                time.sleep(0.1)

    def _find_compatible_devices(self):
        self.logger.info("Reading udev rules")
        compatible_devices = None

        if compatible_devices is None:
            if os.path.exists(UDEV_PATH):
                regex = r'SUBSYSTEMS==".*?", ATTRS{idVendor}=="([0-9a-fA-F]+)", ATTRS{idProduct}=="([0-9a-fA-F]+)".*?TAG\+="([a-zA-Z0-9_]+)"'  # pylint: disable=C0301
                with open(UDEV_PATH, "r") as file:
                    content = file.read()
                lines = content.split("\n")

                compatible_devices: list[UsbIdentifier] = []
                for line in lines:
                    line = line.replace(', TAG+="uaccess"', "")
                    match = re.search(regex, line)
                    if match:
                        vendor_id = match.group(1)
                        product_id = match.group(2)
                        device_name = match.group(3).replace("_", " ")
                        compatible_devices.append(UsbIdentifier(vendor_id, product_id, device_name))

        self._compatible_devices = compatible_devices

    def get_available_effects(self) -> list[AbstractEffect]:
        """Get all effects"""
        return [effect.name for effect in self._available_effects]

    def supports_color(self, effect) -> bool:
        """Check if effect supports color"""
        effects = [e for e in self._available_effects if e.name == effect]
        if len(effects) > 0:
            return effects[0].supports_color
        return False

    def get_color(self, effect) -> str | None:
        """Get current hex color or none"""
        effects = [e for e in self._available_effects if e.name == effect]
        if len(effects) > 0:
            return effects[0].color
        return None

    def apply_effect(self, effect: str, brightness: RgbBrightness, color: Optional[str] = None) -> bool:
        """Apply effect with specified brightness and color"""
        inst = [i for i in self._available_effects if i.name == effect]
        if inst:
            for mode in self._available_effects:
                mode.stop()

            inst[0].start(self._available_devices, brightness, RGBColor.fromHEX(color or "#000000"))
            return inst[0].supports_color
        return False

    def disable_device(self, name: str):
        """Disable certain device"""
        dev = next((d for d in self._available_devices if d.name == name), None)
        if dev is not None:
            dev.enabled = False

    def get_device_name(self, vendor: int, product: int):
        """Get device name by vendor and product id"""
        for dev in self._compatible_devices:
            if dev.id_vendor == vendor and dev.id_product == product:
                return dev.name
        return None

    def append_custom_effect(self, effect: AbstractEffect):
        """Add custom effect to availability"""
        self._available_effects.append(effect)
        self._available_effects = sorted(self._available_effects, key=lambda e: e.name)


OPEN_RGB_CLIENT = OpenRgbClient()
