#ifdef AUR_HELPER
#include "../../../include/clients/shell/aur_helper_client.hpp"

AurHelperClient::AurHelperClient() : AbstractCmdClient(AUR_HELPER, "AurHelperClient", false) {
	logger.info("Using AUR helper {}", AUR_HELPER);
}

std::string AurHelperClient::getVersion(std::string package) {
	return StringUtils::trim(run_command(fmt::format("--aur -Si {} | grep Version | cut -d':' -f 2 | xargs ", package)).stdout_str);
}

void AurHelperClient::install(std::string package) {
	shell.wait_for(
		shell.launch_in_terminal(fmt::format("{} --aur -S {}; read -p \"{}\"; exit", AUR_HELPER, package, translator.translate("press.enter.exit"))));
}
#endif