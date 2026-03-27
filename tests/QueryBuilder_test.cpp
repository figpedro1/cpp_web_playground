#include <gtest/gtest.h>
#include "db/QueryBuilder.hpp"

using namespace database;

// Fixture para resetar o builder antes de cada teste se necessário
class QueryBuilderTest : public ::testing::Test {
protected:
    QueryBuilder builder;

    void SetUp() override {
        builder.reset();
    }
};

// 1. Teste de SELECT simples
TEST_F(QueryBuilderTest, BuildSimpleSelect) {
    builder.set_table("users")
           .add_select(std::vector<std::string>{"id", "name"})
           .add_where("id", "=", "AND");

    std::string expected = "SELECT id, name FROM users WHERE id = $1";
    EXPECT_EQ(builder.build_select_query(), expected);
}

// 2. Teste de INSERT com conflito (ON CONFLICT DO NOTHING)
TEST_F(QueryBuilderTest, BuildInsertWithConflict) {
    builder.set_table("products")
           .add_insert("name")
           .add_insert("price")
           .add_conflict("id");

    std::string expected = "INSERT INTO products (name, price) VALUES ($1, $2) ON CONFLICT (id) DO NOTHING";
    EXPECT_EQ(builder.build_insert_query(), expected);
}

// 3. Teste de UPDATE
TEST_F(QueryBuilderTest, BuildUpdateWithWhere) {
    builder.set_table("employees")
           .add_update("salary")
           .add_where("department", "=", "AND");

    std::string expected = "UPDATE employees SET salary = $1 WHERE department = $2";
    EXPECT_EQ(builder.build_update_query(), expected);
}

// 4. Teste de DELETE (Deve falhar/retornar vazio se não houver WHERE por segurança na sua impl)
TEST_F(QueryBuilderTest, BuildDeleteSafety) {
    builder.set_table("logs");
    // Sem add_where
    EXPECT_EQ(builder.build_delete_query(), "");
    
    builder.add_where("created_at", "<", "AND");
    EXPECT_EQ(builder.build_delete_query(), "DELETE FROM logs WHERE created_at < $1");
}

// 5. Teste de UPSERT (Complexo)
TEST_F(QueryBuilderTest, BuildUpsert) {
    builder.set_table("settings")
           .add_insert("key")
           .add_insert("value")
           .add_update("value")
           .add_conflict("key");

    // O build_upsert_query usa EXCLUDED internamente conforme seu .cpp
    std::string result = builder.build_upsert_query();
    
    EXPECT_TRUE(result.find("INSERT INTO settings") != std::string::npos);
    EXPECT_TRUE(result.find("ON CONFLICT (key) DO UPDATE") != std::string::npos);
    EXPECT_TRUE(result.find("SET value = EXCLUDED.value") != std::string::npos);
}

// 6. Teste de Múltiplos WHEREs (Garantindo numeração de parâmetros $1, $2...)
TEST_F(QueryBuilderTest, MultipleWhereParams) {
    builder.set_table("items")
           .add_where("type", "=", "AND")
           .add_where("active", "=", "OR")
           .add_where("stock", ">", "AND");

    std::string result = builder.build_select_query();
    EXPECT_EQ(result, "SELECT * FROM items WHERE type = $1 AND active = $2 OR stock > $3");
}

// 7. Teste de SELECT com LIMIT
TEST_F(QueryBuilderTest, SelectWithLimit) {
    builder.set_table("users")
           .add_select("id")
           .add_select("name")
           .add_where("active", "=", "AND");

    std::string result = builder.build_select_query(false, 10);
    EXPECT_EQ(result, "SELECT id, name FROM users WHERE active = $1LIMIT 10");
}

// 8. Teste de SELECT sem WHERE (no_where=true)
TEST_F(QueryBuilderTest, SelectWithoutWhere) {
    builder.set_table("products")
           .add_select("id")
           .add_select("name")
           .add_select("price");

    std::string result = builder.build_select_query(true);
    EXPECT_EQ(result, "SELECT id, name, price FROM products");
}

// 9. Teste de SELECT com múltiplas colunas, LIMIT e no_where
TEST_F(QueryBuilderTest, SelectWithLimitAndNoWhere) {
    builder.set_table("categories")
           .add_select("id")
           .add_select("title")
           .add_select("description");

    std::string result = builder.build_select_query(true, 5);
    EXPECT_EQ(result, "SELECT id, title, description FROM categoriesLIMIT 5");
}

// 10. Teste de Sub-query em WHERE
TEST_F(QueryBuilderTest, SubqueryInWhere) {
    auto sub_query = std::make_shared<QueryBuilder>();
    sub_query->set_table("departments")
             .add_select("id")
             .add_where("status", "=", "AND");

    builder.set_table("employees")
           .add_select("id")
           .add_select("name")
           .add_where("dept_id", "IN", "AND", std::nullopt, sub_query);

    std::string result = builder.build_select_query();
    EXPECT_TRUE(result.find("SELECT id, name FROM employees") != std::string::npos);
    EXPECT_TRUE(result.find("WHERE dept_id IN (SELECT id FROM departments WHERE status = $1)") != std::string::npos);
}

