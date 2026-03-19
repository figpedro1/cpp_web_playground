#pragma once

#include "viewer_Token.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <mutex>

namespace autodesk_viewer{
    class AutodeskViewer {
        private:
            std::string client_id;
            std::string client_secret;
            Token public_token;
            Token private_token;
            std::mutex pvt_token_mutex;
            std::mutex pb_token_mutex;

        public:
            AutodeskViewer(std::string aps_client_id, std::string aps_client_secret);
            std::optional<Token> get_public_token ();
            std::optional<Token> get_private_token ();
    };
}