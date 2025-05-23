from rcc.communications.client.cmd.abstract_cmd_client import AbstractCmdClient
from rcc.models.cpu_governor import CpuGovernor


class CpuPowerClient(AbstractCmdClient):
    """Client for cpupower cmd"""

    def __init__(self):
        super().__init__("cpupower")

    def set_governor(self, governor: CpuGovernor):
        """Set CPU governor"""
        return self._run_command(f"frequency-set -g {governor.value}", True)


CPU_POWER_CLIENT = CpuPowerClient()
