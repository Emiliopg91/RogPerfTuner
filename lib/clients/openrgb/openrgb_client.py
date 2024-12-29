from .sdk.orgb import OpenRGBClient, Device
from .sdk.utils import RGBColor
from ...models.rgb_brightness import RgbBrightness
from ...models.usb_identifier import UsbIdentifier
from ...utils.constants import orgb_path
from ...utils.event_bus import event_bus
from ...utils.logger import Logger
from ...utils.singleton import singleton
from .effects.base.abstract_effect import AbstractEffect
from .effects.breathing import breathing_effect
from .effects.dance_floor import dance_floor
from .effects.digital_rain import digital_rain
from .effects.rain import rain
from .effects.rainbow_wave import rainbow_wave
from .effects.spectrum_cycle import spectrum_cycle
from .effects.starry_night import starry_night
from .effects.static import static_effect

from typing import Optional

import pathlib
import os
import re
import socket
import subprocess
import tempfile
import threading
import time

@singleton
class OpenRgbClient:
    def __init__(self):
        self.logger = Logger("OpenRgbClient")
        event_bus.on("stop", self.stop)
        self.available_devices:list[Device] = []
        self.available_effects:list[AbstractEffect] = [
            breathing_effect, 
            dance_floor,
            digital_rain,
            rain,
            rainbow_wave,
            spectrum_cycle,
            starry_night,
            static_effect
        ]
        self.start()
        
    def start(self):
        self.logger.info("Initializing OpenRgbClient")
        self.logger.addTab()
        self._start_orgb_process()
        self._start_client()
        self._get_available_devices()
        self.client.set_color(RGBColor(0,0,0), True)
        self.logger.remTab()

    def stop(self, _=None):
        self.logger.info("Stopping OpenRgbClient")
        self.logger.addTab()
        for mode in self.available_effects:
            mode.stop()
        self.client.set_color(RGBColor(0,0,0), True)
        time.sleep(0.2)
        self._stop_orgb_process()
        self.logger.remTab()

    def _start_client(self):
        self.logger.info("Connecting to server")
        self.logger.addTab()
        self.client = OpenRGBClient(port=self.port, name="RogControlCenter")
        self.client.connect()
        self.logger.info("Connected")
        self.logger.remTab()

    def _stop_client(self):
        self.client.disconnect()
        self.client = None

    def _get_available_devices(self):
        self.logger.info("Getting available devices")
        self.logger.addTab()
        self.available_devices = self.client.ee_devices
        for dev in self.available_devices:
            self.logger.info(dev.name)
        self.logger.remTab()

    def _start_orgb_process(self):
        self.logger.info("Starting OpenRgb server")
        self.logger.addTab()
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind(("", 0)) 
            self.port = s.getsockname()[1]
        
        self.orgb_thread = threading.Thread(name="ORGBServer", target=self.run_in_background)
        self.orgb_thread.start()

        self._wait_for_server()
        self.logger.info("OpenRgb server ready")
        self.compatible_devices = self._find_compatible_devices()
        self.logger.remTab()

    def _stop_orgb_process(self):
        self.orgb_process.kill()
        
    def run_in_background(self):
        self.orgb_process = subprocess.Popen(
                                    [orgb_path,
                                    '--server-host', "localhost",
                                    '--server-port', str(self.port)],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE,
                                    text=True
                                )

        for _ in self.orgb_process.stdout:
            pass

        for _ in self.orgb_process.stderr:
            pass

        self.orgb_process.wait()
        self.logger.info(f"OpenRgb finished with code {self.orgb_process.returncode}")
        self.orgb_process = None

    def _wait_for_server(self) -> bool:
        while True:
            try:
                with socket.create_connection(("localhost", self.port), timeout=1):
                    break
            except Exception:
                time.sleep(0.1)

    def _find_compatible_devices(self):
        self.logger.info("Reading udev rules")
        compatible_devices = None
        tmp_dir = pathlib.Path(tempfile.gettempdir())
        entries = [entry for entry in tmp_dir.iterdir() if entry.is_dir()]

        matching_entries = [
            entry.absolute() for entry in entries
            if entry.name.startswith('.mount_' + pathlib.Path(os.path.basename(orgb_path)).stem[:6])
        ]

        for mount_dir in matching_entries:
            try:
                if compatible_devices is None:
                    udev_path = os.path.join(mount_dir, 'usr', 'lib', 'udev', 'rules.d', '60-openrgb.rules')
                    if os.path.exists(udev_path):
                        with open(udev_path, 'r') as file:
                            content = file.read()
                        lines = content.split('\n')

                        regex = r'SUBSYSTEMS==".*?", ATTRS{idVendor}=="([0-9a-fA-F]+)", ATTRS{idProduct}=="([0-9a-fA-F]+)"'
                        results = []
                        for line in lines:
                            match = re.search(regex, line)
                            if match:
                                results.append(UsbIdentifier(match.group(1), match.group(2)))
                        compatible_devices = results
            except Exception:
                pass

        return compatible_devices
    
    def get_available_effects(self) -> list[AbstractEffect]:
        return [effect.name for effect in self.available_effects]
    
    def supports_color(self, effect) -> bool:
        effects = [e for e in self.available_effects if e.name == effect]
        if len(effects) > 0:
            return effects[0].supports_color
        return False
    
    def get_color(self, effect) -> str|None:
        effects = [e for e in self.available_effects if e.name == effect]
        if len(effects) > 0:
            return effects[0].color
        return None
    
    def apply_effect(self, effect: str, brightness: RgbBrightness, color: Optional[str] = None) -> None:
        inst = [i for i in self.available_effects if i.name == effect]
        if inst:
            for mode in self.available_effects:
                mode.stop()

            inst[0].start(self.available_devices, brightness, RGBColor.fromHEX(color or '#000000'))

open_rgb_client = OpenRgbClient()