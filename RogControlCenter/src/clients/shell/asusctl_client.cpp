#include "../../../include/clients/shell/asusctl_client.hpp"

void AsusCtlClient::turnOffAura() {
	run_command("aura static -c 000000", true, false);
}