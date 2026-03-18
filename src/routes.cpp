#include "routes.hpp"

#include "auth.hpp"
#include "ServerConfig.hpp"
#include "crow.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <iostream>

namespace server {

    void setup_routes(crow::SimpleApp& app, const ServerConfig& cfg) {
        
        CROW_ROUTE(app, "/")([](){
            return "Servidor está funcionando!!! Hospedado na AWS";
        });

        CROW_ROUTE(app, "/ping")([](){
            return "pong";
        });

        CROW_ROUTE(app, "/test_auth")([&cfg](){
            auto token = auth::get_token(cfg.aps_client_id, cfg.aps_client_secret);
            if (token) {
                return token.value().dump();
            }
            return "pluh";
        });

        }

}