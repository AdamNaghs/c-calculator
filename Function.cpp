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
			s << "(Name: " << pair.first << ", Params: " << tok::vectostr(pair.second.GetParams()) << "\b, Expr: " << tok::vectostr(pair.second.GetExpr()) << "\b, Builtin Params:" << pair.second.builtin.num_params << "), \n";
		}
		s << "}\n";
		return s.str();
	}

	void dump_table(void)
	{
		for (auto it = table.begin(); it != table.end(); /* no increment here */)
		{
			if (!it->second.builtin.available)
				it = table.erase(it);
			else
				++it;
		}
	}

	std::vector<std::vector<tok::OpToken>> split_args(const std::vector<tok::OpToken>& argTokens) {
		std::vector<std::vector<tok::OpToken>> arguments;
		std::vector<tok::OpToken> currentArg;
		int parenDepth = 0;

		for (const auto& token : argTokens)
		{
			auto t_type = token.GetType();
			if (t_type == tok::TokenType::OPERATOR && token.GetOperator() == cmn::op::L_PAREN)
				parenDepth++;
			else if (t_type == tok::TokenType::OPERATOR && token.GetOperator() == cmn::op::R_PAREN)
				parenDepth--;

			if ((t_type == tok::TokenType::OPERATOR) && token.GetOperator() == cmn::op::COMMA)
			{
				if (parenDepth == 0)
				{
					if (!currentArg.empty()) 
					{
						arguments.push_back(currentArg);
						currentArg.clear();
					}
				}
				else
					currentArg.push_back(token);
			}
			else
				currentArg.push_back(token);
		}

		if (!currentArg.empty())
			arguments.push_back(currentArg);

		return arguments;
	}

	std::vector<tok::OpToken> sub_params(std::vector<tok::OpToken> expr, std::vector<tok::OpToken> param_names, std::vector<std::vector<tok::OpToken>> v)
	{
		std::vector<int> func_idxs;
		int i = 0;
		int c = 0;
		for (tok::OpToken name : param_names)
		{
			for (int i = 0; i < expr.size(); i++)
			{
				// set parameter to value
				if (expr.at(i).GetType() == tok::FUNCTION && expr.at(i).GetName() == name.GetName())
				{
					int tmp = i;
					for (tok::OpToken t : v[c])
					{
						expr.insert(expr.begin() + i, t);
						i++;
					}
					expr.erase(expr.begin() + i);
					--i;
				}
			}
			c++;
		}
		expr.insert(expr.begin(), tok::OpToken(cmn::L_PAREN));
		expr.insert(expr.end(), tok::OpToken(cmn::R_PAREN));
		return expr;
	}

	// return 0 on success, 1 on delay evaluation, -1 on failure
	int proc_func_call(std::vector<tok::OpToken>& tokens, int funcIndex) {
		if (funcIndex < 0 || funcIndex >= tokens.size()) return FAILURE;

		auto it = tokens.begin() + funcIndex;
		if (it->GetType() != tok::FUNCTION) return FAILURE;
		auto funcIt = func::table.find(it->GetName());
		if (funcIt == func::table.end()) return FAILURE;

		// Find the left parenthesis
		auto leftParenIt = std::find_if(it, tokens.end(), [](const tok::OpToken& token) {
			return (token.GetType() == tok::OPERATOR) && (token.GetOperator() == cmn::op::L_PAREN);
			});
		if (leftParenIt == tokens.end())
		{
			if (funcIt->second.builtin.num_params + funcIt->second.GetParams().size())
			{
				std::cerr << "Invalid input. Expected param count " << funcIt->second.GetParams().size() + funcIt->second.builtin.num_params << ", not 0\n";
				return INVALID_INPUT;
			}
			return FAILURE;
		}

		// Find the matching right parenthesis, accounting for nested parentheses
		int parenCount = 1;
		auto rightParenIt = std::find_if(leftParenIt + 1, tokens.end(), [&parenCount](const tok::OpToken& token) {
			if (token.GetType() == tok::OPERATOR) {
				if (token.GetOperator() == cmn::op::L_PAREN) ++parenCount;
				if (token.GetOperator() == cmn::op::R_PAREN) --parenCount;
			}
			return parenCount == 0;
			});
		if (rightParenIt == tokens.end()) return INVALID_INPUT;

		std::vector<tok::OpToken> argTokens(leftParenIt + 1, rightParenIt);
		std::vector<std::vector<tok::OpToken>> arguments = split_args(argTokens);
		if (arguments.size() != (funcIt->second.GetParams().size() + funcIt->second.builtin.num_params))
		{
			std::cerr << "Invalid input. Expected param count " << funcIt->second.GetParams().size() + funcIt->second.builtin.num_params << ", not " << arguments.size() << "\n";
			return INVALID_INPUT;
		}
		bool error = false;
		std::vector<std::vector<tok::OpToken>> collapsed = collapse_function(arguments, error);
		if (error) return INVALID_INPUT;
		if (funcIt->second.builtin.available)
		{
			tok::OpToken val = table[funcIt->second.GetName()].run_builtin(collapsed);
			if (val.GetType() == tok::VALUE) 
			{
				tokens.erase(leftParenIt, rightParenIt+1);
				tokens[funcIndex] = val;
			}
			else
			{
				return DELAY_EVAL;
			}
			return SUCCESS;
		}
		std::vector<tok::OpToken> substitutedExpr = sub_params(funcIt->second.GetExpr(), funcIt->second.GetParams(), collapsed);
		substitutedExpr = collapse_function(substitutedExpr,error);
		if (error) return INVALID_INPUT;
		//rpn::sort(substitutedExpr);
		//cmn::value val = rpn::eval(substitutedExpr);
		//substitutedExpr.clear();
		//substitutedExpr.emplace_back(val);
		tokens.erase(tokens.begin() + funcIndex, rightParenIt + 1 <= tokens.end() ? rightParenIt + 1 : tokens.end());
		//tokens.erase(it, rightParenIt + 1);
		tokens.insert(tokens.begin() + funcIndex, substitutedExpr.begin(), substitutedExpr.end());
		return SUCCESS;
	}

