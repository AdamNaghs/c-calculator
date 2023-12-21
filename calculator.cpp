// calculator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <stack>
#include <stdexcept>
#include <cmath> 
#include "OpToken.h"
#include "rpn.h"
#include "Function.h"

#define LAST_VALUE "!"

void parse_expr(std::string input)
{
	int found_eq = -1;
	int found_param_start = -1;
	int found_param_end = -1;
	int i = 0;
	const std::vector<tok::OpToken> empty_vec;
	std::vector<tok::OpToken> tok_vec = tok::str_to_optoks(input);

	int l = 0, r = 0;
	int l_index = -1, r_index = -1;
	bool open = false;
	for (int i = 0; i < input.size(); i++)
	{
		char c = input.at(i);
		switch (c)
		{
		case '(':
			l++;
			l_index = i;
			open = true;
			if (r > l)
			{
				std::string tmp(i, ' ');
				tmp.append("^");
				std::cerr << "Invalid input. Parenthesis not closed.\n" << input << "\n" << tmp << "\n";
				return;
			}
			break;
		case ',':
			if (l == r) // no open parenthesis
			{
				std::string tmp(i, ' ');
				tmp.append("^");
				std::cerr << "Invalid input. Unexpected comma.\n" << input << "\n" << tmp << "\n";
				return;
			}
			break;
		case ')':
			if (!open)
			{
				std::string tmp(i, ' ');
				tmp.append("^");
				std::cerr << "Invalid input. Parenthesis not opened.\n" << input << "\n" << tmp << "\n";
				return;
			}
			r++;
			r_index = i;
			open = false;
			break;
		default:
			continue;
		}
	}
	if (l > r) // only need to check left because right is checked in loop
	{
			std::string tmp(l_index, ' ');
			tmp.append("^");
			std::cerr << "Invalid input. Parenthesis not closed 1.\n" << input << "\n" << tmp << "\n";
			return;
	}

	for (tok::OpToken t : tok_vec)
	{
		if (t.IsOperator()) {
			cmn::op c = t.GetOperator();
			if (cmn::op::L_PAREN == c) found_param_start = i;
			if (cmn::op::R_PAREN == c) found_param_end = i;
			if (cmn::op::EQUAL == c)
			{
				found_eq = i;
				break;
			}
		}
		i++;
	}

	if (found_eq > -1)
	{
		// function
		if (tok_vec.at(0).GetType() != tok::FUNCTION)
		{
			std::cerr << "Invalid value before '=' (func name cannot be number or operator).\n";
			return;
		}
		std::string func_name = tok_vec.at(0).GetName();
		auto it = func::table.find(func_name);
		if ((it != func::table.end() && it->second.builtin.available) || func_name == LAST_VALUE)
		{
			std::cerr << "Invalid input. Cannot overwrite builtin function '" << func_name << "'.\n";
			return;
		}
		std::vector<tok::OpToken> expr_vec(tok_vec.begin() + found_eq + 1, tok_vec.end());
		if (found_param_start > -1)
		{
			// init function with params
			// must only be param_names
			std::vector<tok::OpToken> param_vec(tok_vec.begin() + found_param_start + 1, tok_vec.begin() + found_param_end);
			param_vec.erase(std::remove_if(param_vec.begin() + 1, param_vec.end(),
				[](tok::OpToken token) {
					cmn::op tmp = token.GetOperator();
					return token.IsOperator() && tmp && (tmp == cmn::op::COMMA);}),
				param_vec.end());
			tok::OpToken func = tok_vec.at(0);
			std::cout << "Input: " << tok::vectostr(tok_vec) << "\nParams:" << tok::vectostr(param_vec) << "\nExpr:" << tok::vectostr(expr_vec) << "\n\n";
			for (tok::OpToken param : param_vec)
			{
				auto type = param.GetType();
				if (type != tok::FUNCTION)
				{
					std::cerr << "Invalid input. All parameters must be functions (variables).\n";
					return;
				}
				if (param.GetName() == func_name)
				{
					std::cerr << "Invalid input. Parameter name cannot be same as the name of the function it is in.\n";
					return;
				}
			}
			func::table[func_name] = func::Function(func_name, param_vec, expr_vec);
		}
		else
		{
			std::cout << "\nInput: " << tok::vectostr(tok_vec) << "\nParams : " << tok::vectostr(empty_vec) << "\nExpr : " << tok::vectostr(expr_vec) << "\n";
			func::table[func_name] = func::Function(func_name, empty_vec, expr_vec);
		}
	}
	else
	{
		bool error = false;
		std::vector<tok::OpToken> tokens = tok::str_to_optoks(input);
		std::vector<tok::OpToken> collapsed = func::collapse_function(tokens, error);
		if (error)
		{
			std::cout << "Error parsing expression:'" << input << "'\n";
			return;
		}
		std::string tmp = "";
		if (collapsed != tokens)
		{
			std::string col_str = tok::vectostr(collapsed);
			tmp.append(" = " + col_str);
		}
		rpn::sort(collapsed);
		cmn::value n = rpn::eval(collapsed);
		func::table[LAST_VALUE] = func::Function(LAST_VALUE, empty_vec, { tok::OpToken(n) });
		std::cout << n << " = " << input << tmp << "\n";
	}
}

