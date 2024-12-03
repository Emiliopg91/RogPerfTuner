from enum import Enum
from openrgb import OpenRGBClient
from openrgb.utils import ModeData, ModeColors, RGBColor, Profile
from openrgb.orgb import Device

import asyncio
import sys
import time

class BrightnessLevel(Enum):
    OFF = 0
    LOW = 1
    MED = 2
    HIGH = 3

class OpenRgbClient:
    available_modes = {}
    device_mode_data = {}
    cli = None

    def __init__(self):
        print("Connecting to OpenRGB server...")
        self.cli = OpenRGBClient(port=int(sys.argv[3]))
        print("Connected")

        print("Scanning for compatible devices...")
        common_modes = None
        devices = []
        for device in self.cli.ee_devices:
            """active_mode = [mode for mode in device.modes if mode.id == device.active_mode][0]"""
            devices.append(device.name)
            device_modes = {mode.name for mode in device.modes if mode.name != "Direct" and mode.name != "Rainbow Wave" and mode.name != "Off"  and mode.name != "Off" and "Reactive" not in mode.name}
            
            if common_modes is None:
                common_modes = device_modes
            else:
                common_modes.intersection_update(device_modes)
        print(f"Devices found: {devices}")
            
        self.available_modes = common_modes
        print(f"Common modes across all devices: {common_modes}")
            
        for device in self.cli.devices:
            if device.name in devices:
                self.device_mode_data[device.name] = {}
                for mode in device.modes:
                    if mode.name in common_modes:
                        if mode.speed_min is not None and mode.speed_max is not None:
                            mode.speed = round(mode.speed_min + ((mode.speed_max - mode.speed_min) * 0.855))
                        self.device_mode_data[device.name][mode.name] = mode

        self.cli.update_profiles()

    async def set_mode_async(self, device:Device, mode:ModeData, brightness:BrightnessLevel, color:str):    
        old_brightness = mode.brightness
        if mode.brightness_min is not None:
            if brightness == BrightnessLevel.OFF.value:
                mode.brightness = mode.brightness_min
            elif brightness == BrightnessLevel.LOW.value:
                mode.brightness = round((mode.brightness_max + mode.brightness_min)/3)
            elif brightness == BrightnessLevel.MED.value:
                mode.brightness = round(2*(mode.brightness_max + mode.brightness_min)/3)
            else:
                mode.brightness = mode.brightness_max

        color = RGBColor.fromHEX(color)
        if mode.color_mode is ModeColors.MODE_SPECIFIC:
            old_colors = mode.colors
            mode.colors = [color]
            await asyncio.to_thread(device.set_mode, mode)
            mode.colors = old_colors
        else:
            await asyncio.to_thread(device.set_mode, mode)
            if mode.color_mode is ModeColors.PER_LED:
                for led in device.leds:
                    led.set_color(color, True)

        mode.brightness = old_brightness
        
    async def set_mode(self, mode, brightness: BrightnessLevel, color=None):
        if mode in self.available_modes:
            t0 = time.time()
            print(f"Setting mode {mode}")
            tasks = []
            for device in self.cli.devices:
                if self.device_mode_data[device.name] is not None:
                    mode_data = self.device_mode_data[device.name][mode]
                    tasks.append(self.set_mode_async(device, mode_data, brightness, color))
            await asyncio.gather(*tasks)       
            print(f"Mode setted in {time.time() - t0}s")
        else:
            print(f"Mode {mode} not available")