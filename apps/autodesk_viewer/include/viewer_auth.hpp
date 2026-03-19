#pragma once

#include <cpr/cpr.h>
#include <string>
#include <optional>

namespace autodesk_viewer {

    nlohmann::json get_token (std::string client_id, std::string client_secret, std::string scope = "viewables:read", std::string auth_type = "Basic");

}