from rcc.communications.client.file.abstract_file_client import AbstractFileClient


class CpuInfoClient(AbstractFileClient):
    """CpuInfo client"""

    def __init__(self):
        super().__init__("/proc/cpuinfo")


CPU_INFO_CLIENT = CpuInfoClient()
