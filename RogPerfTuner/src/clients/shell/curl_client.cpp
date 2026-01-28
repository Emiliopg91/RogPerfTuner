#include "clients/shell/curl_client.hpp"

#include <string>

CurlClient::CurlClient() : AbstractCmdClient("curl", "CurlClient") {
}

void CurlClient::download(std::string url, std::string dst) {
	Logger::add_tab();

	auto ret = run_command("-L -f -s -o \"" + dst + "\" \"" + url + "\"").exit_code;
	if (ret != 0) {
		logger->error("Failed to download {} (exit code {})", url, ret);
		Logger::rem_tab();
		throw std::runtime_error("Curl download failed: " + std::to_string(ret));
	}

	Logger::rem_tab();
}

std::string CurlClient::fetch(std::string url) {
	Logger::add_tab();

	auto res = run_command("-L -f -s \"" + url + "\"");
	if (res.exit_code != 0) {
		Logger::rem_tab();
		throw std::runtime_error("Failed to run curl command: " + std::to_string(res.exit_code));
	}

	Logger::rem_tab();
	return res.stdout_str;
}