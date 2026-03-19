#include "server.hpp"

#include "ServerConfig.hpp"
#include "crow.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <iostream>
#include <dotenv.h>
#include <memory>

namespace server {

    Server::Server () {
        dotenv::init();
        this->cfg.client_id = std::getenv("APS_CLIENT_ID") ?: "";
        this->cfg.client_secret = std::getenv("APS_CLIENT_SECRET") ?: "";
        try {
            int port = std::stoi(std::getenv("SERVER_PORT") ?: "8080");
            if (port < 10 || port > 9999) {
                port = 8080;
            }
            this->cfg.port = port;
        } catch (...) {
            this->cfg.port = 8080;
        }

        this->autodesk_viewer = std::make_unique<autodesk_viewer::AutodeskViewer>(this->cfg.client_id, this->cfg.client_secret);

    }
    
    void Server::setup_routes () {
        
        CROW_ROUTE(this->app, "/")([](){
            return "Servidor está funcionando!!! Hospedado na AWS";
        });

        CROW_ROUTE(this->app, "/ping")([](){
            return "pong";
        });

        CROW_ROUTE(this->app, "/test_auth")([this](){
            std::optional<nlohmann::json> r = this->autodesk_viewer->get_viewer_token();
            if (!r) {
                return std::string("pluh");
            }
            return r.value().dump();
        });

        }

    void Server::start_server() {
        this->setup_routes();
        this->app.port(this->cfg.port).multithreaded().run();
    }

}