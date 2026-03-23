#include "crow.h"
#include "ServerConfig.hpp"
#include "Server.hpp"
#include <iostream>
#include <string>
#include <dotenv.h>

int main() {
    
    dotenv::init();

    const char* env_ptr = std::getenv("APP_ENV");

    if (env_ptr == nullptr) {
        CROW_LOG_CRITICAL << ".env file or APP_ENV variable not found, aborting...";
        return -1;
    } else {
        CROW_LOG_INFO << ".env file initialized successfully";

        if (std::string(env_ptr) == "PROD") {
            crow::logger::setLogLevel(crow::LogLevel::INFO);
            CROW_LOG_INFO << "Running in 'PROD' environment";
        } else if (std::string(env_ptr) == "DEV") {
            crow::logger::setLogLevel(crow::LogLevel::DEBUG);
            CROW_LOG_INFO << "Running in 'DEV' environment";
        }
    }

    server::ServerConfig server_config;
    
    server_config.autodesk_client_id = std::getenv("APS_CLIENT_ID") ?: "";
    server_config.autodesk_client_secret = std::getenv("APS_CLIENT_SECRET") ?: "";
    server_config.database_name = std::getenv("DB_NAME") ?: "";
    server_config.database_user = std::getenv("DB_USER") ?: "";
    server_config.database_password = std::getenv("DB_PSWD") ?: "";
    server_config.database_address = std::getenv("DB_ADDR") ?: "";
    server_config.storage_root_dir = std::getenv("STORAGE_ROOT_DIR") ?: "";
    server_config.viewer_dir = std::getenv("VIEWER_DIR") ?: "";
    try {
        int port = std::stoi(std::getenv("SERVER_PORT"));
        if (port < 10 || port > 9999) {
            port = 8080;
        }
        server_config.port = port;
    } catch (...) {
        server_config.port = 8080;
    }
    
    server::Server server(server_config);

    server.start_server();
    
    return 0;
}