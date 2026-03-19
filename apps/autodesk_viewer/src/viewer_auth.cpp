#include "viewer_auth.hpp"

#include "utils.hpp"
#include <iostream>
#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>




namespace autodesk_viewer {
    
    nlohmann::json get_token (std::string client_id, std::string client_secret, std::string scope, std::string auth_type){
        std::string b64credentials = utils::base64_encode(client_id + ":" + client_secret);

        cpr::Response r = cpr::Post(
            cpr::Url{"https://developer.api.autodesk.com/authentication/v2/token"},
            cpr::Header{{"Authorization", auth_type + " " + b64credentials}, {"Content-Type", "application/x-www-form-urlencoded"}},
            cpr::Payload{{"grant_type", "client_credentials"}, {"scope", scope}}
        );
        
        nlohmann::json token{{"status_code", std::to_string(r.status_code)}, {"body", nlohmann::json::parse(r.text)}};

        return token;
    }
    
}