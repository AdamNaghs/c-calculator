#include "Function.h"
#include "rpn.h"
#include <map>
#include <sstream>

namespace func {
	std::map<std::string, Function> table;

	std::string tabletostr(void)
	{
		std::stringstream s;
		s << "{\n";
		for (std::pair<std::string, Function> pair : table)
		{
			s << "(Name:" << pair.first << ",Params:" << tok::vectostr(pair.second.GetParams()) << ",Expr:" << tok::vectostr(pair.second.GetExpr()) << "),\n";
		}
		s << "}";
		return s.str();
	}

    std::vector<std::vector<tok::OpToken>> split_args(const std::vector<tok::OpToken>& argTokens) {
        std::vector<std::vector<tok::OpToken>> arguments;
        std::vector<tok::OpToken> currentArg;

        for (const auto& token : argTokens) {
            cmn::op op = token.GetOperator();
            if (op == cmn::op::COMMA) {
                if (!currentArg.empty()) {
                    arguments.push_back(currentArg);
                    currentArg.clear();
                }
            }
            else {
                currentArg.push_back(token);
            }
        }

        // Add the last argument if exists
        if (!currentArg.empty()) {
            arguments.push_back(currentArg);
        }

        return arguments;
    }

    std::vector<tok::OpToken> sub_params(std::vector<tok::OpToken> expr, std::vector<tok::OpToken> param_names, std::vector<std::vector<tok::OpToken>> v)
    {
        std::vector<int> func_idxs;
        int i = 0;
        for (tok::OpToken t : expr)
        {
            if (t.GetType() == tok::FUNCTION)
            {
                func_idxs.push_back(i);
            }
            i++;
        }
        if (func_idxs.size() != param_names.size())
        {
            std::vector<tok::OpToken> t;
            std::cerr << "Incorrent number of parameters given to 'sub_params'.";
            return t;
            throw std::runtime_error("Incorrent number of parameters give.");
        }
        int c = 0;
        for (tok::OpToken name : param_names)
        {
            for (int i = 0; i < v.size(); i++)
            {

                if (expr.at(i).GetType() == tok::FUNCTION && expr.at(i).GetName() == name.GetName())
                {
                    expr.erase(expr.begin() + i);

                    v.at(0).insert(v.at(0).begin(), tok::OpToken(cmn::L_PAREN));
                    v.at(0).insert(v.at(0).end(), tok::OpToken(cmn::R_PAREN));
                    expr.insert(expr.begin() + i, v[c].begin(), v[c].end());
                }
            }
            c++;
        }
        expr.insert(expr.begin(), tok::OpToken(cmn::L_PAREN));
        expr.insert(expr.end(), tok::OpToken(cmn::R_PAREN));
        return expr;
    }

    void proc_func_call(std::vector<tok::OpToken>& tokens, int funcIndex) {
        if (funcIndex < 0 || funcIndex >= tokens.size()) return;

        auto it = tokens.begin() + funcIndex;
        if (it->GetType() != tok::FUNCTION) return;
        auto funcIt = func::table.find(it->GetName());
        if (funcIt == func::table.end()) return;

        // Find the left parenthesis
        auto leftParenIt = std::find_if(it, tokens.end(), [](const tok::OpToken& token) {
            return (token.GetType() == tok::OPERATOR) && (token.GetOperator() == cmn::op::L_PAREN);
            });
        if (leftParenIt == tokens.end()) return;

        // Find the matching right parenthesis, accounting for nested parentheses
        int parenCount = 1;
        auto rightParenIt = std::find_if(leftParenIt + 1, tokens.end(), [&parenCount](const tok::OpToken& token) {
            if (token.GetType() == tok::OPERATOR) {
                if (token.GetOperator() == cmn::op::L_PAREN) ++parenCount;
                if (token.GetOperator() == cmn::op::R_PAREN) --parenCount;
                //std::cout << cmn::optoch(token.GetOperator()) << ' ';
            }
            return parenCount == 0;
            });
        if (rightParenIt == tokens.end()) return;

        std::vector<tok::OpToken> argTokens(leftParenIt, rightParenIt);
        std::vector<std::vector<tok::OpToken>> arguments = split_args(argTokens);
        auto collapsed = collapse_function(arguments);
        if (funcIt->second.builtin.available)
        {
            
            cmn::value val = table[funcIt->second.GetName()].run_builtin(collapsed);
            tokens.erase(leftParenIt, rightParenIt);
            tokens[funcIndex] = tok::OpToken(val);
            return;
        }
        std::vector<tok::OpToken> substitutedExpr = sub_params(funcIt->second.GetExpr(), funcIt->second.GetParams(), collapsed);

        //tokens.erase(it, rightParenIt + 1);
        tokens.insert(tokens.begin() + funcIndex, substitutedExpr.begin(), substitutedExpr.end());
    }

