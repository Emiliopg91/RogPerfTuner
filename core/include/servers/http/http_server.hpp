#pragma once

#include "httplib.h"
#include "RccCommons.hpp"
#include "../../services/open_rgb_service.hpp"
#include "../../services/profile_service.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;
class HttpServer
{
private:
    httplib::Server svr;
    std::thread runner;
    std::atomic<bool> started{false};
    Logger logger{"HttpServer"};

    HttpServer()
    {
        logger.info("Initializing HTTP server");

        svr.set_idle_interval(std::chrono::milliseconds(100));

        svr.Get("/ping", [this](const httplib::Request &req, httplib::Response &res) {});

        svr.Get(Constants::URL_PERF_PROF, [this](const httplib::Request &req, httplib::Response &res)
                {
            json response;
            response["profile"] = ProfileService::getInstance().nextPerformanceProfile().toName();

            res.set_content(response.dump(4), "application/json"); });

        svr.Get(Constants::URL_INC_BRIGHT, [this](const httplib::Request &req, httplib::Response &res)
                {
            json response;
            response["brightness"] = OpenRgbService::getInstance().increaseBrightness().toName();

            res.set_content(response.dump(4), "application/json"); });

        svr.Get(Constants::URL_DEC_BRIGHT, [this](const httplib::Request &req, httplib::Response &res)
                {
            json response;
            response["brightness"] = OpenRgbService::getInstance().decreaseBrightness().toName();

            res.set_content(response.dump(4), "application/json"); });

        svr.Get(Constants::URL_NEXT_EFF, [this](const httplib::Request &req, httplib::Response &res)
                {
            json response;
            response["effect"] = OpenRgbService::getInstance().nextEffect();

            res.set_content(response.dump(4), "application/json"); });

        if (!svr.bind_to_port("127.0.0.1", Constants::HTTP_PORT))
        {
            throw std::runtime_error("Cannot bind to port");
        }

        runner = std::thread([this]
                             {
            started = true;
            logger.info("Server listening on 127.0.0.1:" + std::to_string(Constants::HTTP_PORT));
            svr.listen_after_bind();            // <- entra al bucle
            logger.info("Server loop exited"); });
    }

public:
    static HttpServer &getInstance()
    {
        static HttpServer instance;
        return instance;
    }

    ~HttpServer()
    {
        stop();
    }

    void stop()
    {
        try
        {
            static std::atomic<bool> stopping{false};
            bool expected = false;
            if (!stopping.compare_exchange_strong(expected, true))
                return;

            logger.info("Stopping HTTP server");
            logger.add_tab();
            svr.stop();

            logger.info("Waiting for thread to join");
            if (runner.joinable())
                runner.join();
            logger.rem_tab();
            logger.info("HTTP server stopped");
        }
        catch (...)
        {
        }
    }
};
