#pragma once
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <vector>
#include "common.h"

namespace tok {

enum TokenType {
    OPERATOR,
    FUNCTION,
    VALUE,
};

class OpToken {
public:
    // Constructor for values
    explicit OpToken(cmn::value v){
        data.val = v;
        type = TokenType::VALUE;
    }

    // Constructor for operators
    explicit OpToken(cmn::op o) {
        data.op = o;
        type = TokenType::OPERATOR;
    }

    explicit OpToken(std::string s)
    {
        func_name = s;
        type = TokenType::FUNCTION;
        data.op = cmn::op::NONE;
    }

    TokenType GetType() const {
        return type;
    }

    // Check if the token is an operator
    bool IsOperator() const {
        return type == TokenType::OPERATOR;
    }

    // Get the value of the token
    cmn::value GetValue() const {
        if (!IsOperator()) return data.val;
        throw std::logic_error("Token is not a value.");
    }

    // Get the operator of the token
    cmn::op GetOperator() const {
        if (IsOperator()) return data.op;
        return cmn::op::NONE;
    }

    std::string GetName() const {
        if (type == FUNCTION) return func_name;
        throw std::logic_error("Cannot get name of nameless token.");
    }

    friend std::ostream& operator<<(std::ostream& os, OpToken& token);

    bool operator==(const OpToken& other) const {
        if (type == other.type)
        {
            switch (type)
            {
			case TokenType::OPERATOR:
				return data.op == other.data.op;
			case TokenType::VALUE:
				return data.val == other.data.val;
			case TokenType::FUNCTION:
				return func_name == other.func_name;
			default:
				return false;
			}
		}
    }

    std::string toString()
    {
        std::stringstream s;
        switch (type)
        {
        case TokenType::OPERATOR:
            s << cmn::optoch(data.op);
            break;
        case TokenType::VALUE:
            s << data.val;
            break;
        case TokenType::FUNCTION:
            s << func_name;
            break;
        default:
            s << "Error converting Token to stream";
        }
        return s.str();
    }
        
private:
    enum TokenType type;
    std::string func_name = "NOT VARIABLE";
    union Data {
        cmn::value val;
        cmn::op op;

        Data() {}  // Constructor
        ~Data() {} // Destructor
    } data;

};
const OpToken func_token = OpToken("func");
const OpToken val_token = OpToken(0);
const OpToken op_token = OpToken(cmn::op::NONE);

std::vector<OpToken> str_to_optoks(std::string s);

std::string vectostr(std::vector<OpToken> v);
    
std::string vectodbgstr(std::vector<OpToken> v);

} // namespace tok
