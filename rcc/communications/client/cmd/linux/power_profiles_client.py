from framework.singleton import singleton
from rcc.communications.client.cmd.abstract_cmd_client import AbstractCmdClient
from rcc.models.power_profile import PowerProfile


@singleton
class PowerProfilesClient(AbstractCmdClient):
    """DBus power profile client"""

    def __init__(self):
        super().__init__("gdbus")

    @property
    def active_profile(self) -> PowerProfile:
        """Active power profile"""
        # return PowerProfile(self._get_property("ActiveProfile"))

    @active_profile.setter
    def active_profile(self, val: PowerProfile) -> None:
        self._run_command(
            f"call --system --dest net.hadess.PowerProfiles --object-path /net/hadess/PowerProfiles --method org.freedesktop.DBus.Properties.Set net.hadess.PowerProfiles ActiveProfile \"<'{val.value}'>\""  # pylint: disable=line-too-long
        )


POWER_PROFILE_CLIENT = PowerProfilesClient()
