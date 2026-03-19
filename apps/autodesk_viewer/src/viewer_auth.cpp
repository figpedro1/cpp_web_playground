#include "viewer_auth.hpp"

#include "utils.hpp"
#include <iostream>
#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>




namespace autodesk_viewer {
    
    std::optional<cpr::Response> get_token (std::string client_id, std::string client_secret, std::string scope = "viewables:read", std::string auth_type = "Basic"){
        std::string b64credentials = utils::base64_encode(client_id + ":" + client_secret);

        cpr::Response r = cpr::Post(
            cpr::Url{"https://developer.api.autodesk.com/authentication/v2/token"},
            cpr::Header{{"Authorization", auth_type + " " + b64credentials}, {"Content-Type", "application/x-www-form-urlencoded"}},
            cpr::Payload{{"grant_type", "client_credentials"}, {"scope", scope}}
        );

        if (r.status_code != 200) {
            return std::nullopt;
        }

        return r;
    }
    
}