#include "../../../include/servers/http/http_server.hpp"

#include <nlohmann/json.hpp>

#include "../../../include/utils/process_utils.hpp"
#include "httplib.h"

using json = nlohmann::json;

HttpServer::HttpServer() {
	logger.info("Initializing HTTP server");
	Logger::add_tab();

	svr.set_idle_interval(std::chrono::milliseconds(100));

	svr.Get("/ping", [this](const httplib::Request&, httplib::Response&) {
	});

	svr.Get(Constants::URL_PERF_PROF, [this](const httplib::Request&, httplib::Response& res) {
		json response;
		response["profile"] = profileService.nextPerformanceProfile().toName();

		res.set_content(response.dump(4), "application/json");
	});

	svr.Get(Constants::URL_INC_BRIGHT, [this](const httplib::Request&, httplib::Response& res) {
		json response;
		response["brightness"] = openRgbService.increaseBrightness().toName();

		res.set_content(response.dump(4), "application/json");
	});

	svr.Get(Constants::URL_DEC_BRIGHT, [this](const httplib::Request&, httplib::Response& res) {
		json response;
		response["brightness"] = openRgbService.decreaseBrightness().toName();

		res.set_content(response.dump(4), "application/json");
	});

	svr.Get(Constants::URL_NEXT_EFF, [this](const httplib::Request&, httplib::Response& res) {
		json response;
		response["effect"] = openRgbService.nextEffect();

		res.set_content(response.dump(4), "application/json");
	});

	svr.Get(Constants::URL_RENICE, [this](const httplib::Request& req, httplib::Response&) {
		auto pidStr = req.get_param_value("pid");
		hardwareService.renice(static_cast<pid_t>(std::stoi(pidStr)));
	});

	svr.Get(Constants::URL_GAME_CFG, [this](const httplib::Request& req, httplib::Response& res) {
		auto gidStr = req.get_param_value("appid");
		json j		= steamService.getConfiguration(gidStr);
		res.set_content(j.dump(), "application/json");
	});

	if (!svr.bind_to_port("127.0.0.1", Constants::HTTP_PORT)) {
		throw std::runtime_error("Cannot bind to port");
	}

	runner = std::thread([this] {
		started = true;
		logger.info("Server listening on 127.0.0.1:{}", Constants::HTTP_PORT);
		svr.listen_after_bind();  // <- entra al bucle
		logger.info("Server loop exited");
	});
	while (!started) {
		ProcessUtils::sleep(10);
	}

	eventBus.onApplicationStop([this]() {
		stop();
	});

	Logger::rem_tab();
}

HttpServer::~HttpServer() {
	stop();
}

void HttpServer::stop() {
	try {
		static std::atomic<bool> stopping{false};
		bool expected = false;
		if (!stopping.compare_exchange_strong(expected, true)) {
			return;
		}

		logger.info("Stopping HTTP server");
		Logger::add_tab();
		svr.stop();

		if (runner.joinable()) {
			runner.join();
		}
		Logger::rem_tab();
		logger.info("HTTP server stopped");
	} catch (...) {
	}
}