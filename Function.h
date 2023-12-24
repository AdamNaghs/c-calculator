#pragma once
#include <string>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <map>
#include <functional>
#include "common.h"
#include "OpToken.h"

#define MAX_DEPTH 1000

namespace func {
	// type for built-in functions
	typedef std::function <tok::OpToken(std::vector<std::vector<tok::OpToken>>)> params_func;

	class Function
	{
	public:
		// parencontents cant contain () and expression cannot contain =
		Function() : name(""), expr(), param_names() { builtin = { 0 }; }
		~Function() {}

		Function(std::string _name, size_t _num_params, params_func func) : name(_name)
		{
			builtin.available = true;
			builtin.num_params = _num_params;
			builtin.func = func;
		}

		Function(std::string _name, std::vector<tok::OpToken> _param_names, std::vector<tok::OpToken> unordered_expression) : name(_name), param_names(_param_names), expr(unordered_expression)
		{
			builtin = { 0 };
		}

		std::string GetName() { return name; }

		std::vector<tok::OpToken> GetExpr() { return expr; }

		std::vector<tok::OpToken> GetParams() { return param_names; }

		tok::OpToken run_builtin(std::vector<std::vector<tok::OpToken>> v)
		{
			if (!builtin.available) 
			{
				std::cerr << "Error: Attempt to call 'run_builtin' for non-builtin function.\n";
				mw::MessageWindow::getInstance().print("Error: Attempt to call 'run_builtin' for non-builtin function.\n");
				return tok::OpToken(0);
			}
			if (builtin.num_params == v.size())
			{
				tok::OpToken ret = builtin.func(v);
				return ret;
			}
			std::cerr << "Invalid input. Function '" << name << "' expected param count " << builtin.num_params << ", not " << v.size() << "\n";
			mw::MessageWindow::getInstance().print("Invalid input. Function '" + name + "' expected param count " + std::to_string(builtin.num_params) + ", not " + std::to_string(v.size()) + "\n");
			return tok::OpToken(0);

		}

		struct {
			bool available;
			size_t num_params;
			params_func func;
		} builtin;

	private:
		std::string name;
		std::vector<tok::OpToken> expr;
		std::vector<tok::OpToken> param_names; // variables which have names
	};

	enum return_code {
		FAILURE = 0,
		SUCCESS = 1,
		DELAY_EVAL = -1,
		INVALID_INPUT = -2,
	};

	// map to store all built-in and user functions
	extern std::map<std::string, Function> table;

	void dump_table(void);

	void add_builtin_func(std::string name, int num_params, params_func func);

	std::string tabletostr(void);

	// seperates arguments based on commas and parenthesis depth
	std::vector<std::vector<tok::OpToken>> split_args(const std::vector<tok::OpToken>& argTokens);
	
	std::vector<tok::OpToken> collapse_function(std::string input, bool& encountered_error);
	// collaspe function collapsed function calls in the expression into their values
	// so that the expression (vec of tokens) only contains numbers and operators and can be sorted and evaluated safely
	std::vector<tok::OpToken> collapse_function(std::vector<tok::OpToken>, bool& encountered_error);

	std::vector<std::vector<tok::OpToken>> collapse_function(std::vector<std::vector<tok::OpToken>> token_vecs, bool& encountered_error);
}