#include "db/QueryBuilder.hpp"

#include <utility>

namespace database {
    QueryBuilder::QueryBuilder(){}
    QueryBuilder::QueryBuilder(std::string table){
        this->table_name = std::move(table);
    }

    QueryBuilder& QueryBuilder::set_table(std::string table){
        this->table_name = std::move(table);
        return *this;
    }
    
    QueryBuilder& QueryBuilder::add_where(std::string column, std::string where_operator, std::string where_comparator, std::optional<FunctionBuilder> func_builder, std::shared_ptr<QueryBuilder> select_query){
        ColumnValue val = {.column = std::move(column), .query_builder = select_query, .function_builder = func_builder, .where_operator = where_operator, .compare_with = where_comparator};
        this->where_clauses.push_back(std::move(val));
        return *this;
    }

    QueryBuilder& QueryBuilder::add_where(ColumnValue column){
        this->where_clauses.push_back(std::move(column));
        return *this;
    };

    QueryBuilder& QueryBuilder::add_where(std::vector<ColumnValue> column){
        this->where_clauses.insert(this->where_clauses.end(), column.begin(), column.end());
        return *this;
    }

    QueryBuilder& QueryBuilder::add_update(std::string column, std::optional<FunctionBuilder> func_builder, std::shared_ptr<QueryBuilder> select_query){
        ColumnValue val = {.column = std::move(column), .query_builder = select_query, .function_builder = func_builder};
        this->update_columns.push_back(std::move(val));
        return *this;
    }

    QueryBuilder& QueryBuilder::add_update(ColumnValue column){
        this->update_columns.push_back(std::move(column));
        return *this;
    }

    QueryBuilder& QueryBuilder::add_update(std::vector<ColumnValue> column){
        this->update_columns.insert(this->update_columns.begin(), column.begin(), column.end());
        return *this;
    }

    QueryBuilder& QueryBuilder::add_insert(std::string column, std::optional<FunctionBuilder> func_builder, std::shared_ptr<QueryBuilder> select_query){
        ColumnValue val = {.column = std::move(column), .query_builder = select_query, .function_builder = func_builder};
        this->insert_columns.push_back(std::move(val));
        return *this;
    }

    QueryBuilder& QueryBuilder::add_insert(ColumnValue column){
        this->insert_columns.push_back(std::move(column));
        return *this;
    };

    QueryBuilder& QueryBuilder::add_insert(std::vector<ColumnValue> column){
        this->insert_columns.insert(this->insert_columns.end(), column.begin(), column.end());
        return *this;
    }

    QueryBuilder& QueryBuilder::add_conflict(std::string column){
        this->conflict_columns.push_back(std::move(column));
        return *this;
    }

    QueryBuilder& QueryBuilder::add_conflict(std::vector<std::string> column){
        this->conflict_columns.insert(this->conflict_columns.end(), column.begin(), column.end());
        return *this;
    }

    QueryBuilder& QueryBuilder::add_select(std::string column){
        this->select_columns.push_back(std::move(column));
        return *this;
    }

    QueryBuilder& QueryBuilder::add_select(std::vector<std::string> column){
        this->select_columns.insert(this->select_columns.end(), column.begin(), column.end());
        return *this;
    }

    QueryBuilder& QueryBuilder::reset(){
        this->table_name = std::nullopt;
        this->where_clauses.clear();
        this->update_columns.clear();
        this->insert_columns.clear();
        this->conflict_columns.clear();
        return *this;
    }
    
