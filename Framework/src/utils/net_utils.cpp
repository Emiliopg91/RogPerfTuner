#include "framework/utils/net_utils.hpp"

#include <netinet/in.h>
#include <unistd.h>

#include <regex>
#include <stdexcept>

#include "framework/logger/logger_provider.hpp"
#include "framework/utils/time_utils.hpp"
#include "httplib.h"

bool NetUtils::isPortFree(int port) {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		return false;
	}

	sockaddr_in addr{};
	addr.sin_family		 = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port		 = htons(port);

	int result = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
	close(sock);

	return result == 0;
}

int NetUtils::getRandomFreePort() {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		throw std::runtime_error("Couldn't get port");
	}

	sockaddr_in addr{};
	addr.sin_family		 = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port		 = 0;

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(sock);
		throw std::runtime_error("Couldn't bind socket");
	}

	// Obtener el puerto asignado
	socklen_t addrlen = sizeof(addr);
	if (getsockname(sock, (struct sockaddr*)&addr, &addrlen) < 0) {
		close(sock);
		throw std::runtime_error("Couldn't get attached port");
	}

	int port = ntohs(addr.sin_port);
	close(sock);
	return port;
}

std::tuple<std::string, std::string> split_url(const std::string& url) {
	static const std::regex re(R"(^(?:https?:\/\/)?([^\/:]+)(\/.*)?$)");
	std::smatch match;
	if (std::regex_match(url, match, re)) {
		std::string host = match.size() > 1 ? match[1].str() : "";
		std::string path = match.size() > 2 && match[2].matched ? match[2].str() : "/";
		return {host, path};
	}
	return {"", ""};
}

void NetUtils::download(const std::string url, const std::string dst) {
	auto logger = LoggerProvider::getLogger("NetUtils");

	logger->info("Downloading {} into {}", url, dst);
	Logger::add_tab();

	auto t0			  = TimeUtils::now();
	auto [host, path] = split_url(url);
	if (host.empty()) {
		Logger::rem_tab();
		throw std::runtime_error("Invalid URL: " + url);
	}

	httplib::SSLClient client(host);

	client.set_follow_location(true);

	auto res = client.Get(path.c_str());
	if (!res || res->status != 200) {
		Logger::rem_tab();
		throw std::runtime_error("HTTP download failed");
	}

	std::ofstream ofs(dst, std::ios::binary);
	if (!ofs) {
		Logger::rem_tab();
		throw std::runtime_error("Failed to open destination file");
	}

	ofs << res->body;
	logger->info("Download completed after {} ms", TimeUtils::getTimeDiff(t0, TimeUtils::now()));

	Logger::rem_tab();
}
std::string NetUtils::fetch(const std::string url) {
	auto logger = LoggerProvider::getLogger("NetUtils");

	logger->info("Fetching {}", url);
	Logger::add_tab();

	auto t0			  = TimeUtils::now();
	auto [host, path] = split_url(url);
	if (host.empty()) {
		Logger::rem_tab();
		throw std::runtime_error("Invalid URL: " + url);
	}

	httplib::SSLClient client(host);

	client.set_follow_location(true);

	auto res = client.Get(path.c_str());

	logger->info("Fetch completed after {} ms with status {}", TimeUtils::getTimeDiff(t0, TimeUtils::now()), res->status);
	if (!res || res->status != 200) {
		Logger::rem_tab();
		throw std::runtime_error("HTTP fetch failed");
	}

	Logger::rem_tab();
	return res->body;
}
