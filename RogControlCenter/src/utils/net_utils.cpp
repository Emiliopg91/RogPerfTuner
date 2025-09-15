#include "../../include/utils/net_utils.hpp"

#include <netinet/in.h>
#include <unistd.h>

#include <stdexcept>

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