#include "../../../include/clients/shell/scxctl_client.hpp"

#include <optional>

ScxCtlClient::ScxCtlClient() : AbstractCmdClient("scxctl", "ScxCtlClient") {
	std::array<std::array<std::string, 2>, 8> all = {{{"bpfland", "-m performance"},
													  {"cosmos", "-c 0 -p 0"},
													  {"flash", "-m all"},
													  {"lavd", "--performance"},
													  {"p2dq", "--task-slice true -f --sched-mode performance"},
													  {"rustland", "rustland"},
													  {"rusty", ""},
													  {"tickless", "-f 5000 -s 5000"}}};
	logger.info("Initializing ScxCtlClient");
	Logger::add_tab();

	logger.info("Available schedulers:");
	Logger::add_tab();
	for (auto entry : all) {
		if (shell.which("scx_" + entry[0]).has_value()) {
			available_sched[entry[0]] = entry[1];
			logger.info("{}", entry[0]);
		}
	}

	Logger::rem_tab();
	Logger::rem_tab();
}

void ScxCtlClient::start(std::string name) {
	auto it = available_sched.find(name);
	if (it == available_sched.end()) {
		logger.error("Scheduler {} not available", name);
		return;
	}

	auto action = "start";

	if (current.has_value()) {
		if (name == current) {
			return;
		}

		action = "switch";
	}

	run_command(fmt::format("{} --sched {} --args=\"{}\"", action, name, it->second));
	current = name;
}

void ScxCtlClient::stop() {
	if (!current.has_value()) {
		return;
	}

	run_command("stop");
	current = std::nullopt;
}

std::vector<std::string> ScxCtlClient::getAvailable() {
	std::vector<std::string> res;
	for (const auto& [key, val] : available_sched) {
		res.emplace_back(key);
	}
	return res;
}

std::optional<std::string> ScxCtlClient::getCurrent() {
	return current;
}