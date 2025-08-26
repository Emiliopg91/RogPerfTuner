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
    Logger logger{"HttpServer"};
    std::thread runner;

    HttpServer()
    {
        logger.info("Initializing HTTP server");

        svr.set_pre_routing_handler([this](const httplib::Request &req, httplib::Response &res)
                                    { 
                                        logger.info("Incoming "+req.method+" request to "+req.path);
                                        logger.add_tab();
                                        return httplib::Server::HandlerResponse::Unhandled; });
        svr.set_post_routing_handler([this](const httplib::Request &req, httplib::Response &res)
                                     { 
                                        logger.rem_tab(); 
                                        logger.info("Finished request wit status: " + std::to_string(res.status)); });

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

        runner = std::thread([this]()
                             { run(); });
    }

public:
    static HttpServer &getInstance()
    {
        static HttpServer instance;
        return instance;
    }

    void run()
    {
        logger.info("Server up and running on localhost:" + Constants::HTTP_PORT);
        svr.listen("127.0.0.1", Constants::HTTP_PORT);
    }
};
