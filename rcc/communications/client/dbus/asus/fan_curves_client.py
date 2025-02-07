from PyQt5.QtCore import QMetaType
from PyQt5.QtDBus import QDBusArgument

from rcc.communications.client.dbus.abstract_dbus_client import AbstractDbusClient
from rcc.models.platform_profile import PlatformProfile
from framework.singleton import singleton


@singleton
class FanCurvesClient(AbstractDbusClient):
    """Fan curves client"""

    def __init__(self):
        super().__init__(True, "xyz.ljones.Asusd", "/xyz/ljones", "xyz.ljones.FanCurves")

    def reset_profile_curves(self, profile: PlatformProfile):
        """Reset fan curves"""
        self._invoke_method("ResetProfileCurves", QDBusArgument(profile.value, QMetaType.UInt))

    def set_curves_to_defaults(self, profile: PlatformProfile):
        """Restore cuves to default"""
        self._invoke_method("SetCurvesToDefaults", QDBusArgument(profile.value, QMetaType.UInt))

    def set_fan_curves_enabled(self, profile: PlatformProfile, enabled: bool):
        """Enable fan curves"""
        self._invoke_method("SetFanCurvesEnabled", QDBusArgument(profile.value, QMetaType.UInt), enabled)


FAN_CURVES_CLIENT = FanCurvesClient()