#define check_depth(d,func_name,ret_bool) if (d >= MAX_DEPTH) { std::cerr << "Reached MAX_DEPTH (" << MAX_DEPTH << ") in '" << func_name << "'; check for recursive variable reference.\n"; ret_bool = true;}

	// left hand slice tokenized string, i.e. without =
	// returns index of func with params or -1 on fail
	int has_function(std::vector<tok::OpToken>& v, bool& reached_depth)
	{
		if (v.empty()) return -1;
		if (v.size() == 1 && v[0].GetType() != tok::TokenType::FUNCTION) return -1;
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
				// found function with params
				return i;
			}
		}
		check_depth(i, "has_function",reached_depth);
		return -1;
	}

	std::vector<tok::OpToken> collapse_function(std::vector<tok::OpToken> tokens, bool& encountered_error)
	{
		int tmp, depth = 0;
		while ((tmp = has_function(tokens,encountered_error)) > -1 && (depth++ < MAX_DEPTH))
		{
			if (encountered_error)
				break;
			auto ret = proc_func_call(tokens, tmp);
			if ((ret == INVALID_INPUT))
			{
				encountered_error = true;
				break;
			}
			if (ret < FAILURE) break;
		}
		check_depth(depth, "collapse_function",encountered_error);
		return tokens;
	}

	std::vector<tok::OpToken> collapse_function(std::string input, bool& encountered_error)
	{
		std::vector<tok::OpToken> tokens = tok::str_to_optoks(input);
		if (tokens.empty()) return tokens;
		return collapse_function(tokens, encountered_error);
	}

	std::vector<std::vector<tok::OpToken>> collapse_function(std::vector<std::vector<tok::OpToken>> token_vecs, bool& encountered_error)
	{
		for (std::vector<tok::OpToken>& tokens : token_vecs) \
		{
			tokens = collapse_function(tokens, encountered_error);
			if (encountered_error) break;
		}
		return token_vecs;
	}

	void add_builtin_func(std::string name, int param_len, params_func func)
	{
		table[name] = Function(name, param_len, func);
	}


}