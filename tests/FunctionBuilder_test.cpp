#include <gtest/gtest.h>
#include "db/FunctionBuilder.hpp"

using namespace database;

// Fixture para resetar o builder antes de cada teste
class FunctionBuilderTest : public ::testing::Test {
protected:
    FunctionBuilder builder;
};

// 1. Teste de construção básica
TEST_F(FunctionBuilderTest, BasicConstruction) {
    FunctionBuilder func("UPPER");
    std::string result = func.build_function();
    EXPECT_EQ(result, "UPPER()");
}

// 2. Teste de função com parâmetros estáticos
TEST_F(FunctionBuilderTest, FunctionWithStaticParams) {
    FunctionBuilder func("UPPER");
    func.add_param(FunctionParam{.value = "test", .position = 0, .comma_separated = true, .equals_needed = true});
    
    std::string result = func.build_function();
    EXPECT_EQ(result, "UPPER(test)");
}

// 3. Teste de função com múltiplos parâmetros estáticos
TEST_F(FunctionBuilderTest, FunctionWithMultipleStaticParams) {
    FunctionBuilder func("SUBSTRING");
    func.add_param(FunctionParam{.value = "text", .position = 0, .comma_separated = true})
        .add_param(FunctionParam{.value = "1", .position = 1, .comma_separated = true})
        .add_param(FunctionParam{.value = "5", .position = 2, .comma_separated = true});
    
    std::string result = func.build_function();
    EXPECT_EQ(result, "SUBSTRING(text, 1, 5)");
}

// 4. Teste de função com parâmetro dinâmico simples
TEST_F(FunctionBuilderTest, FunctionWithDynamicParam) {
    FunctionBuilder func("UPPER");
    func.add_insert_positions(FunctionParam{.value = "", .position = 0, .equals_needed = false});
    
    size_t current = 1;
    std::string result = func.build_function(current);
    EXPECT_EQ(result, "UPPER($1)");
    EXPECT_EQ(current, 2);
}

// 5. Teste de função com múltiplos parâmetros dinâmicos
TEST_F(FunctionBuilderTest, FunctionWithMultipleDynamicParams) {
    FunctionBuilder func("CONCAT");
    func.add_insert_positions(FunctionParam{.value = "", .position = 0})
        .add_insert_positions(FunctionParam{.value = "", .position = 1});
    
    size_t current = 1;
    std::string result = func.build_function(current);
    EXPECT_TRUE(result.find("CONCAT") != std::string::npos);
    EXPECT_TRUE(result.find("$1") != std::string::npos);
    EXPECT_TRUE(result.find("$2") != std::string::npos);
    EXPECT_EQ(current, 3);
}

// 6. Teste de função EXTRACT (como usado no DBManager)
TEST_F(FunctionBuilderTest, ExtractFunction) {
    FunctionBuilder func("EXTRACT");
    func.add_insert_positions(FunctionParam{.value = "EPOCH FROM", .position = 1, .equals_needed = false});
    
    size_t current = 1;
    std::string result = func.build_function(current);
    EXPECT_EQ(result, "EXTRACT(EPOCH FROM $1)");
    EXPECT_EQ(current, 2);
}

// 7. Teste de função com parâmetros estáticos e dinâmicos misturados
TEST_F(FunctionBuilderTest, MixedStaticAndDynamicParams) {
    FunctionBuilder func("COALESCE");
    func.add_param(FunctionParam{.value = "NULL", .position = 0})
        .add_param(FunctionParam{.value = "default_value", .position = 1})
        .add_insert_positions(FunctionParam{.value = "", .position = 2, .equals_needed = false});
    
    size_t current = 1;
    std::string result = func.build_function(current);
    EXPECT_TRUE(result.find("COALESCE") != std::string::npos);
    EXPECT_TRUE(result.find("NULL") != std::string::npos);
    EXPECT_TRUE(result.find("default_value") != std::string::npos);
    EXPECT_TRUE(result.find("$1") != std::string::npos);
}

