# pylint: disable=E0611, E0401
from PyQt5.QtCore import QMetaType
from PyQt5.QtDBus import QDBusVariant, QDBusArgument

from rcc.clients.dbus.base.abstract_dbus_client import AbstractDbusClient
from rcc.models.battery_threshold import BatteryThreshold
from rcc.models.platform_profile import PlatformProfile
from rcc.utils.singleton import singleton


@singleton
class PlatformClient(AbstractDbusClient):
    """DBus platform client"""

    def __init__(self):
        super().__init__(True, "xyz.ljones.Asusd", "/xyz/ljones", "xyz.ljones.Platform")

    @property
    def charge_control_end_threshold(self) -> BatteryThreshold:
        """Battery charge limit"""
        return BatteryThreshold(int.from_bytes(self._get_property("ChargeControlEndThreshold")))

    @charge_control_end_threshold.setter
    def charge_control_end_threshold(self, val: BatteryThreshold) -> None:
        self._set_property(
            "ChargeControlEndThreshold",
            QDBusVariant(QDBusArgument(val.value, QMetaType.Type.UChar)),
        )

    @property
    def platform_profile(self) -> PlatformProfile:
        """Thermal throttle policy"""
        return PlatformProfile(self._get_property("PlatformProfile"))

    @platform_profile.setter
    def platform_profile(self, val: bool) -> None:
        self._set_property(
            "PlatformProfile",
            QDBusVariant(QDBusArgument(val.value, QMetaType.Type.UInt)),
        )

    @property
    def change_platform_profile_on_battery(self) -> bool:
        """Change Platform Profile On Battery"""
        return PlatformProfile(self._get_property("ChangePlatformProfileOnBattery"))

    @change_platform_profile_on_battery.setter
    def change_platform_profile_on_battery(self, val: bool) -> None:
        self._set_property(
            "ChangePlatformProfileOnBattery",
            QDBusVariant(QDBusArgument(val, QMetaType.Type.Bool)),
        )

    @property
    def change_platform_profile_on_ac(self) -> bool:
        """Change Platform Profile On AC"""
        return PlatformProfile(self._get_property("ChangePlatformProfileOnAc"))

    @change_platform_profile_on_ac.setter
    def change_platform_profile_on_ac(self, val: bool) -> None:
        self._set_property(
            "ChangePlatformProfileOnAc",
            QDBusVariant(QDBusArgument(val, QMetaType.Type.Bool)),
        )


platform_client = PlatformClient()
