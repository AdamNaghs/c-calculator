#pragma once
#include <iostream>
#include <vector>
#include <stack>
#include <stdexcept>
#include <cmath> 
#include "common.h"
#include "OpToken.h"
#include "Function.h"
#include "rpn.h"
#include "Graph.h"
#include <raylib.h>
#include <chrono>
#include <future>
#include <map>

#define LAST_VALUE "!"
#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 1500

class Calculator
{
public:
	Calculator() {}
	~Calculator() { }
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
				r++;
				r_index = i;
				break;
			default:
				continue;
			}
		}
		if (l != r) // only need to check left because right is checked in loop
		{
			if (l > r)
			{
				std::string tmp(l_index, ' ');
				tmp.append("^");
				std::cerr << "Invalid input. Parenthesis not closed 1.\n" << input << "\n" << tmp << "\n";
				return;
			}
			else
			{
				std::string tmp(r_index, ' ');
				tmp.append("^");
				std::cerr << "Invalid input. Parenthesis not opened 1.\n" << input << "\n" << tmp << "\n";
				return;
			}
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
				if (!param_vec.empty())
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
	void input_loop()
	{
		auto input_future = std::async(std::launch::async, [&]() {
			std::string input;
			std::cout << "\nInput expression: ";
			std::getline(std::cin, input);
			std::cout << "\n";
			return input;
			});

		while (1)
		{
			update_graph();
			// Check if input is ready
			if (input_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
				std::string input = input_future.get();

				// Process the input
				handle_input(input);

				// Prepare for the next input
				input_future = std::async(std::launch::async, [&]() {
					std::string new_input;
					std::cout << "\nInput expression: ";
					std::getline(std::cin, new_input);
					std::cout << "\n";
					return new_input;
					});
			}

			// Sleep for a short duration to reduce CPU usage
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	void plot(plot::Graph graph, std::string name)
	{
		if (!window_open) start_window();
		internal_graph = graph;
		internal_graph_name = name;
		BeginDrawing();
		ClearBackground(WHITE);

		internal_graph.draw_axis();
		internal_graph.plot();
		// Draw points
		EndDrawing();
	}

	bool is_plotting()
	{
		return window_open;
	}

	std::pair<int, int> get_graph_size()
	{
		return std::make_pair(internal_graph.get_width(), internal_graph.get_height());
	}

	std::pair<int, int> get_x_axis()
	{
		return std::make_pair(internal_graph.get_x_start(), internal_graph.get_x_end());
	}

	std::pair<int, int> get_y_axis()
	{
		return std::make_pair(internal_graph.get_y_start(), internal_graph.get_y_end());
	}

	void add_point(int x, int y)
	{
		internal_graph.add_point(x, y);
	}

	void add_point(plot::Point p)
	{
		internal_graph.add_point(p);
	}

	void add_point(std::vector<plot::Point> v)
	{
		internal_graph.add_point(v);
	}

private:
	bool window_open = false;

	plot::Graph internal_graph;

	std::string internal_graph_name;

	const std::map<std::string, Color> color_map = {
	{"lightgray", LIGHTGRAY},
	{"gray", GRAY},
	{"darkgray", DARKGRAY},
	{"yellow", YELLOW},
	{"gold", GOLD},
	{"orange", ORANGE},
	{"pink", PINK},
	{"red", RED},
	{"maroon", MAROON},
	{"green", GREEN},
	{"lime", LIME},
	{"darkgreen", DARKGREEN},
	{"skyblue", SKYBLUE},
	{"blue", BLUE},
	{"darkblue", DARKBLUE},
	{"purple", PURPLE},
	{"violet", VIOLET},
	{"darkpurple", DARKPURPLE},
	{"beige", BEIGE},
	{"brown", BROWN},
	{"darkbrown", DARKBROWN},
	{"white", WHITE},
	{"black", BLACK},
	{"blank", BLANK},
	{"magenta", MAGENTA},
	{"raywhite", RAYWHITE}
	// Add more colors if raylib updates
	};

	Color get_color(std::string input)
	{
		auto it = color_map.find(input);
		if (it != color_map.end()) {
			return it->second;
		}
		else {
			return BLACK;
		}
	}

	void handle_input(std::string input)
	{
		if (input == "quit")
		{
			if (window_open) CloseWindow();
			exit(0);
		}
		else if (input == "table")
		{
			std::cout << func::tabletostr();
			return;
		}
		else if (input.find("test ") == 0)
		{
			input.erase(input.begin(), input.begin() + 5);
			test_parse_and_rpn(input, "", 0);
			return;
		}
		else if (input == "dump")
		{
			func::dump_table();
			return;
		}
		else if (input == "debug")
		{
			rpn::debug = !rpn::debug;
			std::cout << "Debug mode " << (rpn::debug ? "enabled" : "disabled") << "\n";
			return;
		}
		else if (input.find("setfg ") == 0)
		{
			input.erase(input.begin(), input.begin() + 6);
			Color color = get_color(input);
			internal_graph.set_fgcolor(color);
			return;
		}
		else if (input.find("setbg ") == 0)
		{
			input.erase(input.begin(), input.begin() + 6);
			Color color = get_color(input);
			internal_graph.set_bgcolor(color);
			return;
		}
		else if (input == "colors")
		{
			for (auto it = color_map.begin(); it != color_map.end(); it++)
			{
				std::cout << it->first << "\n";
			}
			return;
		}
		parse_expr(input);
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

	void start_window()
	{
		InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Graph");
		SetTargetFPS(60);
		window_open = true;
	}

	void update_graph()
	{
		if (!window_open) return;
		plot(internal_graph, internal_graph_name);

	}


};
