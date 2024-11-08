import dbus
from typing import Any

class PlatformClient:
    def __init__(self):
        self.bus = dbus.SystemBus()
        self.service_name = "org.asuslinux.Daemon"
        self.object_path = "/org/asuslinux"
        self.interface_name = "org.asuslinux.Platform"
        self.proxy = self.bus.get_object(self.service_name, self.object_path)

    # Métodos de la interfaz
    """def supported_properties(self) -> List[str]:
        method = self.proxy.get_dbus_method("SupportedProperties", self.interface_name)
        return method()

    def next_throttle_thermal_policy(self):
        method = self.proxy.get_dbus_method("NextThrottleThermalPolicy", self.interface_name)
        method()
    """

    # Métodos para leer y escribir propiedades
    
    def get_property(self, property_name: str):
        return self.proxy.Get(self.interface_name, property_name, dbus_interface="org.freedesktop.DBus.Properties")
    

    def set_property(self, property_name: str, value: Any):
        self.proxy.Set(self.interface_name, property_name, value, dbus_interface="org.freedesktop.DBus.Properties")

    # Propiedades
    @property
    def throttle_thermal_policy(self) -> int:
        return self.get_property("ThrottleThermalPolicy")

    @throttle_thermal_policy.setter
    def throttle_thermal_policy(self, mode: int):
        self.set_property("ThrottleThermalPolicy", dbus.UInt32(mode))

    @property
    def charge_control_end_threshold(self) -> int:
        return int(self.get_property("ChargeControlEndThreshold"))
    
    @charge_control_end_threshold.setter
    def charge_control_end_threshold(self, value: int):
        self.set_property("ChargeControlEndThreshold", dbus.Byte(value))

    """        
    @property
    def ppt_pl1_spl(self) -> int:
        return int(self.get_property("PptPl1Spl"))

    @ppt_pl1_spl.setter
    def ppt_pl1_spl(self, value: int):
        self.set_property("PptPl1Spl", dbus.Byte(value))

    @property
    def ppt_pl2_sppt(self) -> int:
        return int(self.get_property("PptPl2Sppt"))

    @ppt_pl2_sppt.setter
    def ppt_pl2_sppt(self, value: int):
        self.set_property("PptPl2Sppt", dbus.Byte(value))

    @property
    def boot_sound(self) -> bool:
        return bool(self.get_property("BootSound"))

    @boot_sound.setter
    def boot_sound(self, value: bool):
        self.set_property("BootSound", dbus.Boolean(value))

    @property
    def gpu_mux_mode(self) -> int:
        return int(self.get_property("GpuMuxMode"))
    
    @gpu_mux_mode.setter
    def gpu_mux_mode(self, value: int):
        self.set_property("GpuMuxMode", dbus.Byte(value))
        
    @property
    def panel_od(self) -> bool:
        return bool(self.get_property("PanelOd"))

    @panel_od.setter
    def panel_od(self, value: bool):
        self.set_property("PanelOd", dbus.Boolean(value))

    @property
    def change_throttle_policy_on_ac(self) -> bool:
        return self.get_property("ChangeThrottlePolicyOnAc")

    @change_throttle_policy_on_ac.setter
    def change_throttle_policy_on_ac(self, value: bool):
        self.set_property("ChangeThrottlePolicyOnAc", dbus.Boolean(value))

    @property
    def change_throttle_policy_on_battery(self) -> bool:
        return self.get_property("ChangeThrottlePolicyOnBattery")

    @change_throttle_policy_on_battery.setter
    def change_throttle_policy_on_battery(self, value: bool):
        self.set_property("ChangeThrottlePolicyOnBattery", dbus.Boolean(value))
        
    @property
    def dgpu_disable(self) -> bool:
        return self.get_property("DgpuDisable")
        
    @property
    def egpu_enable(self) -> bool:
        return self.get_property("EgpuEnable")

    @property
    def mini_led_mode(self) -> bool:
        return self.get_property("MiniLedMode")

    @mini_led_mode.setter
    def mini_led_mode(self, value: bool):
        self.set_property("MiniLedMode", dbus.Boolean(value))

    @property
    def nv_dynamic_boost(self) -> int:
        return self.get_property("NvDynamicBoost")

    @nv_dynamic_boost.setter
    def nv_dynamic_boost(self, value: int):
        self.set_property("NvDynamicBoost", dbus.Byte(value))

    @property
    def nv_temp_target(self) -> int:
        return self.get_property("NvTempTarget")

    @nv_temp_target.setter
    def nv_temp_target(self, value: int):
        self.set_property("NvTempTarget", dbus.Byte(value))

    @property
    def ppt_apu_sppt(self) -> int:
        return self.get_property("PptApuSppt")

    @ppt_apu_sppt.setter
    def ppt_apu_sppt(self, value: int):
        self.set_property("PptApuSppt", dbus.Byte(value))

    @property
    def ppt_fppt(self) -> int:
        return self.get_property("PptFppt")

    @ppt_fppt.setter
    def ppt_fppt(self, value: int):
        self.set_property("PptFppt", dbus.Byte(value))

    @property
    def ppt_platform_sppt(self) -> int:
        return self.get_property("PptPlatformSppt")

    @ppt_platform_sppt.setter
    def ppt_platform_sppt(self, value: int):
        self.set_property("PptPlatformSppt", dbus.Byte(value))

    @property
    def throttle_balanced_epp(self) -> int:
        return self.get_property("ThrottleBalancedEpp")

    @throttle_balanced_epp.setter
    def throttle_balanced_epp(self, value: int):
        self.set_property("ThrottleBalancedEpp", dbus.UInt32(value))

    @property
    def throttle_performance_epp(self) -> int:
        return self.get_property("ThrottlePerformanceEpp")

    @throttle_performance_epp.setter
    def throttle_performance_epp(self, value: int):
        self.set_property("ThrottlePerformanceEpp", dbus.UInt32(value))

    @property
    def throttle_policy_linked_epp(self) -> bool:
        return self.get_property("ThrottlePolicyLinkedEpp")

    @throttle_policy_linked_epp.setter
    def throttle_policy_linked_epp(self, value: bool):
        self.set_property("ThrottlePolicyLinkedEpp", dbus.Boolean(value))

    @property
    def throttle_policy_on_ac(self) -> int:
        return self.get_property("ThrottlePolicyOnAc")

    @throttle_policy_on_ac.setter
    def throttle_policy_on_ac(self, value: int):
        self.set_property("ThrottlePolicyOnAc", dbus.UInt32(value))

    @property
    def throttle_policy_on_battery(self) -> int:
        return self.get_property("ThrottlePolicyOnBattery")

    @throttle_policy_on_battery.setter
    def throttle_policy_on_battery(self, value: int):
        self.set_property("ThrottlePolicyOnBattery", dbus.UInt32(value))

    @property
    def throttle_quiet_epp(self) -> int:
        return self.get_property("ThrottleQuietEpp")

    @throttle_quiet_epp.setter
    def throttle_quiet_epp(self, value: int):
        self.set_property("ThrottleQuietEpp", dbus.UInt32(value))

    @property
    def version(self) -> str:
        return self.get_property("Version")
    """