    // left hand slice tokenized string, i.e. without =
    // returns index of func with params or -1 on fail
    int has_function(std::vector<tok::OpToken>& v)
    {
        int i;
        for (i = 0; i < v.size() && (i < MAX_DEPTH); i++)
        {
            if (v[i].GetType() == tok::TokenType::FUNCTION)
            {
                auto it = func::table.find(v[i].GetName());
                if (it == func::table.end()) return -1;
                Function funct = it->second;
                if (it->second.GetParams().empty() && (it->second.builtin.num_params == 0))
                {
                    auto tmp = it->second.GetExpr();
                    if ((funct.builtin.available && (funct.builtin.num_params == 0)))
                    {
                        std::vector<std::vector<tok::OpToken>> empty_vec;
                        tmp.emplace_back(v[i] = tok::OpToken(funct.run_builtin(empty_vec)));
                    }
                    tmp.insert(tmp.begin(), tok::OpToken(cmn::L_PAREN));
                    tmp.insert(tmp.end(), tok::OpToken(cmn::R_PAREN));
                    v.erase(v.begin() + i);
                    v.insert(v.begin() + i, tmp.begin(), tmp.end());
                    i++;
                }
                else if (funct.builtin.available)
                {
                    auto it = v.begin() + i;
                    if (it->GetType() != tok::FUNCTION) return i + 1;
                    auto funcIt = func::table.find(it->GetName());
                    if (funcIt == func::table.end()) return i + 1;

                    // Find the left parenthesis
                    auto leftParenIt = std::find_if(it, v.end(), [](const tok::OpToken& token) {
                        return (token.GetType() == tok::OPERATOR) && (token.GetOperator() == cmn::op::L_PAREN);
                        });
                    if (leftParenIt == v.end()) return i + 1;

                    // Find the matching right parenthesis, accounting for nested parentheses
                    int parenCount = 1;
                    auto rightParenIt = std::find_if(leftParenIt + 1, v.end(), [&parenCount](const tok::OpToken& token) {
                        if (token.GetType() == tok::OPERATOR) {
                            if (token.GetOperator() == cmn::op::L_PAREN) ++parenCount;
                            if (token.GetOperator() == cmn::op::R_PAREN) --parenCount;
                            //std::cout << cmn::optoch(token.GetOperator()) << ' ';
                        }
                        return parenCount == 0;
                        });
                    if (rightParenIt == v.end()) return i + 1;


                    std::vector<tok::OpToken> vec(v.begin() + i, v.end());
                    std::vector<tok::OpToken> ret;
                    if (rightParenIt != v.end() && leftParenIt != v.end())
                    {
                        std::vector<tok::OpToken> argsTokens(leftParenIt+1, rightParenIt);
                        std::vector<std::vector<tok::OpToken>> arguments = split_args(argsTokens);
                        auto collapsed = collapse_function(arguments);
                        v.erase(leftParenIt-1, rightParenIt+1);
                        ret.emplace_back(tok::OpToken(cmn::L_PAREN));
                        ret.emplace_back(tok::OpToken(funct.run_builtin(collapsed)));
                        ret.emplace_back(tok::OpToken(cmn::R_PAREN));
                        v.insert(v.begin() + i, ret.begin(), ret.end());
                    }
                }
                // found function with params
                return ++i;
            }
        }
        if (i >= MAX_DEPTH) std::cerr << "\nReached MAX_DEPTH (" << MAX_DEPTH << ") in 'has_function'; check for recursive variable access.\n";
        return -1;
    }

#define COLLAPSE_DEF \
do { \
    int tmp, depth = 0; \
    while ((tmp = has_function(tokens)) != -1 && (depth++ < MAX_DEPTH)) proc_func_call(tokens, tmp); \
    if (depth >= MAX_DEPTH) std::cerr << "\nReached MAX_DEPTH (" << MAX_DEPTH << ") in 'collapse_function'; check for recursive variable access.\n"; \
    return tokens;\
} while(0)

    std::vector<tok::OpToken> collapse_function(std::string input)
    {
        std::vector<tok::OpToken> tokens = tok::str_to_optoks(input);
        if (tokens.empty()) return tokens;
        int tmp, depth = 0; 
        while (((tmp = has_function(tokens)) != -1) && (depth++ < MAX_DEPTH))
        {
            proc_func_call(tokens, tmp);
        }
        if (depth >= MAX_DEPTH) std::cerr << "\nReached MAX_DEPTH (" << MAX_DEPTH << ") in 'collapse_function'; check for recursive variable access.\n"; 
        return tokens;
}

    std::vector<tok::OpToken> collapse_function(std::vector<tok::OpToken> tokens)
    {
        int tmp, depth = 0;
        while ((tmp = has_function(tokens)) != -1 && (depth++ < MAX_DEPTH)) 
            proc_func_call(tokens, tmp);
        if (depth >= MAX_DEPTH) std::cerr << "\nReached MAX_DEPTH (" << MAX_DEPTH << ") in 'collapse_function'; check for recursive variable access.\n";
        return tokens;
    }

    std::vector<std::vector<tok::OpToken>> collapse_function(std::vector<std::vector<tok::OpToken>> token_vecs)
    {
        for (std::vector<tok::OpToken>& tokens :token_vecs){
            int tmp, depth = 0;
            while ((tmp = has_function(tokens)) != -1 && (depth++ < MAX_DEPTH)) 
                proc_func_call(tokens, tmp);
            if (depth >= MAX_DEPTH) std::cerr << "\nReached MAX_DEPTH (" << MAX_DEPTH << ") in 'collapse_function'; check for recursive variable access.\n";
        }
        return token_vecs;
    }

    void add_builtin_func(std::string name, int param_len, cmn::value (*func_ptr)(std::vector<std::vector<tok::OpToken>>))
    {
        table[name] = Function(name, param_len, func_ptr);
    }

}