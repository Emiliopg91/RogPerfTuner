#include "../../include/servers/socket_server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <cstring>
#include <filesystem>
#include <string>

#include "../../include/utils/file_utils.hpp"

SocketServer::SocketServer() : Loggable("SocketServer") {
	logger.info("Initializing socket server");
	Logger::add_tab();

	auto modTime = std::filesystem::file_time_type::min();
	if (FileUtils::exists(Constants::SOCKET_FILE)) {
		modTime = FileUtils::getMTime(Constants::SOCKET_FILE);
	}

	started.store(true);
	runner = std::thread(&SocketServer::run, this);

	while (true) {
		try {
			if (FileUtils::exists(Constants::SOCKET_FILE) && FileUtils::getMTime(Constants::SOCKET_FILE) != modTime) {
				break;
			}
		} catch (std::exception& e) {
		}
	}

	eventBus.onApplicationStop([this] {
		stop();
	});

	logger.info("Socket server started on " + Constants::SOCKET_FILE);
	Logger::rem_tab();
}

SocketServer::~SocketServer() {
	stop();
}

void SocketServer::stop() {
	logger.info("Stopping socket server");

	if (!started.load()) {
		return;
	}

	started.store(false);

	if (server_fd != -1) {
		shutdown(server_fd, SHUT_RDWR);
		close(server_fd);
		server_fd = -1;
		unlink(Constants::SOCKET_FILE.c_str());
	}

	if (runner.joinable()) {
		runner.join();
	}

	logger.info("Socket server stopped");
}

void SocketServer::run() {
	server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_fd < 0) {
		logger.error("Failed to create socket");
		return;
	}

	sockaddr_un addr{};
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, Constants::SOCKET_FILE.c_str(), sizeof(addr.sun_path) - 1);

	unlink(Constants::SOCKET_FILE.c_str());

	if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		logger.error("Failed to bind socket: " + std::string(strerror(errno)));
		close(server_fd);
		return;
	}

	if (listen(server_fd, 5) < 0) {
		logger.error("Failed to listen on socket");
		close(server_fd);
		return;
	}

	while (started.load()) {
		int client_fd = accept(server_fd, nullptr, nullptr);
		if (client_fd < 0) {
			if (started.load()) {
				logger.error("Accept failed: " + std::string(strerror(errno)));
			}
			continue;
		}

		std::thread(&SocketServer::handleClient, this, client_fd).detach();
	}
}
void SocketServer::handleClient(int client_fd) {
	while (true) {
		uint32_t msg_len_net;
		ssize_t n = read(client_fd, &msg_len_net, sizeof(msg_len_net));
		if (n <= 0) {
			break;
		}

		if (n != sizeof(msg_len_net)) {
			logger.error("Incomplete header received");
			break;
		}

		uint32_t msg_len = ntohl(msg_len_net);
		if (msg_len == 0 || msg_len > 10 * 1024 * 1024) {
			logger.error("Invalid message length: " + std::to_string(msg_len));
			break;
		}

		std::string data(msg_len, '\0');
		size_t total_read = 0;
		while (total_read < msg_len) {
			ssize_t r = read(client_fd, &data[total_read], msg_len - total_read);
			if (r <= 0) {
				logger.error("Error reading message body");
				close(client_fd);
				return;
			}
			total_read += r;
		}

		Logger::add_tab();
		try {
			YAML::Node node = YAML::Load(data);

			CommunicationMessage req = node.as<CommunicationMessage>();

			if (req.type == "REQUEST") {
				handleRequest(client_fd, req);
			}
		} catch (const std::exception& e) {
			logger.error("YAML parse error: " + std::string(e.what()));
		}
		Logger::rem_tab();
	}

	close(client_fd);
}

void SocketServer::handleEvent(const CommunicationMessage& req) {
	eventBus.emitServerSocketEvent(req.name, req.data);
}

void SocketServer::handleRequest(const int& clientFd, const CommunicationMessage& req) {
	CommunicationMessage res = CommunicationMessage(req);
	res.type				 = "RESPONSE";
	res.data				 = {};

	try {
		if (req.name == Constants::NEXT_EFF) {
			res.data.emplace_back(openRgbService.nextEffect());
		} else if (req.name == Constants::INC_BRIGHT) {
			res.data.emplace_back(openRgbService.increaseBrightness().toName());
		} else if (req.name == Constants::DEC_BRIGHT) {
			res.data.emplace_back(openRgbService.decreaseBrightness().toName());
		} else if (req.name == Constants::PERF_PROF) {
			res.data.emplace_back(performanceService.nextPerformanceProfile().toName());
		} else if (req.name == Constants::GAME_CFG) {
			std::string idStr;
			try {
				idStr = std::to_string(std::any_cast<long long>(req.data[0]));
			} catch (std::exception& e) {
				idStr = std::to_string(std::any_cast<uint64_t>(req.data[0]));
			}
			YAML::Node node = YAML::convert<SteamGameConfig>::encode(steamService.getConfiguration(idStr));

			std::stringstream ss;
			ss << node;
			res.data.emplace_back(ss.str());
		} else {
			res.error = "No such method";
		}
	} catch (std::exception& e) {
		logger.error("Error on request handling: " + std::string(e.what()));
		res.error = e.what();
	}

	YAML::Node node = YAML::convert<CommunicationMessage>::encode(res);
	std::stringstream ss;
	ss << node;
	std::string resp_str = ss.str();

	uint32_t resp_len = htonl(resp_str.size());
	write(clientFd, &resp_len, sizeof(resp_len));
	write(clientFd, resp_str.c_str(), resp_str.size());
}