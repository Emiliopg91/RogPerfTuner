from framework.singleton import singleton
from rcc.communications.client.dbus.abstract_dbus_client import AbstractDbusClient
from rcc.models.power_profile import PowerProfile
from rcc.utils.constants import APP_NAME


@singleton
class PowerProfilesClient(AbstractDbusClient):
    """DBus power profile client"""

    def __init__(self):
        super().__init__(
            True,
            "net.hadess.PowerProfiles",
            "/net/hadess/PowerProfiles",
            "net.hadess.PowerProfiles",
        )
        self.__cookie = None

    @property
    def active_profile(self) -> PowerProfile:
        """Active power profile"""
        return PowerProfile(self._get_property("ActiveProfile"))

    @active_profile.setter
    def active_profile(self, val: PowerProfile) -> None:
        self._set_property("ActiveProfile", val.value)

    def hold_profile(self, profile: str):
        """Request profile"""
        if self.__cookie is not None:
            self._invoke_method("ReleaseProfile", self.__cookie)
            self.__cookie = None
        self.__cookie = self._invoke_method("HoldProfile", profile, "Im one of those who dare", APP_NAME)


POWER_PROFILE_CLIENT = PowerProfilesClient()
