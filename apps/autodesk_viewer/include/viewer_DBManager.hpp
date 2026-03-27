#pragma once

#include "db/DBPool.hpp"
#include "viewer_ObjectStatus.hpp"
#include "db/FunctionBuilder.hpp"
#include <memory>
#include <string>
#include <ctime>
#include <optional>
#include <vector>
#include <utility>

namespace autodesk_viewer {
    struct ViewerOssObject {
        std::optional<uintmax_t> id;
        std::optional<std::string> file_name;
        std::optional<bool> file_exists;
        std::optional<std::string> path_from_root;
        std::optional<uintmax_t> folder_id;
        std::optional<std::string> bucket_key;
        std::optional<std::string> object_key;
        std::optional<std::string> object_id;
        std::optional<std::string> sha1;
        std::optional<uintmax_t> size;
        std::optional<std::string> location;
        std::optional<ObjectStatus> status;
        std::optional<time_t> last_modified;
        std::optional<time_t> created_at;
        std::optional<time_t> deleted_at;
        std::optional<time_t> uploaded_at;
    };

    std::vector<ViewerOssObject> map_viewer_object (pqxx::result result);

    struct ViewerFolder {
        std::optional<uintmax_t> id;
        std::optional<uintmax_t> parent_id;
        std::optional<std::string> name;
        std::optional<std::string> associated_bucket_key;
        std::optional<std::string> path_from_root;
        std::optional<time_t> created_at;
        std::optional<time_t> updated_at;
        std::optional<std::string> thumbnail_path_from_root;
    };

    std::vector<ViewerFolder> map_viewer_folder (pqxx::result result);

    class DBManager {
        private:
            std::shared_ptr<database::DBPool> db;
            std::string objects_table = "viewer_oss_objects";
            std::string folder_table = "viewer_folders";

            database::FunctionBuilder to_time_t ();

        public:
            DBManager(std::shared_ptr<database::DBPool> db_pool);

            void update_viewer_oss_object(ViewerOssObject object, bool update_path = false, std::string parent_path = "");
            void insert_viewer_oss_object(ViewerOssObject object, std::string parent_path = "");
            void upsert_viewer_oss_object(ViewerOssObject object, std::string parent_path = "");
            void delete_viewer_oss_object(ViewerOssObject object);
            std::vector<ViewerOssObject> get_viewer_oss_objects(ViewerOssObject object, uintmax_t limit = 0, std::vector<std::string> select_columns = {"*"}, bool no_where = false);

            void update_viewer_folder(ViewerFolder folder, bool update_path = false, std::string parent_path = "");
            void insert_viewer_folder(ViewerFolder folder, std::string parent_path = "");
            void upsert_viewer_folder(ViewerFolder folder, std::string parent_path = "");
            void delete_viewer_folder(ViewerFolder folder);
            std::vector<ViewerFolder> get_folders(ViewerFolder folder, uintmax_t limit = 0, std::vector<std::string> select_columns = {"*"}, bool no_where = false);

            void sync_db();
    };
}