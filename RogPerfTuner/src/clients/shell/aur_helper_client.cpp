#include <string>
#ifdef AUR_HELPER
#include "clients/shell/aur_helper_client.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/time_utils.hpp"

AurHelperClient::AurHelperClient() : AbstractCmdClient(AUR_HELPER, "AurHelperClient", false) {
	logger->info("Using AUR helper " + std::string(AUR_HELPER));
}

std::string AurHelperClient::getVersion(std::string package) {
	return StringUtils::trim(run_command("--aur -Si " + package + " | grep Version | cut -d':' -f 2 | xargs ").stdout_str);
}

void AurHelperClient::install(std::string package) {
	shell.wait_for(shell.launch_in_terminal(std::string(AUR_HELPER) + " -S " + package + "; read -p \"" + translator.translate("press.enter.exit") +
											"\"; systemd-run --user --unit=" + Constants::EXEC_NAME + "-" +
											std::to_string(TimeUtils::now().time_since_epoch().count()) + " " + Constants::EXEC_NAME));
}
#endif