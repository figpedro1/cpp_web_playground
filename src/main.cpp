#include "crow.h"
#include "ServerConfig.hpp"
#include "routes.hpp"
#include <iostream>

int main() {
    crow::SimpleApp app;
    ServerConfig cfg;
    server::setup_routes(app, cfg);

    return 0;
}