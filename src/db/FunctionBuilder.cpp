#include "db/FunctionBuilder.hpp"

#include <algorithm>

namespace database {
    FunctionBuilder::FunctionBuilder(std::string function_name) {
        this->function = function_name;
    }
    FunctionBuilder::FunctionBuilder(){}

    FunctionBuilder& FunctionBuilder::add_param(FunctionParam param){
        this->params.push_back(std::move(param));
        return *this;
    }

    FunctionBuilder& FunctionBuilder::add_insert_positions(FunctionParam position){
        this->insert_in_positions.push_back(std::move(position));
        return *this;
    }

    std::string FunctionBuilder::build_function() const {
        std::string func;

        for(const auto& param : this->params){
            if (!func.empty()) func += param.comma_separated ? ", " : "";
            func += param.value;
        }

        return this->function + "(" + func + ")";
    }

    std::string FunctionBuilder::build_function(size_t& current) const {
        std::sort(this->insert_in_positions.begin(), 
              this->insert_in_positions.end(), 
              [](const FunctionParam& a, const FunctionParam& b) {
                  return a.position < b.position; 
              });

        std::vector<FunctionParam> temp = this->params;
        if (!this->insert_in_positions.empty()) {
            for (const auto& pos : this->insert_in_positions){
                    std::string insert = "";
                    if (pos.value != ""){
                        insert = pos.value + (pos.equals_needed ? " := $" : " $") + std::to_string(current);
                    } else {
                        insert = "$" + std::to_string(current);
                    }
                    if (pos.position >= temp.size()) {
                        temp.push_back(FunctionParam{.value = insert});
                    } else {
                        temp.insert(temp.begin() + pos.position, FunctionParam{.value = insert});
                    }
                ++current;
            }
        }
    
        std::string func;

        for(const auto& param : temp){
            if (!func.empty()) func += ", ";
            func += param.value;
        }

        return this->function + "(" + func + ")";
    
    }
}