#include "DBPool.hpp"

    DBPool::DBPool(std::string conn_str, size_t pool_size) {
        for (size_t i = 0; i < pool_size; ++i) {
            connections.emplace_back(std::make_unique<pqxx::connection>(conn_str));
        }
    }

    std::unique_ptr<pqxx::connection, std::function<void(pqxx::connection*)>> DBPool::acquire() {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [this] { return !connections.empty(); });

        auto conn = std::move(connections.back());
        connections.pop_back();

        return std::unique_ptr<pqxx::connection, std::function<void(pqxx::connection*)>>(
            conn.release(),
            [this](pqxx::connection* c) {
                std::lock_guard<std::mutex> lock(mutex);
                connections.push_back(std::unique_ptr<pqxx::connection>(c));
                condition.notify_one();
            }
        );
    }
