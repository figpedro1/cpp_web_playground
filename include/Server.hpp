#pragma once

#include "crow.h"
#include "viewer_app.hpp"
#include "ServerConfig.hpp"
#include "DBPool.hpp"

namespace server {
    class Server{
        private:
            std::unique_ptr<autodesk_viewer::AutodeskViewer> autodesk_viewer;
            crow::SimpleApp app;
            ServerConfig cfg;
            std::unique_ptr<DBPool> db;

        public:
            Server(ServerConfig server_config);
            void setup_routes();
            void start_server();
    };
}