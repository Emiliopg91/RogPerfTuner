#ifdef AUR_HELPER
#include "../../../include/clients/shell/aur_helper_client.hpp"

#include "../../../include/utils/constants.hpp"
#include "../../../include/utils/time_utils.hpp"

AurHelperClient::AurHelperClient() : AbstractCmdClient(AUR_HELPER, "AurHelperClient", false) {
	logger.info("Using AUR helper {}", AUR_HELPER);
}

std::string AurHelperClient::getVersion(std::string package) {
	return StringUtils::trim(run_command(fmt::format("--aur -Si {} | grep Version | cut -d':' -f 2 | xargs ", package)).stdout_str);
}

void AurHelperClient::install(std::string package) {
	shell.wait_for(shell.launch_in_terminal(fmt::format("{} -S {}; read -p \"{}\"; systemd-run --user --unit={}-{} {}", AUR_HELPER, package,
														translator.translate("press.enter.exit"), Constants::EXEC_NAME,
														TimeUtils::now().time_since_epoch().count(), Constants::EXEC_NAME)));
}
#endif