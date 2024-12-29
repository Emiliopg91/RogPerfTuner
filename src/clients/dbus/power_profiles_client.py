from .base.abstract_dbus_client import AbstractDbusClient
from ...models.power_profile import PowerProfile
from ...utils.singleton import singleton


@singleton
class PowerProfilesClient(AbstractDbusClient):
    def __init__(self):
        super().__init__(
            True,
            "net.hadess.PowerProfiles",
            "/net/hadess/PowerProfiles",
            "net.hadess.PowerProfiles",
        )

    @property
    def activeProfile(self) -> PowerProfile:
        return PowerProfile(self.getProperty("ActiveProfile"))

    @activeProfile.setter
    def activeProfile(self, val: PowerProfile) -> None:
        self.setProperty("ActiveProfile", val.value)


power_profile_client = PowerProfilesClient()
