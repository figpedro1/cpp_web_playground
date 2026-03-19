#include "viewer_app.hpp"

#include "dotenv.h"
#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include "viewer_auth.hpp"
#include <cpr/cpr.h>

namespace autodesk_viewer {
    AutodeskViewer::AutodeskViewer(std::string aps_client_id, std::string aps_client_secret){
        this->client_id = aps_client_id;
        this->client_secret = aps_client_secret;
    }

    std::optional<nlohmann::json> AutodeskViewer::get_viewer_token () {
        auto r = autodesk_viewer::get_token(this->client_id, this->client_secret, "viewables:read");

        if (!r) {
            return std::nullopt;
        }

        return nlohmann::json::parse(r.value().text);
    }
}