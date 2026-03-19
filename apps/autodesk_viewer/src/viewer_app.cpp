#include "viewer_app.hpp"

#include "viewer_Token.hpp"
#include "viewer_auth.hpp"
#include "crow.h"
#include "dotenv.h"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <chrono>
#include <mutex>

namespace autodesk_viewer {
    AutodeskViewer::AutodeskViewer(std::string aps_client_id, std::string aps_client_secret){
        this->client_id = aps_client_id;
        this->client_secret = aps_client_secret;
        this->public_token.expiry_time = std::chrono::system_clock::now();
        this->private_token.expiry_time = std::chrono::system_clock::now();
    }

    std::optional<Token> AutodeskViewer::get_public_token () {

        if (std::chrono::system_clock::now() < this->public_token.expiry_time){
            return this->public_token;
        }

        std::lock_guard<std::mutex> lock(this->pb_token_mutex);

        if (std::chrono::system_clock::now() < this->public_token.expiry_time){
            return this->public_token;
        }

        nlohmann::json r = autodesk_viewer::get_token(this->client_id, this->client_secret, "viewables:read");
        CROW_LOG_DEBUG << "API KEY: " << this->client_id;
        CROW_LOG_DEBUG << "API SECRET: " << this->client_secret;
        if (r["status_code"] != 200) {
            CROW_LOG_ERROR << "Could not retrieve access token, error: " << r.dump(2);
            return std::nullopt;
        }

        this->public_token.access_token = r["body"]["access_token"];
        this->public_token.token_type = r["body"]["token_type"];
        this->public_token.expires_in = r["body"]["expires_in"];
        this->public_token.expiry_time = std::chrono::system_clock::now() + std::chrono::seconds(this->public_token.expires_in/2);
        this->public_token.refresh_token = "N/A";
        this->public_token.scope = "viewables:read";

        return this->public_token;
    }

        std::optional<Token> AutodeskViewer::get_private_token () {

        if (std::chrono::system_clock::now() < this->private_token.expiry_time){
            return this->private_token;
        }

        std::lock_guard<std::mutex> lock(this->pvt_token_mutex);

        if (std::chrono::system_clock::now() < this->private_token.expiry_time){
            return this->private_token;
        }

        std::string scopes = "data:read data:write data:create data:search bucket:create bucket:read bucket:update bucket:delete viewables:read";
        
        nlohmann::json r = autodesk_viewer::get_token(this->client_id, this->client_secret, scopes);
        CROW_LOG_DEBUG << "API KEY: " << this->client_id;
        CROW_LOG_DEBUG << "API SECRET: " << this->client_secret;
        if (r["status_code"] != 200) {
            CROW_LOG_ERROR << "Could not retrieve access token, error: " << r.dump(2);
            return std::nullopt;
        }

        this->private_token.access_token = r["body"]["access_token"];
        this->private_token.token_type = r["body"]["token_type"];
        this->private_token.expires_in = r["body"]["expires_in"];
        this->private_token.expiry_time = std::chrono::system_clock::now() + std::chrono::seconds(this->private_token.expires_in/2);
        this->private_token.refresh_token = "N/A";
        this->private_token.scope = scopes;

        return this->private_token;
    }
}