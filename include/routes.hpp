#pragma once

#include "crow.h"

namespace server {

    void setup_routes(crow::SimpleApp& app, const ServerConfig& cfg);

}