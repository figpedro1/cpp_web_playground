#pragma once

#include <string>
#include <chrono>

namespace autodesk_viewer {
    struct Token {
        std::string access_token;
        std::string token_type;
        std::chrono::system_clock::time_point expiry_time;
        int expires_in;
        std::string refresh_token = "N/A";
        std::string scope = "N/A";
    };
}