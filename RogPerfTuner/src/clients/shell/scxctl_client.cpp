#include "clients/shell/scxctl_client.hpp"

#include <optional>

#include "framework/utils/string_utils.hpp"

ScxCtlClient::ScxCtlClient() : AbstractCmdClient("scxctl", "ScxCtlClient") {
	if (available()) {
		std::array<std::array<std::string, 3>, 9> all = {{
			{"bpfland", "", "-s 20000 -m powersave -I 100 -t 100"},
			{"cosmos", "-c 0 -p 0", "-m powersave -d -p 5000"},
			{"flash", "-m all", "-m powersave -I 10000 -t 10000 -s 10000 -S 1000"},
			{"lavd", "--performance", "--powersave"},
			{"p2dq", "--task-slice true -f --sched-mode performance", "--sched-mode efficiency"},
		}};

		auto output = run_command("list").stdout_str;
		std::vector<std::string> schedulers;
		for (auto [sched, perf, power] : all) {
			if (StringUtils::isSubstring("\"" + sched + "\"", output)) {
				available_sched[sched] = {perf, power};
				schedulers.emplace_back(sched);
			}
		}

		auto currentStr = run_command("get").stdout_str;
		size_t pos		= currentStr.find(' ');
		currentStr		= currentStr.substr(pos + 1);
		pos				= currentStr.find(' ');
		current			= StringUtils::toLowerCase(currentStr.substr(0, pos));

		if (current == "scx") {
			current = std::nullopt;
		}
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
		logger->debug("Scheduler {}-{} already applied", name, current_powersave);
		return;
	}

	auto action = "start";

	if (current.has_value()) {
		action = "switch";
		logger->debug("Switching scheduler from {}-{} to {}-{}", current.value(), current_powersave, name, newPowersaveLiteral);
	} else {
		logger->debug("Starting scheduler {}-{}", name, newPowersaveLiteral);
	}

	Logger::add_tab();
	run_command(std::string(action) + " --sched " + name + " --args=\"" + it->second[powersave ? 1 : 0] + "\"", false, true);
	logger->debug("Scheduler applied succesfully");
	Logger::rem_tab();

	current			  = name;
	current_powersave = newPowersaveLiteral;
}

void ScxCtlClient::stop() {
	if (!current.has_value()) {
		return;
	}
	logger->debug("Stopping scheduler {}-{}", current.value(), current_powersave);

	Logger::add_tab();
	run_command("stop", false, true);

	logger->debug("Scheduler stopped succesfully");
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
