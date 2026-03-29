#include "clients/file/scaling_governor_client.hpp"

#include "framework/clients/abstract/abstract_glob_client.hpp"
#include "framework/utils/enum_utils.hpp"

ScalingGovernorClient::ScalingGovernorClient() : AbstractGlobClient(SCALING_GOVERNOR_FILE, "ScalingGovernorClient", true, false) {
}

void ScalingGovernorClient::setGovernor(CpuGovernor governor) {
	write(toString(governor));
}
