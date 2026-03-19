#pragma once

#include "dotenv.h"
#include <string>

struct ServerConfig {
    std::string client_id;
    std::string client_secret;
    int port;
};