from .base.abstract_dbus_client import AbstractDbusClient
from ...models.thermal_throttle_profile import ThermalThrottleProfile
from ...utils.singleton import singleton
from PyQt5.QtCore import QMetaType
from PyQt5.QtDBus import QDBusArgument

@singleton
class FanCurvesClient(AbstractDbusClient):
    def __init__(self):
        super().__init__(True, 'org.asuslinux.Daemon', '/org/asuslinux', 'org.asuslinux.FanCurves')

    def resetProfileCurves(self, profile: ThermalThrottleProfile): 
        self.invokeMethod("ResetProfileCurves", QDBusArgument(profile.value, QMetaType.UInt))

    def setCurvesToDefaults(self, profile: ThermalThrottleProfile): 
        self.invokeMethod("SetCurvesToDefaults", QDBusArgument(profile.value, QMetaType.UInt))

    def setFanCurvesEnabled(self, profile: ThermalThrottleProfile, enabled: bool): 
        self.invokeMethod("SetFanCurvesEnabled", QDBusArgument(profile.value, QMetaType.UInt), enabled)

fan_curves_client = FanCurvesClient()