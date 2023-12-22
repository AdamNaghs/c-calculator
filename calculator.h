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
#include <cmath>
#include <limits>
#include <functional>
#include <conio.h>




#define LAST_VALUE "!"
#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 1500
#define MAX_INPUT_CHARS 256
#define TARGET_FPS 12

void empty_func(void){}
class Calculator
{
private:
	std::vector<std::string> history;
	std::vector<std::string> future;

public:
	Calculator() {}
	~Calculator() { }
	void clear()
	{
		//history.clear();
		//future.clear();
		internal_graph.clear();
	}
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
	std::string get_input(std::function<void()> optional_func = empty_func)
	{
		int cursor = 0;
		std::string ret;
		std::cout << "\nInput expression: ";
		if (window_open)
		while(1)
		{
			if (_kbhit()) {
				char ch = _getch();

				if (ch == '\r') { // Enter key pressed
					std::cout << "\n";
					history.push_back(ret);
					return ret;
				}

				if (ch == '\b') { // Backspace pressed
					if (!ret.empty()) {
						ret.pop_back();
						std::cout << "\b \b" << std::flush; // Erase the last character on console
					}
					goto exit_loop;
				}
				else if (ch == -32) // Special keys (like arrow keys)
				{
					int arrow = _getch(); // Get the second character
					std::string tmp(cursor, '\b'),tmp1(cursor,' ');
					
					switch (arrow)
					{
					case 72: // Up arrow
						future.push_back(ret);
						if (!history.empty())
						{
							ret = history.back();
							history.pop_back();
							cursor = ret.size();
							std::cout << tmp << tmp1 << tmp << ret << std::flush;
						}
						break;
					case 80: // Down arrow
						history.push_back(ret);
						if (!future.empty())
						{
							ret = future.back();
							future.pop_back();
						}
						else
						{
							ret = "";
						}
						cursor = ret.size();
						std::cout << tmp << tmp1 << tmp << ret << std::flush;
						break;
					case 75: // Left arrow
						if (cursor > 0)
						{
							std::cout << "\b" << std::flush;
							cursor--;
						}
						break;
					case 77: // Right arrow
						if (cursor < ret.size())
						{
							std::cout << "\b" << std::flush;
							cursor++;
						}
						break;
					}
				} else if (ch >= 32 && ch <= 126) { // Printable characters
					ret.push_back(ch);
					std::cout << ch << std::flush; // Display the character
					cursor++;
				}
			}
			exit_loop:
			optional_func(); // Update the graph if the window is open
			//std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Reduce CPU usage
			// Only work on raylib windows
			// Get char pressed (unicode character) on the queue
			/*int key = GetCharPressed();*/

			// Check if more characters have been pressed on the same frame
			//while (key > 0)
			//{
			//	// NOTE: Only allow keys in range [32..125]
			//	if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS))
			//	{
			//		std::cout << char(key);
			//		ret.push_back((char)key);
			//		letterCount++;
			//	}
			//	key = GetCharPressed();  // Check next character in the queue
			//}

			//if (IsKeyPressed(KEY_BACKSPACE))
			//{
			//	letterCount--;
			//	if (letterCount < 0) letterCount = 0;
			//	std::cout << "\b \b";
			//	ret.pop_back();
			//}
			//else if (IsKeyPressed(KEY_ENTER))
			//{
			//	return ret;
			//}
		}
		else
		{
			std::getline(std::cin, ret);
			std::cout << "\n";
		}
		history.push_back(ret);
		return ret;
	}
	void input_loop()
	{
		//auto input_future = std::async(std::launch::async, [&]() {
		//	std::string input;
		//	std::cout << "\nInput expression: ";
		//	std::getline(std::cin, input);
		//	std::cout << "\n";
		//	return input;
		//	});

		while (1)
		{
			update_graph();
			// Check if input is ready
			//if (input_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			//	std::string input = input_future.get();
			std::string input = get_input(std::bind(&Calculator::update_graph, this));

			handle_input(input);

				// Prepare for the next input
			//	input_future = std::async(std::launch::async, [&]() {
			//		std::string new_input;
			//		std::cout << "\nInput expression: ";
			//		std::getline(std::cin, new_input);
			//		std::cout << "\n";
			//		return new_input;
			//		});
			//}

			// Sleep for a short duration to reduce CPU usage
			/*std::this_thread::sleep_for(std::chrono::milliseconds(10));*/
		}
	}
	void plot(plot::Graph graph, std::string name)
	{
		if (!window_open) start_window();
		internal_graph = graph;
		internal_graph_name = name;
		BeginDrawing();
		ClearBackground(internal_graph.get_bgcolor());
		internal_graph.draw();
		// Draw points
		EndDrawing();
	}

