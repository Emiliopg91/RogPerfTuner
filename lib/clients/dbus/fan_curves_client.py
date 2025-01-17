# pylint: disable=E0611, E0401
from PyQt6.QtCore import QMetaType
from PyQt6.QtDBus import QDBusArgument

from lib.clients.dbus.base.abstract_dbus_client import AbstractDbusClient
from lib.models.thermal_throttle_profile import ThermalThrottleProfile
from lib.utils.singleton import singleton


@singleton
class FanCurvesClient(AbstractDbusClient):
    """Fan curves client"""

    def __init__(self):
        super().__init__(True, "org.asuslinux.Daemon", "/org/asuslinux", "org.asuslinux.FanCurves")

    def reset_profile_curves(self, profile: ThermalThrottleProfile):
        """Reset fan curves"""
        self._invoke_method("ResetProfileCurves", QDBusArgument(profile.value, QMetaType.Type.UInt.value))

    def set_curves_to_defaults(self, profile: ThermalThrottleProfile):
        """Restore cuves to default"""
        self._invoke_method("SetCurvesToDefaults", QDBusArgument(profile.value, QMetaType.Type.UInt.value))

    def set_fan_curves_enabled(self, profile: ThermalThrottleProfile, enabled: bool):
        """Enable fan curves"""
        self._invoke_method("SetFanCurvesEnabled", QDBusArgument(profile.value, QMetaType.Type.UInt.value), enabled)


fan_curves_client = FanCurvesClient()
