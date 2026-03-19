#include "Server.hpp"

#include "ServerConfig.hpp"
#include "viewer_app.hpp"
#include "viewer_Token.hpp"
#include "crow.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <iostream>
#include <dotenv.h>
#include <memory>

namespace server {

    Server::Server () {
        dotenv::init("../.env");
        this->cfg.client_id = std::getenv("APS_CLIENT_ID") ?: "";
        this->cfg.client_secret = std::getenv("APS_CLIENT_SECRET") ?: "";
        try {
            int port = std::stoi(std::getenv("SERVER_PORT"));
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
            crow::response res("Servidor está funcionando!");
            res.set_header("Content-Type", "text/plain; charset=utf-8");
            return res;
        });

        CROW_ROUTE(this->app, "/ping")([](){
            return "pong";
        });

        CROW_ROUTE(this->app, "/test_auth")([this](){
            std::optional<autodesk_viewer::Token> token = this->autodesk_viewer->get_viewer_token();
            if (!token) {
                crow::response res("pluh");
                res.set_header("Content-Type", "text/plain; charset=utf-8");
                return res;
            }

            nlohmann::json response_body = {
                {"access_token", token.value().access_token},
                {"token_type", token.value().token_type},
                {"expires_in", token.value().expires_in}
            };

            crow::response res(response_body.dump());
            res.set_header("Content-Type", "application/json; charset=utf-8");

            return res;
        });

        }

    void Server::start_server() {
        this->setup_routes();
        std::cout << "App rodando na porta " << this->cfg.port << std::endl;
        this->app.port(this->cfg.port).multithreaded().run();
    }

}