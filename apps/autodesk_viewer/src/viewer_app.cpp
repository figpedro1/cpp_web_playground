#include "viewer_app.hpp"

#include "viewer_Token.hpp"
#include "viewer_auth.hpp"
#include "crow.h"
#include "dotenv.h"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <chrono>

namespace autodesk_viewer {
    AutodeskViewer::AutodeskViewer(std::string aps_client_id, std::string aps_client_secret){
        this->client_id = aps_client_id;
        this->client_secret = aps_client_secret;
        this->token.expiry_time = std::chrono::system_clock::now();
    }

    std::optional<Token> AutodeskViewer::get_viewer_token () {

        if (std::chrono::system_clock::now() < this->token.expiry_time){
            return this->token;
        }

        nlohmann::json r = autodesk_viewer::get_token(this->client_id, this->client_secret, "viewables:read");

        if (r["status_code"] != "200") {
            CROW_LOG_ERROR << "Could not retrieve access token, error: " << r["error"] << "          error message: " << r["error_description"];
            return std::nullopt;
        }

        this->token.access_token = r["access_token"];
        this->token.token_type = r["token_type"];
        this->token.expires_in = r["expires_in"];
        this->token.expiry_time = std::chrono::system_clock::now() + std::chrono::seconds(this->token.expires_in/2);
        this->token.refresh_token = "N/A";
        this->token.scope = "viewables:read";

        return this->token;
    }
}