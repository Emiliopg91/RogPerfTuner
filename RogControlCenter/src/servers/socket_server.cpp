#include "../../include/servers/socket_server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <cstring>
#include <filesystem>

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

	logger.info("Socket server started on " + Constants::SOCKET_FILE);
	Logger::rem_tab();
}

SocketServer::~SocketServer() {
	stop();
}

void SocketServer::stop() {
	if (!started.load()) {
		return;
	}

	started.store(false);

	if (server_fd != -1) {
		close(server_fd);
		server_fd = -1;
	}

	if (!Constants::SOCKET_FILE.empty()) {
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
	char buffer[1024];
	ssize_t n;

	while ((n = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) {
		buffer[n] = '\0';
		std::string data(buffer);

		try {
			auto json_msg = nlohmann::json::parse(data);
			logger.debug("Received message: " + json_msg.dump());

			CommunicationMessage req = CommunicationMessage::from_json(json_msg);
			CommunicationMessage res = CommunicationMessage::from_json(json_msg);
			res.type				 = "RESPONSE";
			res.data				 = {};

			try {
				if (req.name == Constants::NEXT_EFF) {
					logger.info("Requested next effect");
					Logger::add_tab();
					res.data.emplace_back(openRgbService.nextEffect());
					Logger::rem_tab();
					logger.info("Request finished");
				} else if (req.name == Constants::INC_BRIGHT) {
					logger.info("Requested increase brightness");
					Logger::add_tab();
					res.data.emplace_back(openRgbService.increaseBrightness().toInt());
					Logger::rem_tab();
					logger.info("Request finished");
				} else if (req.name == Constants::DEC_BRIGHT) {
					logger.info("Requested decrease brightness");
					Logger::add_tab();
					res.data.emplace_back(openRgbService.decreaseBrightness().toInt());
					Logger::rem_tab();
					logger.info("Request finished");
				} else if (req.name == Constants::PERF_PROF) {
					logger.info("Requested next performance profile");
					Logger::add_tab();
					res.data.emplace_back(performanceService.nextPerformanceProfile().toName());
					Logger::rem_tab();
					logger.info("Request finished");
				} else if (req.name == Constants::GAME_CFG) {
					logger.info("Requested configuration for game {}", std::any_cast<std::string>(req.data[0]));
					Logger::add_tab();
					json j;
					to_json(j, steamService.getConfiguration(std::any_cast<std::string>(req.data[0])));
					res.data.emplace_back(j.dump());
					Logger::rem_tab();
					logger.info("Request finished");
				} else {
					res.error = "No such functionality";
				}
			} catch (std::exception& e) {
				logger.error("Error on request handling: {}", e.what());
				res.error = e.what();
			}

			std::string resp_str = res.to_json();
			logger.debug("Response message: " + resp_str);
			uint32_t len	 = resp_str.size();
			uint32_t net_len = htonl(len);	// host -> network byte order (big endian)

			write(client_fd, &net_len, sizeof(net_len));
			write(client_fd, resp_str.c_str(), resp_str.size());
		} catch (const std::exception& e) {
			logger.error("JSON parse error: " + std::string(e.what()));
		}
	}

	close(client_fd);
}
