#include "apps/autodesk_viewer/include/auth.hpp"

#include "include/utils.hpp"
#include <iostream>
#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>


using nlohmann::json;
using std::string;

namespace auth {
    
    std::optional<json> get_token (string client_id, string client_secret, string scope){
        string b64credentials = utils::base64_encode(client_id + ":" + client_secret);

        cpr::Response r = cpr::Post(
            cpr::Url{"https://developer.api.autodesk.com/authentication/v2/token"},
            cpr::Header{{"Authorization", "Basic" + b64credentials}, {"Content-Type", "application/x-www-form-urlencoded"}},
            cpr::Payload{{"grant_type", "client_credentials"}, {"scope", scope}}
        );

        if (r.status_code != 200) {
            return std::nullopt;
        }

        return json::parse(r.text);
    }
    
}