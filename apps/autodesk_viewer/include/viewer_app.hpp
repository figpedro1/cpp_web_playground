#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace autodesk_viewer{
    class AutodeskViewer {
        private:
            std::string client_id;
            std::string client_secret;

        public:
            AutodeskViewer(std::string aps_client_id, std::string aps_client_secret);
            std::optional<nlohmann::json> get_viewer_token ();
    };
}