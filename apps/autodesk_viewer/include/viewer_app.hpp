#pragma once

#include "db/DBPool.hpp"
#include "ServerConfig.hpp"
#include "viewer_Token.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <mutex>
#include <filesystem>
#include <memory>

namespace autodesk_viewer{
    class AutodeskViewer {
        private:
            std::string client_id;
            std::string client_secret;
            Token public_token;
            Token private_token;
            std::filesystem::path storage_dir;
            std::mutex pvt_token_mutex;
            std::mutex pb_token_mutex;
            std::shared_ptr<database::DBPool> db;


        public:
            AutodeskViewer(server::ServerConfig cfg, std::shared_ptr<database::DBPool> db_pool);
            std::optional<Token> get_public_token ();
            std::optional<Token> get_private_token ();
            void sync_db();
    };
}