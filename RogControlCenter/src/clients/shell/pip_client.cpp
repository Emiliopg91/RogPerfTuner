#include "../../../include/clients/shell/pip_client.hpp"

void PipClient::installPackage(std::string package, bool breakSystemPackages) {
	std::string bsp = breakSystemPackages ? "--break-system-packages " : "";
	run_command("install " + bsp + package);
}