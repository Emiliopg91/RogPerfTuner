# pylint: disable=E0611, E0401
from PyQt5.QtCore import QMetaType
from PyQt5.QtDBus import QDBusVariant, QDBusArgument

from lib.clients.dbus.base.abstract_dbus_client import AbstractDbusClient
from lib.models.battery_threshold import BatteryThreshold
from lib.models.thermal_throttle_profile import ThermalThrottleProfile
from lib.utils.singleton import singleton


@singleton
class PlatformClient(AbstractDbusClient):
    """DBus platform client"""

    def __init__(self):
        super().__init__(True, "org.asuslinux.Daemon", "/org/asuslinux", "org.asuslinux.Platform")

    @property
    def charge_control_end_threshold(self) -> BatteryThreshold:
        """Battery charge limit"""
        return BatteryThreshold(int.from_bytes(self._get_property("ChargeControlEndThreshold")))

    @charge_control_end_threshold.setter
    def charge_control_end_threshold(self, val: BatteryThreshold) -> None:
        self._set_property(
            "ChargeControlEndThreshold",
            QDBusVariant(QDBusArgument(val.value, QMetaType.UChar)),
        )

    @property
    def throttle_thermal_policy(self) -> ThermalThrottleProfile:
        """Thermal throttle policy"""
        return ThermalThrottleProfile(self._get_property("ThrottleThermalPolicy"))

    @throttle_thermal_policy.setter
    def throttle_thermal_policy(self, val: ThermalThrottleProfile) -> None:
        self._set_property(
            "ThrottleThermalPolicy",
            QDBusVariant(QDBusArgument(val.value, QMetaType.UInt)),
        )


platform_client = PlatformClient()
