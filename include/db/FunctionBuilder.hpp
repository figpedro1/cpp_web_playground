#pragma once

#include <string>
#include <vector>

namespace database {
    struct FunctionParam {
        std::string value;
        int position;
    };

    class FunctionBuilder {
        private:
            std::string function;
            std::vector<std::string> params;
            std::vector<FunctionParam> insert_in_positions;

        public:
            FunctionBuilder(std::string function_name);
            FunctionBuilder();
            FunctionBuilder& add_param(std::string param);
            FunctionBuilder& add_insert_positions(FunctionParam position);
            std::string build_function() const;
            std::string build_function(size_t& current) const;
    };
}