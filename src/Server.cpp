#include "Server.hpp"

#include "viewer_Token.hpp"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <iostream>
#include <memory>
#include <filesystem>

namespace server {

    Server::Server (ServerConfig server_config) {
        this->cfg = server_config;

        std::string db_connection_string = "postgresql://" + this->cfg.database_user + ":" + this->cfg.database_password + "@" + this->cfg.database_address + "/" + this->cfg.database_name + "?sslmode=disable";
        try {
        
            this->db = std::make_shared<database::DBPool>(db_connection_string, 2);

            CROW_LOG_INFO << "Database connection successfully estabilished";
        } catch (...) {
            CROW_LOG_CRITICAL << "Unable to connect to database.";
        }

        this->autodesk_viewer = std::make_unique<autodesk_viewer::AutodeskViewer>(this->cfg, this->db);

        this->autodesk_viewer->sync_db();
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
            std::optional<autodesk_viewer::Token> token = this->autodesk_viewer->get_public_token();
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