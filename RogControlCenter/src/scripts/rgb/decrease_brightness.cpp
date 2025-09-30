#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

#include "../../../include/models/others/communication_message.hpp"
#include "../../../include/utils/constants.hpp"
#include "../../../include/utils/string_utils.hpp"

int main() {
	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		return 1;
	}

	sockaddr_un addr{};
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, Constants::SOCKET_FILE.c_str(), sizeof(addr.sun_path) - 1);

	if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("connect");
		close(sock);
		return 1;
	}

	CommunicationMessage msg;
	msg.id	 = StringUtils::generateUUIDv4();
	msg.name = Constants::DEC_BRIGHT;

	std::string message = msg.to_json();
	if (write(sock, message.c_str(), message.size()) < 0) {
		perror("write");
		close(sock);
		return 1;
	}

	char buffer[1024];
	ssize_t bytes = read(sock, buffer, sizeof(buffer) - 1);
	if (bytes > 0) {
		buffer[bytes] = '\0';
		std::cout << buffer << "\n";
	}

	close(sock);
	return 0;
}