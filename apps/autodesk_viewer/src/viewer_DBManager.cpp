#include "viewer_DBManager.hpp"

#include "db/QueryBuilder.hpp"
#include "crow.h"
#include <iostream>
#include <memory>

#define OBJECT_NORMAL_FIELD_LIST                                   \
    X(file_name)                                                   \
    X(file_exists)                                                 \
    X(bucket_key)                                                  \
    X(object_key)                                                  \
    X(object_id)                                                   \
    X(sha1)                                                        \
    X(size)                                                        \
    X(location)

#define OBJECT_TIME_FIELD_LIST                                     \
    X(last_modified)                                               \
    X(uploaded_at)                                                 \
    X(deleted_at)

#define FOLDER_NORMAL_FIELD_LIST                                   \
    X(name)                                                        \
    X(associated_bucket_key)                                       \
    X(thumbnail_path_from_root)

#define FOLDER_TIME_FIELD_LIST                                     \
    X(updated_at)

namespace db = database;

namespace autodesk_viewer {
    std::vector<ViewerOssObject> map_viewer_object(pqxx::result result) {
        if (result.empty()) return {};
        
        std::vector<ViewerOssObject> mapped;
        for (const auto& row : result) {
            ViewerOssObject obj;

            if (!row["id"].is_null()) obj.id = row["id"].as<uintmax_t>();
            if (!row["file_name"].is_null()) obj.file_name = row["file_name"].as<std::string>();
            if (!row["file_exists"].is_null()) obj.file_exists = row["file_exists"].as<bool>();
            if (!row["path_from_root"].is_null()) obj.path_from_root = row["path_from_root"].as<std::string>();
            if (!row["sha1"].is_null()) obj.sha1 = row["sha1"].as<std::string>();
            if (!row["size"].is_null()) obj.size = row["size"].as<uintmax_t>();
            if (!row["status"].is_null()) obj.status = ObjectStatusUtils::from_string(row["status"].as<std::string>());
            if (!row["created_at"].is_null()) obj.created_at = row["created_at"].as<time_t>();
            if (!row["folder_id"].is_null()) obj.folder_id = row["folder_id"].as<uintmax_t>();
            if (!row["bucket_key"].is_null()) obj.bucket_key = row["bucket_key"].as<std::string>();
            if (!row["object_key"].is_null()) obj.object_key = row["object_key"].as<std::string>();
            if (!row["object_id"].is_null()) obj.object_id = row["object_id"].as<std::string>();
            if (!row["location"].is_null()) obj.location = row["location"].as<std::string>();
            if (!row["last_modified"].is_null()) obj.last_modified = row["last_modified"].as<time_t>();
            if (!row["uploaded_at"].is_null()) obj.uploaded_at = row["uploaded_at"].as<time_t>();
            if (!row["deleted_at"].is_null()) obj.deleted_at = row["deleted_at"].as<time_t>();

            mapped.push_back(std::move(obj));
        }
        return mapped;
    }

    std::vector<ViewerFolder> map_viewer_folder(pqxx::result result) {
        if (result.empty()) return {};

        std::vector<ViewerFolder> mapped;
        for (const auto& row : result) {
            ViewerFolder folder;

            if (!row["id"].is_null()) folder.id = row["id"].as<uintmax_t>();
            if (!row["name"].is_null()) folder.name = row["name"].as<std::string>();
            if (!row["path_from_root"].is_null()) folder.path_from_root = row["path_from_root"].as<std::string>();
            if (!row["created_at"].is_null()) folder.created_at = row["created_at"].as<time_t>();
            if (!row["updated_at"].is_null()) folder.updated_at = row["updated_at"].as<time_t>();
            
            if (!row["parent_id"].is_null()) folder.parent_id = row["parent_id"].as<uintmax_t>();
            if (!row["associated_bucket_key"].is_null()) {
                if (row["associated_bucket_key"].as<std::string>() != "NONE") {
                    folder.associated_bucket_key = row["associated_bucket_key"].as<std::string>();
                }
            }
            if (!row["thumbnail_path_from_root"].is_null()) {
                if (row["thumbnail_path_from_root"].as<std::string>() != "NONE") {
                    folder.thumbnail_path_from_root = row["thumbnail_path_from_root"].as<std::string>();
                }
            }
            mapped.push_back(std::move(folder));
        }
        return mapped;
    }

    DBManager::DBManager(std::shared_ptr<db::DBPool> db_pool): db(db_pool){}

    db::FunctionBuilder DBManager::to_time_t (){
        db::FunctionBuilder func("EXTRACT");
        db::FunctionParam param{.value = "EPOCH FROM", .position = 1, .equals_needed = false};
        func.add_insert_positions(param);

        return func;
    }