    std::string QueryBuilder::build_insert_query(bool no_conflict) const {
        std::string columns;
        std::string values;
        std::string conflicts;
        size_t current_param = 1;

        for(const auto& insert : this->insert_columns) {
            if (!columns.empty()) columns += ", ";
            columns += insert.column;

            if (!values.empty()) values += ", ";
            if (insert.query_builder){
                values += "(" + insert.query_builder->build_select_query(current_param) + ")";
            } else if (insert.function_builder) {
                values += insert.function_builder.value().build_function(current_param);
            } else {
                values += "$" + std::to_string(current_param++);
            }
        }

        std::string query = "INSERT INTO " + this->table_name.value() + " (" + columns + 
            ") VALUES (" + values + ")";

        if (!this->conflict_columns.empty() && !no_conflict) {        
            for (const auto& conflict : this->conflict_columns) {
                    if (!conflicts.empty()) conflicts += ", ";
                    conflicts += conflict;
            }

            query += " ON CONFLICT (" + conflicts + ") DO NOTHING";
        }

        return query;
    }

    std::string QueryBuilder::build_update_query(bool excluded, bool no_where) const {
        if ((this->where_clauses.empty() && !no_where) || !this->table_name){
            return "";
        }
        std::string column_value;
        size_t current_param = 1;

        for (const auto& update : this->update_columns) {
            if (!column_value.empty()) column_value += ", ";
            if (excluded) {
                column_value += update.column + " = EXCLUDED." + update.column;
                continue;
            }

            if (update.query_builder) {
                column_value += update.column + " = " + update.query_builder->build_select_query(current_param);
                continue;
            }

            if (update.function_builder) {
                column_value += update.column + " = " + update.function_builder.value().build_function(current_param);
                continue;
            }

            column_value += update.column + " = $" + std::to_string(current_param++);
        }

        std::string query = "UPDATE " + (excluded ? "" : this->table_name.value() + " ") + "SET " + column_value;
        if (!no_where) query += this->build_where(current_param);

        return query;
    }

    std::string QueryBuilder::build_upsert_query() const {
        std::string conflicts;
        for (const auto& conflict : this->conflict_columns) {
            if (!conflicts.empty()) conflicts += ", ";
            conflicts += conflict;
        }

        return this->build_insert_query(true) + " ON CONFLICT (" + conflicts + ") DO " + build_update_query(true, true);
    }

    std::string QueryBuilder::build_delete_query() const {
        if (this->where_clauses.empty()){
            return "";
        }
        size_t current_param = 1;
        return "DELETE FROM " + this->table_name.value() + this->build_where(current_param);
    }

    std::string QueryBuilder::build_select_query(bool no_where, uintmax_t limit) const {
        if (this->where_clauses.empty() && !no_where){
            return "";
        }
        
        std::string select;
        for (const auto& i : this->select_columns) {
            if (!select.empty()) select += ", ";
            select += i;
        }

        size_t current_param = 1;

        return "SELECT " + (select.empty() ? "*" : select) + " FROM " + this->table_name.value() +
            (no_where ? "" : this->build_where(current_param)) + (limit == 0 ? "" : "LIMIT " + std::to_string(limit));
    }

    std::string QueryBuilder::build_select_query(size_t& current_param) const {
        if (this->where_clauses.empty()){
            return "";
        }

        
        std::string select;
        for (const auto& i : this->select_columns) {
            if (!select.empty()) select += ", ";
            select += i;
        }

        return "SELECT " + (select.empty() ? "*" : select) + " FROM " + this->table_name.value() + this->build_where(current_param);
    }

    std::string QueryBuilder::build_where(size_t& current_param) const {
         if (this->where_clauses.empty()){
            return "";
        }
        std::string temp_value;

        for (size_t i = 0; i < this->where_clauses.size(); ++i) {
            const auto& where = this->where_clauses[i];
            
            if (i > 0) {
                temp_value += " " + this->where_clauses[i-1].compare_with + " ";
            }

            if (where.query_builder) {
                temp_value += where.column + " " + where.where_operator + " (" + where.query_builder->build_select_query(current_param) + ")";
                continue;
            }

            if (where.function_builder) {
                temp_value += where.column + " " + where.where_operator + " " + where.function_builder.value().build_function(current_param);
                continue;
            }

            temp_value += where.column + " " + where.where_operator + " $" + std::to_string(current_param++);
        }

        return " WHERE " + temp_value;
    }
}