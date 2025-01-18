import os
import re
import socket
import subprocess
from typing import Optional
import threading
import time

# pylint:disable=E0611, E0401
from openrgb import OpenRGBClient
from openrgb.orgb import Device
from openrgb.utils import RGBColor

from rcc.models.rgb_brightness import RgbBrightness
from rcc.models.usb_identifier import UsbIdentifier
from rcc.utils.constants import orgb_path, udev_path
from rcc.utils.event_bus import event_bus
from rcc.utils.logger import Logger
from rcc.utils.singleton import singleton
from rcc.clients.tcp.openrgb.effects.base.abstract_effect import AbstractEffect
from rcc.clients.tcp.openrgb.effects.breathing import breathing_effect
from rcc.clients.tcp.openrgb.effects.dance_floor import dance_floor
from rcc.clients.tcp.openrgb.effects.digital_rain import digital_rain
from rcc.clients.tcp.openrgb.effects.rain import rain
from rcc.clients.tcp.openrgb.effects.rainbow_wave import rainbow_wave
from rcc.clients.tcp.openrgb.effects.spectrum_cycle import spectrum_cycle
from rcc.clients.tcp.openrgb.effects.starry_night import starry_night
from rcc.clients.tcp.openrgb.effects.static import static_effect


@singleton
class OpenRgbClient:
    """Client to communicate with OpenRGB Server"""

    def __init__(self):
        self.logger = Logger()
        event_bus.on("stop", self.stop)
        self._available_devices: list[Device] = []
        self._client = None
        self._orgb_thread = None
        self._orgb_process = None
        self._port = 6472
        self._compatible_devices = None
        self._available_effects: list[AbstractEffect] = [
            breathing_effect,
            dance_floor,
            digital_rain,
            rain,
            rainbow_wave,
            spectrum_cycle,
            starry_night,
            static_effect,
        ]
        self.start()

    def get_compatible_devices(self) -> list[UsbIdentifier] | None:
        """Get list of compatible devices"""
        return self._compatible_devices

    def start(self):
        """Initialize server and client"""
        self.logger.info("Initializing OpenRgbClient")
        subprocess.run(
            ["asusctl", "led-mode", "static", "-c", "000000"],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            encoding="utf-8",
        )
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

        self._orgb_thread = threading.Thread(name="ORGBServer", target=self._run_in_background)
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

    def _run_in_background(self):
        self._orgb_process = subprocess.Popen(  # pylint: disable=R1732
            [orgb_path, "--server-host", "localhost", "--server-port", str(self._port)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )  # pylint: disable=R1732

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
            if os.path.exists(udev_path):
                with open(udev_path, "r") as file:
                    content = file.read()
                lines = content.split("\n")

                regex = r'SUBSYSTEMS==".*?", ATTRS{idVendor}=="([0-9a-fA-F]+)", ATTRS{idProduct}=="([0-9a-fA-F]+)"'
                results = []
                for line in lines:
                    match = re.search(regex, line)
                    if match:
                        results.append(UsbIdentifier(match.group(1), match.group(2)))
                compatible_devices = results

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

    def apply_effect(self, effect: str, brightness: RgbBrightness, color: Optional[str] = None) -> None:
        """Apply effect with specified brightness and color"""
        inst = [i for i in self._available_effects if i.name == effect]
        if inst:
            for mode in self._available_effects:
                mode.stop()

            inst[0].start(self._available_devices, brightness, RGBColor.fromHEX(color or "#000000"))


open_rgb_client = OpenRgbClient()
