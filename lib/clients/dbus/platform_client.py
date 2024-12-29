from .base.abstract_dbus_client import AbstractDbusClient
from ...models.battery_threshold import BatteryThreshold
from ...models.thermal_throttle_profile import ThermalThrottleProfile
from ...utils.singleton import singleton
from PyQt5.QtCore import QMetaType
from PyQt5.QtDBus import QDBusVariant, QDBusArgument

@singleton
class PlatformClient(AbstractDbusClient):
    def __init__(self):
        super().__init__(True, 'org.asuslinux.Daemon', '/org/asuslinux', 'org.asuslinux.Platform')

    @property
    def chargeControlEndThreshold(self) -> BatteryThreshold:
        return BatteryThreshold(int.from_bytes(self.getProperty("ChargeControlEndThreshold")))

    @chargeControlEndThreshold.setter
    def chargeControlEndThreshold(self, val:BatteryThreshold) -> None:
        self.setProperty("ChargeControlEndThreshold", QDBusVariant(QDBusArgument(val.value, QMetaType.UChar)))
        
    @property
    def throttleThermalPolicy(self) -> ThermalThrottleProfile:
        return ThermalThrottleProfile(self.getProperty("ThrottleThermalPolicy"))

    @throttleThermalPolicy.setter
    def throttleThermalPolicy(self, val:ThermalThrottleProfile) -> None:
        self.setProperty("ThrottleThermalPolicy", QDBusVariant(QDBusArgument(val.value, QMetaType.UInt)))
    
platform_client = PlatformClient()