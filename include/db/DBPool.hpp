#pragma once

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <pqxx/pqxx>

namespace database {
    class DBPool {
    public:
        DBPool(std::string conn_str, size_t pool_size);
        std::unique_ptr<pqxx::connection, std::function<void(pqxx::connection*)>> acquire();

    private:
        std::vector<std::unique_ptr<pqxx::connection>> connections;
        std::mutex mutex;
        std::condition_variable condition;
    };
}