    void DBManager::update_viewer_oss_object(ViewerOssObject object, bool update_path, std::string parent_path){
        db::QueryBuilder query;
        pqxx::params params;

        query.set_table(this->folder_table);
        
        #define X(field)                                           \
            if (object.field) {                                    \
                query.add_update(#field);                          \
                params.append(object.field.value());               \
            }
            OBJECT_NORMAL_FIELD_LIST
        #undef X

        #define X(field)                                           \
            if (object.field) {                                    \
                query.add_update(#field, to_time_t());             \
                params.append(object.field.value());               \
            }
            OBJECT_TIME_FIELD_LIST
        #undef X

            if (object.folder_id) {
                query.add_update("folder_id");
                params.append(object.folder_id.value());
            } else if (parent_path != "") {
                auto sub_query = std::make_shared<db::QueryBuilder>();
                sub_query->set_table(folder_table);
                sub_query->add_where("path_from_root");
                query.add_update("folder_id", std::nullopt, sub_query);
                params.append(parent_path);
            }

            if (object.path_from_root && update_path) {
                query.add_update("path_from_root");
                params.append(object.path_from_root.value());
            }

            if (object.status) {
                query.add_update("status");
                params.append(ObjectStatusUtils::to_string(object.status.value()));
            }

            if (object.id) {
                query.add_where("id");
                params.append(object.id.value());
            }

            if (object.path_from_root && !update_path) {
                query.add_where("path_from_root");
                params.append(object.path_from_root.value());
            }

        try {
            auto conn_ptr = this->db->acquire();
            pqxx::work txn(*conn_ptr);
            txn.exec_params(query.build_update_query(), params);
            txn.commit();

        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "viewer_oss_objects update error: " << e.what();
        }

        return;
    }

    void DBManager::insert_viewer_oss_object(ViewerOssObject object, std::string parent_path){
        db::QueryBuilder query;
        pqxx::params params;

        query.set_table(this->objects_table);

        #define X(field)                                           \
            if (object.field) {                                    \
                query.add_insert(#field);                          \
                params.append(object.field.value());               \
            }
            OBJECT_NORMAL_FIELD_LIST
        #undef X

        #define X(field)                                           \
            if (object.field) {                                    \
                query.add_insert(#field, to_time_t());             \
                params.append(object.field.value());               \
            }
            OBJECT_TIME_FIELD_LIST
        #undef X
        
        if (object.folder_id) {
            query.add_insert("folder_id");
            params.append(object.folder_id.value());
        } else if (parent_path != "") {
            auto sub_query = std::make_shared<db::QueryBuilder>();
            sub_query->set_table(folder_table);
            sub_query->add_where("path_from_root");
            query.add_insert("folder_id", std::nullopt, sub_query);
            params.append(parent_path);
        }
        
        if (object.path_from_root) {
            query.add_insert("path_from_root").add_conflict("path_from_root");
            params.append(object.path_from_root.value());
        }

        if (object.status) {
            query.add_insert("status");
            params.append(ObjectStatusUtils::to_string(object.status.value()));
        }

        try {
            auto conn_ptr = this->db->acquire();
            pqxx::work txn(*conn_ptr);
            txn.exec_params(query.build_insert_query(), params);
            txn.commit();
        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "viewer_oss_objects update error: " << e.what();
        }

        return;
    }

    void DBManager::upsert_viewer_oss_object(ViewerOssObject object, std::string parent_path){
        db::QueryBuilder query;
        pqxx::params params;

        query.set_table(this->objects_table);
        
        #define X(field)                                           \
            if(object.field) {                                     \
                query.add_insert(#field);                          \
                query.add_update(#field);                          \
                params.append(object.field.value());               \
            }
        OBJECT_NORMAL_FIELD_LIST
        #undef X

        #define X(field)                                           \
            if(object.field) {                                     \
                query.add_insert(#field, to_time_t());             \
                query.add_update(#field);                          \
                params.append(object.field.value());               \
            }
        OBJECT_TIME_FIELD_LIST
        #undef X

        if (object.folder_id) {
            query.add_insert("folder_id");
            query.add_update("folder_id");
            params.append(object.folder_id.value());
        } else if (parent_path != "") {
            auto sub_query = std::make_shared<db::QueryBuilder>();
            sub_query->set_table(folder_table);
            sub_query->add_where("path_from_root");
            query.add_insert("folder_id", std::nullopt, sub_query);
            query.add_update("folder_id");
            params.append(parent_path);
        }

        if(object.path_from_root) {
            query.add_insert("path_from_root");
            query.add_update("path_from_root");
            params.append(object.path_from_root.value());
        }

        if(object.status) {
            query.add_insert("status");
            query.add_update("status");
            params.append(ObjectStatusUtils::to_string(object.status.value()));
        }

        query.add_conflict("path_from_root");

        try {
            auto conn_ptr = this->db->acquire();
            pqxx::work txn(*conn_ptr);
            txn.exec_params(query.build_upsert_query(), params);
            txn.commit();
        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "viewer_oss_objects upsert error: " << e.what();
        }

        return;
    }

    void DBManager::delete_viewer_oss_object(ViewerOssObject object) {
        db::QueryBuilder query;
        pqxx::params params;

        query.set_table(this->objects_table);

        #define X(field)                                           \
            if (object.field) {                                    \
                query.add_where(#field);                           \
                params.append(object.field.value());               \
            }
        OBJECT_NORMAL_FIELD_LIST
        #undef X

        if (object.id) {
            query.add_where("id");
            params.append(object.id.value());
        }

        if (object.path_from_root) {
            query.add_where("path_from_root");
            params.append(object.path_from_root.value());
        }

        try {
            auto conn_ptr = this->db->acquire();
            pqxx::work txn(*conn_ptr);
            txn.exec_params(query.build_delete_query(), params);
            txn.commit();
        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "viewer_oss_objects delete error: " << e.what();
        }

        return;
    }

    std::vector<ViewerOssObject> DBManager::get_viewer_oss_objects(
        ViewerOssObject object, uintmax_t limit, std::vector<std::string> select_columns,
        bool no_where
    ) {
        db::QueryBuilder query;
        pqxx::params params;

        #define X(field)                                          \
            if (object.field) {                                   \
                query.add_where(#field);                          \
                params.append(object.field.value());              \
            }
        OBJECT_NORMAL_FIELD_LIST
        #undef X

        #define X(field)                                          \
            if (object.field){                                    \
                query.add_where(#field, "=", "AND", to_time_t()); \
                params.append(object.field.value());              \
            }
        OBJECT_TIME_FIELD_LIST
        #undef X

        if (object.id) {
            query.add_where("id");
            params.append(object.id.value());
        }

        if (object.path_from_root) {
            query.add_where("path_from_root");
            params.append(object.path_from_root.value());
        }

        query.add_select(select_columns);

        try {
            auto conn_ptr = this->db->acquire();
            pqxx::work txn(*conn_ptr);
            pqxx::result res = txn.exec_params(query.build_select_query(no_where, limit), params);

            return map_viewer_object(res);
        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "viewer_oss_objects select error: " << e.what();
        }
    }

    void DBManager::update_viewer_folder(ViewerFolder folder, bool update_path, std::string parent_path) {
        if (!folder.path_from_root && !folder.id) {
            CROW_LOG_ERROR << "viewer_folders update failed: Cannot update without path or id";
            return;
        }
        
        db::QueryBuilder query;
        pqxx::params params;

        query.set_table(this->folder_table);

        #define X(field)                                            \
            if (folder.field) {                                     \
                query.add_update(#field);                           \
                params.append(folder.field.value());                \
            }
        FOLDER_NORMAL_FIELD_LIST
        #undef X

        #define X(field)                                            \
            if (folder.field) {                                     \
                query.add_update(#field, to_time_t());              \
                params.append(folder.field.value());                \
            }
        FOLDER_TIME_FIELD_LIST
        #undef X

        if (folder.parent_id) {
            query.add_update("parent_id");
            params.append(folder.parent_id.value());
        } else if (parent_path != "") {
            auto sub_query = std::make_shared<db::QueryBuilder>();
            sub_query->set_table(folder_table);
            sub_query->add_where("path_from_root");
            query.add_update("parent_id", std::nullopt, sub_query);
            params.append(parent_path);
        }

        if (folder.path_from_root) {
            if (update_path) {
                query.add_update("path_from_root");
                params.append(folder.path_from_root.value());
            } else {
                query.add_where("path_from_root");
                params.append(folder.path_from_root.value());
            }
        }

        if (folder.id) {
            query.add_where("id");
            params.append(folder.id.value());
        }


        try {
            auto conn_ptr = this->db->acquire();
            pqxx::work txn(*conn_ptr);
            txn.exec_params(query.build_update_query(), params);
            txn.commit();
        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "viewer_folders insert error: " << e.what();
        }

        return;
    }

    void DBManager::insert_viewer_folder(ViewerFolder folder, std::string parent_path) {
        db::QueryBuilder query;
        pqxx::params params;
        query.set_table(this->folder_table);

        #define X(field)                                            \
            if (folder.field) {                                     \
                query.add_insert(#field);                           \
                params.append(folder.field.value());                \
            }
        FOLDER_NORMAL_FIELD_LIST
        #undef X

        #define X(field)                                            \
            if (folder.field) {                                     \
                query.add_insert(#field, to_time_t());              \
                params.append(folder.field.value());                \
            }
        FOLDER_TIME_FIELD_LIST
        #undef X

        if (folder.parent_id) {
            query.add_insert("parent_id");
            params.append(folder.parent_id.value());
        } else if (parent_path != "") {
            auto sub_query = std::make_shared<db::QueryBuilder>();
            sub_query->set_table(folder_table);
            sub_query->add_where("path_from_root");
            query.add_insert("parent_id", std::nullopt, sub_query);
            params.append(parent_path);
        }

        if (folder.path_from_root) {
            query.add_insert("path_from_root");
            params.append(folder.path_from_root.value());
        }

        try {
            auto conn_ptr = this->db->acquire();
            pqxx::work txn(*conn_ptr);
            txn.exec_params(query.build_insert_query(), params);
            txn.commit();
        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "viewer_folders insert error: " << e.what();
        }

        return;
    }

    void DBManager::upsert_viewer_folder(ViewerFolder folder, std::string parent_path) {
        if (!folder.path_from_root) {
            CROW_LOG_ERROR << "viewer_folder upsert error: Cannot upsert without path";
        }
        db::QueryBuilder query;
        pqxx::params params;
        query.set_table(this->folder_table);

        #define X(field)                                            \
            if (folder.field) {                                     \
                query.add_insert(#field);                           \
                query.add_update(#field);                           \
                params.append(folder.field.value());                \
            }
        FOLDER_NORMAL_FIELD_LIST
        #undef X

        #define X(field)                                            \
            if (folder.field) {                                     \
                query.add_insert(#field, to_time_t());              \
                query.add_update(#field);                           \
                params.append(folder.field.value());                \
            }
        FOLDER_TIME_FIELD_LIST
        #undef X

        if (folder.parent_id) {
            query.add_insert("parent_id");
            query.add_update("parent_id");
            params.append(folder.parent_id.value());
        } else if (parent_path != "") {
            auto sub_query = std::make_shared<db::QueryBuilder>();
            sub_query->set_table(folder_table);
            sub_query->add_where("path_from_root");
            query.add_insert("parent_id", std::nullopt, sub_query);
            query.add_update("parent_id");
            params.append(parent_path);
        }

        if (folder.path_from_root) {
            query.add_insert("path_from_root");
            query.add_conflict("path_from_root");
            params.append(folder.path_from_root.value());
        }

        if (folder.id) {
            query.add_conflict("id");
        }

        try {
            auto conn_ptr = this->db->acquire();
            pqxx::work txn(*conn_ptr);
            txn.exec_params(query.build_upsert_query(), params);
            txn.commit();
        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "viewer_folders upsert error: " << e.what();
        }

        return;
    }

    void DBManager::delete_viewer_folder(ViewerFolder folder){
        db::QueryBuilder query;
        pqxx::params params;

        query.set_table(this->folder_table);

        #define X(field)                                           \
            if (folder.field) {                                    \
                query.add_where(#field);                           \
                params.append(folder.field.value());               \
            }
        FOLDER_NORMAL_FIELD_LIST
        #undef X

        if (folder.id) {
            query.add_where("id");
            params.append(folder.id.value());
        }

        if (folder.path_from_root) {
            query.add_where("path_from_root");
            params.append(folder.path_from_root.value());
        }

        try {
            auto conn_ptr = this->db->acquire();
            pqxx::work txn(*conn_ptr);
            txn.exec_params(query.build_delete_query(), params);
            txn.commit();
        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "viewer_folder delete error: " << e.what();
        }

        return;
    }

    std::vector<ViewerFolder> DBManager::get_folders(
        ViewerFolder folder, uintmax_t limit, std::vector<std::string> select_columns,
        bool no_where
    ) {
        db::QueryBuilder query;
        pqxx::params params;

        #define X(field)                                          \
            if (folder.field) {                                   \
                query.add_where(#field);                          \
                params.append(folder.field.value());              \
            }
        FOLDER_NORMAL_FIELD_LIST
        #undef X

        #define X(field)                                          \
            if (folder.field){                                    \
                query.add_where(#field, "=", "AND", to_time_t()); \
                params.append(folder.field.value());              \
            }
        FOLDER_TIME_FIELD_LIST
        #undef X

        if (folder.id) {
            query.add_where("id");
            params.append(folder.id.value());
        }

        if (folder.path_from_root) {
            query.add_where("path_from_root");
            params.append(folder.path_from_root.value());
        }

        query.add_select(select_columns);

        try {
            auto conn_ptr = this->db->acquire();
            pqxx::work txn(*conn_ptr);
            pqxx::result res = txn.exec_params(query.build_select_query(no_where, limit), params);

            return map_viewer_folder(res);
        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "viewer_folders select error: " << e.what();
        }
    }

} 

