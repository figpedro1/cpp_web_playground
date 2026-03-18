#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <optional>

namespace auth {
    std::optional<nlohmann::json> get_token(string client_id, string client_secret, string scope = "viewables:read");
}