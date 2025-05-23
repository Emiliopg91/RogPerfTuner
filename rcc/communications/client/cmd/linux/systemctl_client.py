from rcc.communications.client.cmd.abstract_cmd_client import AbstractCmdClient


class SystemCtlClient(AbstractCmdClient):
    """Systemctl client"""

    def __init__(self):
        super().__init__("systemctl")

    def restart_service(self, service: str):
        """Restart systemd service"""
        return self._run_command(f"restart {service}", True)


SYSTEM_CTL_CLIENT = SystemCtlClient()
