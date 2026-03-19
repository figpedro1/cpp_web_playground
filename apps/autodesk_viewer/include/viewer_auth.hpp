#pragma once

#include <cpr/cpr.h>
#include <string>
#include <optional>

namespace autodesk_viewer {

    std::optional<cpr::Response> get_token (std::string client_id, std::string client_secret, std::string scope);

}