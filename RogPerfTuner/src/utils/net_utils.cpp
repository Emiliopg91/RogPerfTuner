#include "../../include/utils/net_utils.hpp"

#include <curl/curl.h>
#include <netinet/in.h>
#include <unistd.h>

#include <fstream>
#include <regex>
#include <stdexcept>

#include "../../include/utils/logger/logger.hpp"

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
	Logger logger = *LoggerProvider::getLogger("NetUtils");

	logger.info("Downloading " + url + " into " + dst);
	Logger::add_tab();

	CurlClient::getInstance().download(url, dst);

	Logger::rem_tab();
}

std::string NetUtils::fetch(const std::string url) {
	Logger logger = *LoggerProvider::getLogger("NetUtils");

	logger.info("Fetching " + url);
	Logger::add_tab();

	auto response = CurlClient::getInstance().fetch(url);

	Logger::rem_tab();
	return response;
}