	plot::Graph get_graph()
	{
		return internal_graph;
	}

	void add_line(plot::LineSegment line)
	{
		internal_graph.add_line(line);
	}

	bool is_plotting()
	{
		return window_open;
	}

	void draw()
	{
		if (!window_open) start_window();
		BeginDrawing();
		ClearBackground(internal_graph.get_bgcolor());
		internal_graph.draw();
		// Draw points
		EndDrawing();
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

	void set_threshold(double threshold)
	{
		this->threshold = threshold;
	}

	double get_threshold()
	{
		return threshold;
	}
	std::pair<double,double> get_precision()
	{
		return std::make_pair(internal_graph.precision_x(), internal_graph.precision_y());
	}
private:
	double threshold = 0.1;

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

	unsigned long print_map_capacity(const std::map<plot::Point, Color>& map) {
		unsigned long cap = sizeof(map);
		for (auto it = map.begin(); it != map.end(); ++it) {
			cap += sizeof(it);
		}
		double sizeInMB = static_cast<double>(cap) / (1024 * 1024);
		double sizeInGB = sizeInMB / 1024;
		if (sizeInGB >= 1.0) {
			std::cout << "Approximate map size: " << sizeInGB << " GB" << std::endl;
		}
		else {
			std::cout << "Approximate map size: " << sizeInMB << " MB" << std::endl;
		}
		return cap;
	}

	void print_points_size(const std::map<plot::Point, Color>& m) {
		size_t sizeInBytes = m.size() * (sizeof(plot::Point) + sizeof(Color) + sizeof(std::map<plot::Point, Color>));

		double sizeInMB = static_cast<double>(sizeInBytes) / (1024 * 1024);
		double sizeInGB = sizeInMB / 1024;

		if (sizeInGB >= 1.0) {
			std::cout << "Approximate map size: " << sizeInGB << " GB" << std::endl;
		}
		else {
			std::cout << "Approximate map size: " << sizeInMB << " MB" << std::endl;
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
		else if (input.find("setgrid ") == 0)
		{
			input.erase(input.begin(), input.begin() + 8);
			Color color = get_color(input);
			internal_graph.set_gridcolor(color);
			return;
		}else if (input.find("setaxis ") == 0)
		{
			input.erase(input.begin(), input.begin() + 8);
			Color color = get_color(input);
			internal_graph.set_axiscolor(color);
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
		else if (input == "clear")
		{
			internal_graph.clear_points();
			return;
		}
		else if (input == "mem")
		{
			auto map = internal_graph.get_lines();
			std::map<plot::Point, Color> points;
			for (auto it = map.begin(); it != map.end(); it++)
			{
				points.insert(std::make_pair<>(it->first.start,it->second));
				points.insert(std::make_pair<>(it->first.end, it->second));
			}
			print_map_capacity(points);
			return;
		}
		else if (input == "clear")
		{
			internal_graph.clear();
			return;
		}
		else if (input == "help")
		{
			std::cout << "Commands:\n";
			std::cout << "quit - quit the program\n";
			std::cout << "table - print the function table\n";
			std::cout << "test <expr> - test the parser and RPN sort on <expr>\n";
			std::cout << "dump - dump the function table\n";
			std::cout << "debug - toggle debug mode\n";
			std::cout << "setfg <color> - set the foreground color\n";
			std::cout << "setbg <color> - set the background color\n";
			std::cout << "setgrid <color> - set the grid color\n";
			std::cout << "setaxis <color> - set the axis color\n";
			std::cout << "colors - print the available colors\n";
			std::cout << "clear - clear the graph\n";
			std::cout << "mem - print the approximate memory usage of the graph\n";
			std::cout << "help - print this help message\n";
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
		SetTargetFPS(TARGET_FPS);
		window_open = true;
	}

	void update_graph()
	{
		if (!window_open) return;
		plot(internal_graph, internal_graph_name);

	}




};


