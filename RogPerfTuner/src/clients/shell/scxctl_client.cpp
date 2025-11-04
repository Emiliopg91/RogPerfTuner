#include "../../../include/clients/shell/scxctl_client.hpp"

#include <optional>

#include "../../../include/utils/string_utils.hpp"

ScxCtlClient::ScxCtlClient() : AbstractCmdClient("scxctl", "ScxCtlClient") {
	if (available()) {
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
		auto output = run_command("list").stdout_str;
		std::vector<std::string> schedulers;
		for (auto entry : all) {
			if (StringUtils::isSubstring("\"" + entry[0] + "\"", output)) {
				available_sched[entry[0]] = entry[1];
				schedulers.emplace_back(entry[0]);
			}
		}
		logger.info(StringUtils::join(schedulers, ", "));
		Logger::rem_tab();

		logger.info("Getting current scheduler");
		Logger::add_tab();
		auto currentStr = run_command("get").stdout_str;
		size_t pos		= currentStr.find(' ');
		currentStr		= currentStr.substr(pos + 1);
		pos				= currentStr.find(' ');
		current			= StringUtils::toLowerCase(currentStr.substr(0, pos));

		if (current == "scx") {
			current = std::nullopt;
			logger.info("No active scheduler");
		} else {
			logger.info("Currently using {} scheduler", current.value());
		}
		Logger::rem_tab();

		Logger::rem_tab();
	}
}

void ScxCtlClient::start(std::string name) {
	auto it = available_sched.find(name);
	if (it == available_sched.end()) {
		logger.error("Scheduler {} not available", name);
		return;
	}

	if (name == current.value_or("")) {
		logger.info("Scheduler already applied");
		return;
	}

	auto action = "start";

	if (current.has_value()) {
		if (name == current) {
			return;
		}

		action = "switch";
		logger.info("Switching scheduler from {} to {}", current.value(), name);
	} else {
		logger.info("Starting scheduler {}", name);
	}

	Logger::add_tab();
	run_command(fmt::format("{} --sched {} --args=\"{}\"", action, name, it->second));
	Logger::rem_tab();

	logger.info("Scheduler applied succesfully");

	current = name;
}

void ScxCtlClient::stop() {
	if (!current.has_value()) {
		return;
	}
	logger.info("Stopping scheduler {}", current.value());

	Logger::add_tab();
	run_command("stop");
	Logger::rem_tab();
	current = std::nullopt;

	logger.info("Scheduler stopped succesfully");
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