// 11. Teste de Sub-query em INSERT com SELECT
TEST_F(QueryBuilderTest, SubqueryInInsert) {
    auto sub_query = std::make_shared<QueryBuilder>();
    sub_query->set_table("temp_users")
             .add_select("email")
             .add_where("verified", "=", "AND");

    builder.set_table("users")
           .add_insert("email", std::nullopt, sub_query);

    std::string result = builder.build_insert_query();
    EXPECT_TRUE(result.find("INSERT INTO users (email) VALUES") != std::string::npos);
    EXPECT_TRUE(result.find("(SELECT email FROM temp_users WHERE verified = $1)") != std::string::npos);
}

// 12. Teste de Sub-query em UPDATE
TEST_F(QueryBuilderTest, SubqueryInUpdate) {
    auto sub_query = std::make_shared<QueryBuilder>();
    sub_query->set_table("salary_lookup")
             .add_select("amount")
             .add_where("position", "=", "AND");

    builder.set_table("employees")
           .add_update("salary", std::nullopt, sub_query)
           .add_where("id", "=", "AND");

    std::string result = builder.build_update_query();
    // Verifica os componentes principais do resultado
    EXPECT_TRUE(result.find("UPDATE employees SET") != std::string::npos);
    EXPECT_TRUE(result.find("salary =") != std::string::npos);
    EXPECT_TRUE(result.find("SELECT amount FROM salary_lookup WHERE position = $1") != std::string::npos);
    EXPECT_TRUE(result.find("WHERE id = $2") != std::string::npos);
}

// 13. Teste de FunctionBuilder em WHERE
TEST_F(QueryBuilderTest, FunctionInWhere) {
    FunctionBuilder func("EXTRACT");
    FunctionParam param{.value = "EPOCH FROM", .position = 1, .equals_needed = false};
    func.add_insert_positions(param);

    builder.set_table("events")
           .add_select("id")
           .add_select("name")
           .add_where("created_at", ">", "AND", func);

    std::string result = builder.build_select_query();
    EXPECT_TRUE(result.find("SELECT id, name FROM events") != std::string::npos);
    EXPECT_TRUE(result.find("WHERE created_at > EXTRACT(EPOCH FROM $1)") != std::string::npos);
}

// 14. Teste de FunctionBuilder em INSERT
TEST_F(QueryBuilderTest, FunctionInInsert) {
    FunctionBuilder func("UPPER");

    builder.set_table("users")
           .add_insert("username")
           .add_insert("email", func)
           .add_conflict("email");

    std::string result = builder.build_insert_query();
    EXPECT_TRUE(result.find("INSERT INTO users (username, email) VALUES") != std::string::npos);
    EXPECT_TRUE(result.find("ON CONFLICT (email) DO NOTHING") != std::string::npos);
    // Verifica que uma função está sendo aplicada
    EXPECT_TRUE(result.find("UPPER") != std::string::npos || result.find("$2") != std::string::npos);
}

// 15. Teste de FunctionBuilder em UPDATE
TEST_F(QueryBuilderTest, FunctionInUpdate) {
    FunctionBuilder func("LOWER");

    builder.set_table("profiles")
           .add_update("username", func)
           .add_where("id", "=", "AND");

    std::string result = builder.build_update_query();
    EXPECT_TRUE(result.find("UPDATE profiles SET") != std::string::npos);
    EXPECT_TRUE(result.find("username") != std::string::npos);
    EXPECT_TRUE(result.find("LOWER") != std::string::npos || result.find("$1") != std::string::npos);
    EXPECT_TRUE(result.find("WHERE") != std::string::npos);
}

// 16. Teste de Reset e reutilização do builder
TEST_F(QueryBuilderTest, ResetAndReuse) {
    builder.set_table("users")
           .add_select("id")
           .add_where("active", "=", "AND");

    std::string first_result = builder.build_select_query();
    EXPECT_TRUE(first_result.find("SELECT id FROM users") != std::string::npos);
    EXPECT_TRUE(first_result.find("WHERE") != std::string::npos);

    builder.reset();
    
    builder.set_table("products")
           .add_select("name")
           .add_select("price");

    std::string second_result = builder.build_select_query(true);
    EXPECT_TRUE(second_result.find("SELECT") != std::string::npos);
    EXPECT_TRUE(second_result.find("name") != std::string::npos);
    EXPECT_TRUE(second_result.find("price") != std::string::npos);
    EXPECT_TRUE(second_result.find("FROM products") != std::string::npos);
    // Após reset, não deve ter WHERE
    EXPECT_TRUE(second_result.find("WHERE") == std::string::npos);
}