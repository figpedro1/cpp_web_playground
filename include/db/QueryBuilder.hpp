#pragma once

#include "db/FunctionBuilder.hpp"
#include <vector>
#include <string>
#include <optional>
#include <utility>
#include <cstdint>
#include <memory>

namespace database {
    class QueryBuilder;

    enum class QueryType { 
        INSERT,
        UPDATE,
        UPSERT,
        SELECT,
        DELETE
    };

    struct ColumnValue {
        std::string column;
        std::shared_ptr<QueryBuilder> query_builder;
        std::optional<FunctionBuilder> function_builder;
        std::string where_operator;
        std::string compare_with;
    };

    class QueryBuilder {
        private:
            std::optional<std::string> table_name;
            std::vector<ColumnValue> update_columns;
            std::vector<ColumnValue> insert_columns;
            std::vector<ColumnValue> where_clauses;
            std::vector<std::string> conflict_columns;
            std::vector<std::string> select_columns;

            std::string build_where(size_t& current_param) const;

        public:
            QueryBuilder();
            QueryBuilder(std::string table);
            QueryBuilder& set_table(std::string table);
            QueryBuilder& add_where(std::string column, std::string where_operator = "=", std::string where_comparator = "AND",
                std::optional<FunctionBuilder> function_builder = std::nullopt, std::shared_ptr<QueryBuilder> select_query = nullptr);
            QueryBuilder& add_where(ColumnValue column);
            QueryBuilder& add_where(std::vector<ColumnValue> column);
            QueryBuilder& add_update(std::string column, std::optional<FunctionBuilder> function_builder = std::nullopt, std::shared_ptr<QueryBuilder> select_query = nullptr);
            QueryBuilder& add_update(ColumnValue column);
            QueryBuilder& add_update(std::vector<ColumnValue> column);
            QueryBuilder& add_insert(std::string column, std::optional<FunctionBuilder> function_builder = std::nullopt, std::shared_ptr<QueryBuilder> select_query = nullptr);
            QueryBuilder& add_insert(ColumnValue column);
            QueryBuilder& add_insert(std::vector<ColumnValue> column);
            QueryBuilder& add_conflict(std::string column);
            QueryBuilder& add_conflict(std::vector<std::string> column);
            QueryBuilder& add_select(std::string column);
            QueryBuilder& add_select(std::vector<std::string> column);
            QueryBuilder& reset();
            std::string build_insert_query(bool no_conflict = false) const;
            std::string build_update_query(bool excluded = false, bool no_where = false) const;
            std::string build_upsert_query() const;
            std::string build_delete_query() const;
            std::string build_select_query(bool no_where = false, uintmax_t limit = 0) const;
            std::string build_select_query(size_t& current_param) const;
    };
}