import dbus
import re
import subprocess

class AuraClient:
    def __init__(self):
        self.bus = dbus.SystemBus()
        self.aura_interface = "org.asuslinux.Aura"
        
        # Detecting aura device path
        result = subprocess.run(["asusctl", "led-mode", "--help"], capture_output=True, text=True, check=True)
        match = re.search(r"Found aura device at (.+?)(?:,|$)", result.stdout)
        aura_path = "/org/asuslinux"
        if match:
            aura_path = match.group(1)

        self.aura_proxy = self.bus.get_object("org.asuslinux.Daemon", aura_path)
        self.properties_iface = dbus.Interface(self.aura_proxy, dbus_interface=dbus.PROPERTIES_IFACE)

    # Methods for org.freedesktop.DBus.Properties
    def get_property(self, interface_name: str, property_name: str):
        return self.properties_iface.Get(interface_name, property_name)

    def set_property(self, interface_name: str, property_name: str, value):
        self.properties_iface.Set(interface_name, property_name, value)
        
    """
    def get_all_properties(self, interface_name: str):
        return self.properties_iface.GetAll(interface_name)

    # Methods for org.asuslinux.Aura
    def all_mode_data(self):
        return self.aura_proxy.AllModeData(dbus_interface=self.aura_interface)

    def direct_addressing_raw(self, data):
        self.aura_proxy.DirectAddressingRaw(data, dbus_interface=self.aura_interface)
    """

    # Properties for org.asuslinux.Aura
    @property
    def brightness(self) -> int:
        return self.get_property(self.aura_interface, "Brightness")

    @brightness.setter
    def brightness(self, value: int):
        self.set_property(self.aura_interface, "Brightness", dbus.UInt32(value))

    @property
    def led_mode(self) -> int:
        return self.get_property(self.aura_interface, "LedMode")

    @led_mode.setter
    def led_mode(self, value: int):
        self.set_property(self.aura_interface, "LedMode", dbus.UInt32(value))

    """
    @property
    def device_type(self):
        return self.get_property(self.aura_interface, "DeviceType")
    
    @property
    def led_mode_data(self):
        return self.get_property(self.aura_interface, "LedModeData")

    @led_mode_data.setter
    def led_mode_data(self, value):
        self.set_property(self.aura_interface, "LedModeData", value)

    @property
    def led_power(self):
        return self.get_property(self.aura_interface, "LedPower")

    @led_power.setter
    def led_power(self, value):
        self.set_property(self.aura_interface, "LedPower", value)

    @property
    def supported_basic_modes(self):
        return self.get_property(self.aura_interface, "SupportedBasicModes")

    @property
    def supported_basic_zones(self):
        return self.get_property(self.aura_interface, "SupportedBasicZones")

    @property
    def supported_brightness(self):
        return self.get_property(self.aura_interface, "SupportedBrightness")

    @property
    def supported_power_zones(self):
        return self.get_property(self.aura_interface, "SupportedPowerZones")"""
