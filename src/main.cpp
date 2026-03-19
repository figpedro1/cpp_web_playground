#include "crow.h"
#include "ServerConfig.hpp"
#include "Server.hpp"
#include <iostream>

int main() {
    server::Server server;

    server.start_server();
    
    return 0;
}