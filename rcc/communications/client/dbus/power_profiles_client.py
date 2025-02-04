from rcc.communications.client.dbus.base.abstract_dbus_client import AbstractDbusClient
from rcc.models.power_profile import PowerProfile
from framework.singleton import singleton


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

    @property
    def active_profile(self) -> PowerProfile:
        """Active power profile"""
        return PowerProfile(self._get_property("ActiveProfile"))

    @active_profile.setter
    def active_profile(self, val: PowerProfile) -> None:
        self._set_property("ActiveProfile", val.value)


POWER_PROFILE_CLIENT = PowerProfilesClient()
