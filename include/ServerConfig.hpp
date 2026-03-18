#pragma once

#include "dotenv.h"
#include <string>

struct ServerConfig {
    std::string aps_client_id;
    std::string aps_client_secret;
    int port;

    ServerConfig (int port = 8080) {
        dotenv::init("../.env");
        this->aps_client_id = std::getenv("APS_CLIENT_ID") ?: "";
        this->aps_client_secret = std::getenv("APS_CLIENT_SECRET") ?: "";
        this->port = port;

    }

};