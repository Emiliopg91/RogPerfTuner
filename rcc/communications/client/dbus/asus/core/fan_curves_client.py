from PyQt5.QtCore import QMetaType
from PyQt5.QtDBus import QDBusArgument

from rcc.communications.client.dbus.asus.asus_base_client import AsusBaseClient
from rcc.models.fan_curve import FanCurve
from rcc.models.platform_profile import PlatformProfile
from framework.singleton import singleton


@singleton
class FanCurvesClient(AsusBaseClient):
    """Fan curves client"""

    def __init__(self):
        super().__init__("FanCurves")

    def reset_profile_curves(self, profile: PlatformProfile):
        """Reset fan curves"""
        self._invoke_method("ResetProfileCurves", QDBusArgument(profile.value, QMetaType.UInt))

    def set_curves_to_defaults(self, profile: PlatformProfile):
        """Restore cuves to default"""
        self._invoke_method("SetCurvesToDefaults", QDBusArgument(profile.value, QMetaType.UInt))

    def set_fan_curves_enabled(self, profile: PlatformProfile, enabled: bool):
        """Enable fan curves"""
        self._invoke_method("SetFanCurvesEnabled", QDBusArgument(profile.value, QMetaType.UInt), enabled)

    def fan_curve_data(self, profile: PlatformProfile) -> list[FanCurve]:
        """Retrieve fan curve data"""
        fans = self._invoke_method("FanCurveData", QDBusArgument(profile.value, QMetaType.UInt))
        result: list[FanCurve] = []
        for fan in fans:
            result.append(FanCurve(fan))

        return result

    def set_fan_curve(self, profile: PlatformProfile, fan_curve: FanCurve):
        """Set fan curve data"""
        self._invoke_method(
            "SetFanCurve", QDBusArgument(profile.value, QMetaType.UInt), QDBusArgument(fan_curve.to_dbus())
        )

    @property
    def available(self):
        """Availability flag"""
        return False


FAN_CURVES_CLIENT = FanCurvesClient()
