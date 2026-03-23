#pragma once

#include "dotenv.h"
#include <string>

namespace server {
    struct ServerConfig {
        std::string autodesk_client_id;
        std::string autodesk_client_secret;
        std::string database_name;
        std::string database_user;
        std::string database_password;
        std::string database_address;
        std::string storage_root_dir;
        std::string viewer_dir;
        int port;
    };
}