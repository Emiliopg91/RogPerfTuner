from PyQt5.QtCore import QMetaType
from PyQt5.QtDBus import QDBusVariant, QDBusArgument

from rcc.communications.client.dbus.asus.asus_base_client import AsusBaseClient
from rcc.models.battery_threshold import BatteryThreshold
from rcc.models.platform_profile import PlatformProfile
from framework.singleton import singleton


@singleton
class PlatformClient(AsusBaseClient):
    """DBus platform client"""

    def __init__(self):
        super().__init__("Platform")

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
    def platform_profile(self) -> PlatformProfile:
        """Thermal throttle policy"""
        return PlatformProfile(self._get_property("PlatformProfile"))

    @platform_profile.setter
    def platform_profile(self, val: bool) -> None:
        self._set_property(
            "PlatformProfile",
            QDBusVariant(QDBusArgument(val.value, QMetaType.UInt)),
        )

    @property
    def enable_ppt_group(self) -> PlatformProfile:
        """Enable PPT group"""
        return PlatformProfile(self._get_property("EnablePptGroup"))

    @enable_ppt_group.setter
    def enable_ppt_group(self, val: bool) -> None:
        self._set_property(
            "EnablePptGroup",
            QDBusVariant(QDBusArgument(val, QMetaType.Bool)),
        )

    @property
    def platfom_profile_linked_epp(self) -> PlatformProfile:
        """Enable EPP linking to profile"""
        return PlatformProfile(self._get_property("PlatformProfileLinkedEpp"))

    @platfom_profile_linked_epp.setter
    def platfom_profile_linked_epp(self, val: bool) -> None:
        self._set_property(
            "PlatformProfileLinkedEpp",
            QDBusVariant(QDBusArgument(val, QMetaType.Bool)),
        )

    @property
    def change_platform_profile_on_battery(self) -> bool:
        """Change Platform Profile On Battery"""
        return PlatformProfile(self._get_property("ChangePlatformProfileOnBattery"))

    @change_platform_profile_on_battery.setter
    def change_platform_profile_on_battery(self, val: bool) -> None:
        self._set_property(
            "ChangePlatformProfileOnBattery",
            QDBusVariant(QDBusArgument(val, QMetaType.Bool)),
        )

    @property
    def change_platform_profile_on_ac(self) -> bool:
        """Change Platform Profile On AC"""
        return PlatformProfile(self._get_property("ChangePlatformProfileOnAc"))

    @change_platform_profile_on_ac.setter
    def change_platform_profile_on_ac(self, val: bool) -> None:
        self._set_property(
            "ChangePlatformProfileOnAc",
            QDBusVariant(QDBusArgument(val, QMetaType.Bool)),
        )


PLATFORM_CLIENT = PlatformClient()