bool test_parse_and_rpn(const std::string& input, const std::string& expected_rpn, const cmn::value expected_result)
{
	std::cout << "\nYou input: " << input << '\n';
	auto vec = tok::str_to_optoks(input);
	std::cout << "Before Sort: " << tok::vectostr(vec) << '\n' << "After RPN Sort: ";
	rpn::sort(vec);
	std::string rpn_str = tok::vectostr(vec);
	std::cout << rpn_str << '\n';
	rpn_str.erase(rpn_str.end());
	int rpn_correct = rpn_str.compare(expected_rpn);
	std::cout << "Expected RPN:\t" << expected_rpn << "\nActual RPN:\t" << rpn_str << "\n";
	cmn::value result = rpn::eval(vec);
	std::cout << "Expected Result: " << expected_result << "\nActual Result:   " << result << "\n";
	bool result_correct = result == expected_result;
	return rpn_correct == 0 && result_correct;
}

void input_loop()
{
	std::string input;
	while (1)
	{
		std::cout << "\nInput expression: ";
		std::getline(std::cin, input);
		std::cout << std::endl;
		if (input == "quit")
		{
			break;
		}
		else if (input == "table")
		{
			std::cout << func::tabletostr();
			continue;
		}
		else if (input.find("test ") == 0)
		{
			input.erase(input.begin(), input.begin() + 5);
			test_parse_and_rpn(input, "", 0);
			continue;
		}
		else if (input == "dump")
		{
			func::dump_table();
			continue;
		}
		else if (input == "debug")
		{
			rpn::debug = !rpn::debug;
			std::cout << "Debug mode " << (rpn::debug ? "enabled" : "disabled") << "\n";
			continue;
		}
		parse_expr(input);
	}
}

