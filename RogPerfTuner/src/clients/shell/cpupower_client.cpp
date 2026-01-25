#include "../../../include/clients/shell/cpupower_client.hpp"

void CpuPowerClient::setGovernor(const CpuGovernor& governor) {
	run_command("frequency-set -g " + CpuGovernorNS::toString(governor), true, true);
}
