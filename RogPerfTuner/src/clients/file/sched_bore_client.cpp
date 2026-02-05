#include "clients/file/sched_bore_client.hpp"

SchedBoreClient::SchedBoreClient() : AbstractFileClient("/proc/sys/kernel/sched_bore", "SchedBoreClient", true, false) {
}