#define check_first_param_type(vec) if (vec[0][0].GetType() == tok::FUNCTION && func::table.find(vec[0][0].GetName()) == func::table.end()) {return vec[0][0];}
static void load_builtin_functions(void)
{
	func::add_builtin_func("pi", 0, [](std::vector<std::vector<tok::OpToken>> s)
		{
			return tok::OpToken(3.14159265359);
		});
	func::add_builtin_func("e", 0, [](std::vector<std::vector<tok::OpToken>> s)
		{
			return tok::OpToken(2.718281828459045);
		});
	func::add_builtin_func("cos", 1, [](std::vector<std::vector<tok::OpToken>> s)
		{
			check_first_param_type(s);
			rpn::sort(s[0]);
			return tok::OpToken((cmn::value)std::cos(rpn::eval(s[0])));
		});
	func::add_builtin_func("sin", 1, [](std::vector<std::vector<tok::OpToken>> s)
		{
			check_first_param_type(s);
			rpn::sort(s[0]);
			return tok::OpToken((cmn::value)std::sin(rpn::eval(s[0])));
		});
	func::add_builtin_func("tan", 1, [](std::vector<std::vector<tok::OpToken>> s)
		{
			check_first_param_type(s);
			rpn::sort(s[0]);
			return tok::OpToken((cmn::value)std::tan(rpn::eval(s[0])));
		});
	func::add_builtin_func("log", 2, [](std::vector<std::vector<tok::OpToken>> s)
		{
			check_first_param_type(s);
			rpn::sort(s[0]);
			rpn::sort(s[1]);
			if (s[2][0].GetType() == tok::FUNCTION && func::table.find(s[2][0].GetName()) == func::table.end())return s[2][0];
			return tok::OpToken((cmn::value)log(rpn::eval(s[0])) / log(rpn::eval(s[1])));
		});
	func::add_builtin_func("ln", 1, [](std::vector<std::vector<tok::OpToken>> s)
		{
			check_first_param_type(s);
			rpn::sort(s[0]);
			return tok::OpToken((cmn::value)log(rpn::eval(s[0])));
		});
	func::add_builtin_func("sqrt", 1, [](std::vector<std::vector<tok::OpToken>> s)
		{
			check_first_param_type(s);
			for (auto& v : s)
			{
				rpn::sort(v);
				v[0] = tok::OpToken(rpn::eval(v));
			}
			return tok::OpToken(powl(s[1][0].GetValue(), (double)1 / s[0][0].GetValue()));
		});
	func::add_builtin_func("root", 2, [](std::vector<std::vector<tok::OpToken>> s)
		{
			check_first_param_type(s);
			for (auto& v : s)
			{
				rpn::sort(v);
				v[0] = tok::OpToken(rpn::eval(v));
			}
			return tok::OpToken(powl(s[1][0].GetValue(), (double)1 / s[0][0].GetValue()));
		});
	// takes range begin, end, sole variable name, expression;
	func::add_builtin_func("sum", 4, [](std::vector<std::vector<tok::OpToken>> s)
		{
			check_first_param_type(s);
			s[0][0] = tok::OpToken(rpn::eval(s[0]));
			s[1][0] = tok::OpToken(rpn::eval(s[1]));
			std::vector<size_t> idxs;
			std::vector<tok::OpToken> expr_vec = s[3];
			for (int i = 0; i < s[3].size(); i++)
			{
				if (s[3][i].GetType() == tok::FUNCTION && s[2][0].GetType() == tok::FUNCTION && s[3][i].GetName() == s[2][0].GetName()) idxs.emplace_back(i);
			}
			cmn::value ret = 0;
			for (size_t n = ((size_t)s[0][0].GetValue()); n < ((size_t)s[1][0].GetValue()); n++)
			{
				auto expr_copy = expr_vec;
				for (size_t idx : idxs)
					expr_copy[idx] = tok::OpToken((cmn::value)n);
				bool error = false;
				expr_copy = func::collapse_function(expr_copy, error);
				if (error)
				{
					return tok::OpToken(0);
				}
				rpn::sort(expr_copy);
				ret += rpn::eval(expr_copy);
			}
			return tok::OpToken(ret);
		});
	func::add_builtin_func("list", 4, [](std::vector<std::vector<tok::OpToken>> s)
		{
			check_first_param_type(s);
			rpn::sort(s[0]);
			rpn::sort(s[1]);
			rpn::sort(s[2]);
			s[0][0] = tok::OpToken(rpn::eval(s[0]));
			s[1][0] = tok::OpToken(rpn::eval(s[1]));
			std::vector<size_t> idxs;
			std::vector<tok::OpToken> expr_vec = s[3];
			for (int i = 0; i < s[3].size(); i++)
			{
				if (s[3][i].GetType() == tok::FUNCTION && s[3][i].GetName() == s[2][0].GetName()) idxs.emplace_back(i);
			}
			cmn::value ret = 0;
			std::cout << "{\n";
			for (size_t n = ((size_t)s[0][0].GetValue()); n < ((size_t)s[1][0].GetValue()); n++)
			{
				auto expr_copy = expr_vec;
				for (size_t idx : idxs)
					expr_copy[idx] = tok::OpToken((cmn::value)n);
				bool error = false;
				auto collapse = func::collapse_function(expr_copy, error);
				if (error)
				{
					return tok::OpToken(0);
				}
				rpn::sort(collapse);
				ret = rpn::eval(collapse);
				std::cout << "(x:" << n << ", y:" << ret << "),\n";
			}
			std::cout << "\b}\n";
			return tok::OpToken(ret);
		});
}

