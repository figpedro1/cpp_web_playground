#include "crow.h"
#include "ServerConfig.hpp"
#include "Server.hpp"
#include <iostream>

int main() {
    crow::logger::setLogLevel(crow::LogLevel::Debug);
    server::Server server;

    server.start_server();
    
    return 0;
}