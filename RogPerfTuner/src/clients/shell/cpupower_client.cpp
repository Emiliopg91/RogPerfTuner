#include "clients/shell/cpupower_client.hpp"

#include "enum_utils.hpp"

void CpuPowerClient::setGovernor(const CpuGovernor& governor) {
	run_command("frequency-set -g " + toString(governor), true, true);
}
