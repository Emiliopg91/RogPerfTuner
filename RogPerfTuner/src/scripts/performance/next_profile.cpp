#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <any>
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
	msg.type = "REQUEST";
	msg.id	 = StringUtils::generateUUIDv4();
	msg.name = Constants::PERF_PROF;

	std::string message = msg.to_json();
	uint32_t resp_len	= htonl(message.size());

	if (write(sock, &resp_len, sizeof(resp_len)) < 0 || write(sock, message.c_str(), message.size()) < 0) {
		perror("write");
		close(sock);
		return 1;
	}

	ssize_t n = read(sock, &resp_len, sizeof(resp_len));
	if (n == sizeof(resp_len)) {
		resp_len = ntohl(resp_len);
		std::string data(resp_len, '\0');
		size_t total_read = 0;
		while (total_read < resp_len) {
			ssize_t r = read(sock, &data[total_read], resp_len - total_read);
			if (r <= 0) {
				perror("read");
				close(sock);
				return 1;
			}
			total_read += r;
		}

		auto json = nlohmann::json::parse(data);
		auto j	  = CommunicationMessage::from_json(json);
		std::cout << std::any_cast<std::string>(j.data[0]) << std::endl;

		return 0;
	} else {
		perror("read");
		close(sock);
		return 1;
	}
}