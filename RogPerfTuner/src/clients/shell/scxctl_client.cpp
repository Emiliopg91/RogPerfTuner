#include "clients/shell/scxctl_client.hpp"

#include <optional>

#include "framework/utils/string_utils.hpp"

ScxCtlClient::ScxCtlClient() : AbstractCmdClient("scxctl", "ScxCtlClient") {
	if (available()) {
		std::array<std::array<std::string, 3>, 9> all = {{
			{"bpfland", "-m performance -w", ""},
			{"cosmos", "-c 0 -p 0", "-m powersave -d -p 5000"},
			{"flash", "-m all", "-m powersave -I 10000 -t 10000 -s 10000 -S 1000"},
			{"lavd", "--performance", "--powersave"},
			{"p2dq", "--task-slice true -f --sched-mode performance", "--sched-mode efficiency"},
		}};
		logger->info("Initializing ScxCtlClient");
		Logger::add_tab();

		logger->info("Available schedulers:");
		Logger::add_tab();
		auto output = run_command("list").stdout_str;
		std::vector<std::string> schedulers;
		for (auto [sched, perf, power] : all) {
			if (StringUtils::isSubstring("\"" + sched + "\"", output)) {
				available_sched[sched] = {perf, power};
				schedulers.emplace_back(sched);
			}
		}
		logger->info(StringUtils::join(schedulers, ", "));
		Logger::rem_tab();

		logger->info("Getting current scheduler");
		Logger::add_tab();
		auto currentStr = run_command("get").stdout_str;
		size_t pos		= currentStr.find(' ');
		currentStr		= currentStr.substr(pos + 1);
		pos				= currentStr.find(' ');
		current			= StringUtils::toLowerCase(currentStr.substr(0, pos));

		if (current == "scx") {
			current = std::nullopt;
			logger->info("No active scheduler");
		} else {
			logger->info("Currently using {} scheduler", current.value());
		}
		Logger::rem_tab();

		Logger::rem_tab();
	}
}

void ScxCtlClient::start(std::string name, bool powersave) {
	auto newPowersaveLiteral = powersave ? "powersave" : "performance";
	auto it					 = available_sched.find(name);
	if (it == available_sched.end()) {
		logger->error("Scheduler {} not available", name);
		return;
	}

	if (name == current.value_or("") && newPowersaveLiteral == current_powersave) {
		logger->info("Scheduler {}-{} already applied", name, current_powersave);
		return;
	}

	auto action = "start";

	if (current.has_value()) {
		action = "switch";
		logger->info("Switching scheduler from {}-{} to {}-{}", current.value(), current_powersave, name, newPowersaveLiteral);
	} else {
		logger->info("Starting scheduler {}-{}", name, newPowersaveLiteral);
	}

	Logger::add_tab();
	run_command(std::string(action) + " --sched " + name + " --args=\"" + it->second[powersave ? 1 : 0] + "\"", false, true);
	logger->info("Scheduler applied succesfully");
	Logger::rem_tab();

	current			  = name;
	current_powersave = newPowersaveLiteral;
}

void ScxCtlClient::stop() {
	if (!current.has_value()) {
		return;
	}
	logger->info("Stopping scheduler {}-{}", current.value(), current_powersave);

	Logger::add_tab();
	run_command("stop", false, true);

	logger->info("Scheduler stopped succesfully");
	Logger::rem_tab();
	current			  = std::nullopt;
	current_powersave = "performance";
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