#include "viewer_app.hpp"

#include "viewer_DBManager.hpp"
#include "viewer_auth.hpp"
#include "ServerConfig.hpp"
#include "utils.hpp"
#include "crow.h"
#include "dotenv.h"
#include <chrono>
#include <ctime>
#include <memory>

namespace autodesk_viewer {
    AutodeskViewer::AutodeskViewer(server::ServerConfig cfg, std::shared_ptr<database::DBPool> db_pool)
        : dbmanager(db_pool)
    {
        this->storage_dir = std::filesystem::path(cfg.storage_root_dir) / cfg.viewer_dir;
        this->client_id = cfg.autodesk_client_id;
        this->client_secret = cfg.autodesk_client_secret;
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

    void AutodeskViewer::sync_db() {
        auto dirs = utils::scan_dir(this->storage_dir, true);
        std::sort(dirs.begin(), dirs.end(), [](const auto& a, const auto& b) {
            return a.path().string().length() < b.path().string().length();
        });
            pqxx::connection conn("");  //apagar
            pqxx::work txn(conn);       //apagar
            this->dbmanager.upsert_viewer_folder(
                {
                    .name = this->storage_dir.filename().string(),
                    .path_from_root = this->storage_dir.string()
                }
            );

            for (const auto& entry : dirs) {
                if (entry.is_directory()) {
                    std::string file_name = entry.path().filename().string();
                    std::string parent_path = entry.path().parent_path().string();
                    std::string file_path = entry.path().string();

                    this->dbmanager.upsert_viewer_folder(
                        {
                            .name = entry.path().filename().string(),
                            .path_from_root = entry.path().string()
                        }
                    );
                } else if(entry.is_regular_file()) {

                    auto r = txn.exec_params(
                        "SELECT EXTRACT(EPOCH FROM last_modified)::BIGINT FROM viewer_oss_objects WHERE path_from_root = $1",
                        entry.path().string()
                    );
                    
                    std::time_t last_modified = utils::get_last_modified(entry.path());
                    
                    if (r.empty() ? true : r[0][0].as<time_t>() != last_modified) {
                    
                        std::string file_name = entry.path().filename().string();
                        std::string parent_path = entry.path().parent_path().string();
                        std::string sha1 = utils::calculate_sha1(entry.path());
                        uintmax_t file_size = entry.file_size();
                        std::string path_from_root = entry.path().string();


                        if (r.empty()) {
                            txn.exec_params(
                                "INSERT INTO viewer_oss_objects (file_name, folder_id, bucket_key, object_key, "
                                "sha1, size, last_modified, path_from_root, created_at) "
                                "VALUES ($1, (SELECT id FROM viewer_folders WHERE path_from_root = $2), "
                                "(SELECT associated_bucket_key FROM viewer_folders WHERE path_from_root = $2), "
                                "$1, $3, $4, to_timestamp($5), $6, NOW())",
                                file_name,
                                parent_path,
                                sha1,
                                file_size,
                                last_modified,
                                path_from_root
                            );
                        } else {
                            txn.exec_params(
                                "UPDATE viewer_oss_objects "
                                "SET sha1 = $1, size = $2, last_modified = to_timestamp($3) "
                                "WHERE path_from_root = $4",
                                sha1,
                                file_size,
                                last_modified,
                                path_from_root
                            );
                        }
                        
                    }
                }
            }

            txn.commit();
    }
}