#pragma once

#include "crow.h"
#include "viewer_app.hpp"
#include "ServerConfig.hpp"

namespace server {
    class Server{
        private:
            std::unique_ptr<autodesk_viewer::AutodeskViewer> autodesk_viewer;
            crow::SimpleApp app;
            ServerConfig cfg;

        public:
            Server();
            void setup_routes();
            void start_server();
    };
}