int main(void)
{
	load_builtin_functions();
	parse_expr("sum(0,1,2,add(1,1))");
	parse_expr("sum(1,10,n,ln(n))");     
	parse_expr("sum(1,10,n+1,ln(n))");     
	parse_expr("!");
	parse_expr("hypot_len(a,b) = root(2,a^2 + b^2)");
	parse_expr("hypot_len(3,4)");
	parse_expr("hypot_len(1,pi)");
	parse_expr("add(x,y) = x + y");
	parse_expr("add(1,add(1,1))");
	parse_expr("x(y) = y");
	parse_expr("x");
	parse_expr("x = y");
	parse_expr("y = x");
	parse_expr("x");
	parse_expr("list(0, 100, n, ln(n))");
	parse_expr("pi * cos(0)");
	// Test cases with expected RPN and expected solution (using doubles)
	std::cout << test_parse_and_rpn("((2 + 1) * 3)", "2 1 + 3 *", 9.0) << " TEST END\n\n";
	std::cout << test_parse_and_rpn("(4 + (13 / 5))", "4 13 5 / +", 6.6) << " TEST END\n\n"; // Assuming floating-point division
	std::cout << test_parse_and_rpn("((10 * (6 / ((9 + 3) * -11))) + 17) + 5", "10 6 9 3 + -11 * / * 17 + 5 +", 21.5455) << " TEST END\n\n"; // Assuming floating-point division
	std::cout << test_parse_and_rpn("1 + 2", "1 2 +", 3.0) << " TEST END\n\n";
	std::cout << test_parse_and_rpn("3 + 4 * 2", "3 4 2 * +", 11.0) << " TEST END\n\n";
	std::cout << test_parse_and_rpn("(1 + (2 * (3 - 4)))", "1 2 3 4 - * +", -1.0) << " TEST END\n\n";
	std::cout << test_parse_and_rpn("(8 / (3 - -3))", "8 3 -3 - /", 1.3333) << " TEST END\n\n"; // Assuming floating-point division
	std::cout << test_parse_and_rpn("((3 + (5 - 2)) * (4 / 2))", "3 5 2 - + 4 2 / *", 12.0) << " TEST END\n\n";
	std::cout << test_parse_and_rpn("(2 ^ 3 * 4)", "2 3 ^ 4 *", 32.0) << " TEST END\n\n";
	std::cout << test_parse_and_rpn("((7 + 3) * (2 - 5) / 4)", "7 3 + 2 5 - * 4 /", -7.5) << " TEST END\n\n"; // Assuming floating-point division
	std::cout << test_parse_and_rpn("(6 + 10 - 4) / (1 + 1 * 2) + 1", "6 10 + 4 - 1 1 2 * + / 1 +", 5.0) << " TEST END\n\n"; // Assuming floating-point 
	std::cout << test_parse_and_rpn("1", "1", 1) << " TEST END\n";
	std::cout << test_parse_and_rpn("-1", "-1", -1) << " TEST END\n";
	std::cout << test_parse_and_rpn("word1 + s(s) + 1", "", 0);
	std::cout << test_parse_and_rpn("test + sin(cos,1)", "", 0);
	std::cout << test_parse_and_rpn("var = 15", "", 0);
	std::cout << test_parse_and_rpn("var , 15", "", 0);
	std::cout << test_parse_and_rpn("1s5", "", 0);
	std::cout << test_parse_and_rpn("1 * s5", "", 0);
	parse_expr("var(x,y,z) = 15+x");
	parse_expr("Func(x) = x^3");
	parse_expr("var(x) = 15+x");
	parse_expr("Larc(x) = x*Func(x/Func(x))");
	parse_expr("Larc(x) = x * Func (x)");
	parse_expr("vec(1)");
	parse_expr("1s5");
	parse_expr("vec = 1 + x");
	parse_expr("x = 1 + vec");
	parse_expr("vec");
	parse_expr("Larc(3)");
	parse_expr("sum(0,2,x)");
	parse_expr("c(s = 1");
	parse_expr("c((s) = 1");
	parse_expr("c((s");
	parse_expr("c))");
	input_loop();

	//func::dump_table();
	return 0;
}
