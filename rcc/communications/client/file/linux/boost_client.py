import os
from typing import Dict, List
from rcc.communications.client.file.abstract_file_client import AbstractFileClient


class BoostClient(AbstractFileClient):
    """Cpu boost status"""

    BOOST_CONTROLS: List[Dict[str, str]] = [
        {
            "path": "/sys/devices/system/cpu/intel_pstate/no_turbo",
            "on": "0",
            "off": "1",
        },
        {
            "path": "/sys/devices/system/cpu/cpufreq/boost",
            "on": "1",
            "off": "0",
        },
    ]

    def __init__(self):
        self._boost_control = None
        for control in self.BOOST_CONTROLS:
            if os.path.exists(control["path"]):
                self._boost_control = control
                break

        if self._boost_control:
            super().__init__(self._boost_control["path"])

    @property
    def available(self):
        return super().available if self._boost_control else False

    @property
    def boost(self):
        """Boost flag"""
        if self.available:
            return self.read() == self._boost_control["on"]
        return False

    @boost.setter
    def boost(self, boost):
        self.write(self._boost_control["on"] if boost else self._boost_control["off"], True)


BOOST_CLIENT = BoostClient()
