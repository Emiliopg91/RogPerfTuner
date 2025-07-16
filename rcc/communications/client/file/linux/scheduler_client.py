from rcc.communications.client.file.abstract_file_client import AbstractFileClient
from rcc.models.ssd_scheduler import SsdScheduler


class SsdSchedulerClient(AbstractFileClient):
    """SSD scheduler client"""

    def __init__(self):
        super().__init__("/sys/block/nvme*/queue/scheduler")

    def set_scheduler(self, scheduler: SsdScheduler):
        """Set scheduler"""
        self.write(scheduler.value, True)

    def get_schedulers(self) -> list[SsdScheduler]:
        """Get common schedulers"""
        result = []

        output = self.read()
        for sched in SsdScheduler:
            if all(sched.value in l for l in output.splitlines()):
                result.append(sched)

        return result


SSD_SCHEDULER_CLIENT = SsdSchedulerClient()