// 8. Teste de função LOWER
TEST_F(FunctionBuilderTest, LowerFunction) {
    FunctionBuilder func("LOWER");
    func.add_insert_positions(FunctionParam{.value = "", .position = 0, .equals_needed = false});
    
    size_t current = 1;
    std::string result = func.build_function(current);
    EXPECT_EQ(result, "LOWER($1)");
}

// 9. Teste de função com valor sem equals_needed
TEST_F(FunctionBuilderTest, FunctionWithNoEqualsNeeded) {
    FunctionBuilder func("DATE_TRUNC");
    func.add_insert_positions(FunctionParam{.value = "'day'", .position = 0, .equals_needed = false})
        .add_insert_positions(FunctionParam{.value = "", .position = 1, .equals_needed = false});
    
    size_t current = 1;
    std::string result = func.build_function(current);
    EXPECT_TRUE(result.find("DATE_TRUNC") != std::string::npos);
    EXPECT_TRUE(result.find("'day'") != std::string::npos);
    EXPECT_TRUE(result.find("$1") != std::string::npos);
}

// 10. Teste de função com inserção fora de ordem
TEST_F(FunctionBuilderTest, UnorderedInsertPositions) {
    FunctionBuilder func("REPLACE");
    func.add_insert_positions(FunctionParam{.value = "", .position = 2, .equals_needed = false})
        .add_insert_positions(FunctionParam{.value = "", .position = 0, .equals_needed = false})
        .add_insert_positions(FunctionParam{.value = "", .position = 1, .equals_needed = false});
    
    size_t current = 1;
    std::string result = func.build_function(current);
    EXPECT_TRUE(result.find("REPLACE") != std::string::npos);
    EXPECT_TRUE(result.find("$1") != std::string::npos);
    EXPECT_TRUE(result.find("$2") != std::string::npos);
    EXPECT_TRUE(result.find("$3") != std::string::npos);
    EXPECT_EQ(current, 4);
}

// 11. Teste de função vazia
TEST_F(FunctionBuilderTest, EmptyFunction) {
    FunctionBuilder func;
    std::string result = func.build_function();
    EXPECT_EQ(result, "()");
}

// 12. Teste de função com parâmetro comma_separated = false
TEST_F(FunctionBuilderTest, ParamWithoutComma) {
    FunctionBuilder func("CAST");
    func.add_param(FunctionParam{.value = "value", .position = 0, .comma_separated = true})
        .add_param(FunctionParam{.value = " AS INTEGER", .position = 1, .comma_separated = false});
    
    std::string result = func.build_function();
    EXPECT_EQ(result, "CAST(value AS INTEGER)");
}

// 13. Teste de função com incremento de parâmetro
TEST_F(FunctionBuilderTest, ParameterIncrement) {
    FunctionBuilder func("TRIM");
    func.add_insert_positions(FunctionParam{.value = "", .position = 0});
    
    size_t current = 5;
    std::string result = func.build_function(current);
    EXPECT_EQ(result, "TRIM($5)");
    EXPECT_EQ(current, 6);
}

// 14. Teste de função com múltiplas inserções em posições específicas
TEST_F(FunctionBuilderTest, MultipleInsertionsSpecificPositions) {
    FunctionBuilder func("CASE");
    func.add_insert_positions(FunctionParam{.value = "WHEN", .position = 0, .equals_needed = false})
        .add_insert_positions(FunctionParam{.value = "THEN", .position = 1, .equals_needed = false});
    
    size_t current = 1;
    std::string result = func.build_function(current);
    EXPECT_TRUE(result.find("CASE") != std::string::npos);
    EXPECT_TRUE(result.find("WHEN") != std::string::npos);
    EXPECT_TRUE(result.find("THEN") != std::string::npos);
}

// 15. Teste chain builder (FluentAPI)
TEST_F(FunctionBuilderTest, ChainBuilder) {
    size_t current = 1;
    std::string result = FunctionBuilder("UPPER")
        .add_insert_positions(FunctionParam{.value = "", .position = 0})
        .build_function(current);
    
    EXPECT_EQ(result, "UPPER($1)");
    EXPECT_EQ(current, 2);
}
