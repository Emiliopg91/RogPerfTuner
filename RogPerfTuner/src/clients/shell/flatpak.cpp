#include "../../../include/clients/shell/flatpak.hpp"

bool FlatpakClient::checkInstalled(const std::string& name, bool userland) {
	std::string args = "info ";
	if (userland) {
		args += "--user ";
	} else {
		args += "--system ";
	}
	args += name;
	return run_command(args, false).exit_code == 0;
}

bool FlatpakClient::install(const std::string& name, bool userland) {
	std::string args = "install -y ";
	if (userland) {
		args += "--user ";
	} else {
		args += "--system ";
	}
	args += name;

	return run_command(args, false).exit_code == 0;
}

bool FlatpakClient::override(const std::string& name, bool userland) {
	std::string args = "override ";
	if (userland) {
		args += "--user ";
	} else {
		args += "--system ";
	}
	args += name;

	return run_command(args, false).exit_code == 0;
}