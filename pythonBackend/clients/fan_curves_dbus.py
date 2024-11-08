import dbus
from typing import Tuple

class FanCurveData:
    def __init__(self, name: str, data: Tuple[Tuple[int, int], Tuple[int, int], bool]):
        self.name = name
        self.data = data

class FanCurvesClient:
    def __init__(self):
        self.bus = dbus.SystemBus()
        self.service_name = "org.asuslinux.Daemon"
        self.object_path = "/org/asuslinux"
        self.interface_name = "org.asuslinux.FanCurves"
        self.proxy = self.bus.get_object(self.service_name, self.object_path)

    # Métodos de la interfaz
    def set_fan_curves_enabled(self, profile: int, enabled: bool):
        method = self.proxy.get_dbus_method("SetFanCurvesEnabled", self.interface_name)
        method(profile, enabled)
    
    def set_curves_to_defaults(self, profile: int):
        method = self.proxy.get_dbus_method("SetCurvesToDefaults", self.interface_name)
        method(profile)

    def reset_profile_curves(self, profile: int):
        method = self.proxy.get_dbus_method("ResetProfileCurves", self.interface_name)
        method(profile)

    """def set_profile_fan_curve_enabled(self, profile: ThrottleThermalPolicy, fan: str, enabled: bool):
        method = self.proxy.get_dbus_method("SetProfileFanCurveEnabled", self.interface_name)
        method(profile.value, fan, enabled)

    def fan_curve_data(self, profile: ThrottleThermalPolicy) -> List[FanCurveData]:
        method = self.proxy.get_dbus_method("FanCurveData", self.interface_name)
        result = method(profile.value)
        fan_curve_data_list = [FanCurveData(name, data) for name, data in result]
        return fan_curve_data_list

    def set_fan_curve(self, profile: ThrottleThermalPolicy, curve: FanCurveData):
        method = self.proxy.get_dbus_method("SetFanCurve", self.interface_name)
        method(profile.value, curve.data)
    """
