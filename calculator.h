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
#include "MessageWindow.h"

#define LAST_VALUE "!"
#define WINDOW_WIDTH 3000
#define WINDOW_HEIGHT 2000
#define MW_WIDTH 1000
#define MW_HEIGHT 2000
#define MW_X 0
#define MW_Y 0
#define GRAPH_WIDTH 2000
#define GRAPH_HEIGHT 2000
#define GRAPH_X 1000
#define GRAPH_Y 0
#define MAX_INPUT_CHARS 256
#define TARGET_FPS 120

void empty_func(void) {}
class Calculator
{
private:
	std::vector<std::string> history;
	std::vector<std::string> future;
	mw::MessageWindow& message_window = mw::MessageWindow::getInstance();
	std::streambuf* coutBuf;

public:
	Calculator() 
	{
		message_window.set_loc(MW_X, MW_Y);
		message_window.set_width(MW_WIDTH);
		message_window.set_height(MW_HEIGHT);
	}
	~Calculator() 
	{		

	}
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
					mw::MessageWindow::getInstance().print("Invalid input. Parenthesis not closed.\n" + input + "\n" + tmp + "\n");
					return;
				}
				break;
			case ',':
				if (l == r) // no open parenthesis
				{
					std::string tmp(i, ' ');
					tmp.append("^");
					std::cerr << "Invalid input. Unexpected comma.\n" << input << "\n" << tmp << "\n";
					mw::MessageWindow::getInstance().print("Invalid input. Unexpected comma.\n" + input + "\n" + tmp + "\n");
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
				mw::MessageWindow::getInstance().print("Invalid input. Parenthesis not closed 1.\n" + input + "\n" + tmp + "\n");
				return;
			}
			else
			{
				std::string tmp(r_index, ' ');
				tmp.append("^");
				std::cerr << "Invalid input. Parenthesis not opened 1.\n" << input << "\n" << tmp << "\n";
				mw::MessageWindow::getInstance().print("Invalid input. Parenthesis not opened 1.\n" + input + "\n" + tmp + "\n");
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
				mw::MessageWindow::getInstance().print("Invalid value before '=' (func name cannot be number or operator).\n");
				return;
			}
			std::string func_name = tok_vec.at(0).GetName();
			auto it = func::table.find(func_name);
			if ((it != func::table.end() && it->second.builtin.available) || func_name == LAST_VALUE)
			{
				std::cerr << "Invalid input. Cannot overwrite builtin function '" << func_name << "'.\n";
				mw::MessageWindow::getInstance().print("Invalid input. Cannot overwrite builtin function '" + func_name + "'.\n");
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
				mw::MessageWindow::getInstance().print("Input: " + tok::vectostr(tok_vec) + "\nParams:" + tok::vectostr(param_vec) + "\nExpr:" + tok::vectostr(expr_vec) + "\n\n");
				for (tok::OpToken param : param_vec)
				{
					auto type = param.GetType();
					if (type != tok::FUNCTION)
					{
						std::cerr << "Invalid input. All parameters must be functions (variables).\n";
						mw::MessageWindow::getInstance().print("Invalid input. All parameters must be functions (variables).\n");
						return;
					}
					if (param.GetName() == func_name)
					{
						std::cerr << "Invalid input. Parameter name cannot be same as the name of the function it is in.\n";
						mw::MessageWindow::getInstance().print("Invalid input. Parameter name cannot be same as the name of the function it is in.\n");
						return;
					}
				}
				func::table[func_name] = func::Function(func_name, param_vec, expr_vec);
			}
			else
			{
				std::cout << "\nInput: " << tok::vectostr(tok_vec) << "\nParams : " << tok::vectostr(empty_vec) << "\nExpr : " << tok::vectostr(expr_vec) << "\n";
				mw::MessageWindow::getInstance().print("Input: " + tok::vectostr(tok_vec) + "\nParams:" + tok::vectostr(empty_vec) + "\nExpr:" + tok::vectostr(expr_vec) + "\n\n");
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
				mw::MessageWindow::getInstance().print("Error parsing expression:'" + input + "'\n");
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
			message_window.print(std::to_string(n) + " = " + input + tmp);
		}
	}
	// window and terminal input
	std::string get_input(std::function<void()> optional_func = empty_func)
	{
		int cursor = 0;
		int cursory = 0;
		std::string ret;
		std::cout << "\nInput expression: ";
		volatile int key;
		if (window_open)
			while (1)
			{
				if (_kbhit())
				{
					char ch = _getch();

					if (ch == '\r') // Enter key pressed
					{
						std::cout << "\n";
						history.push_back(ret);
						message_window.add_message(ret);
						message_window.replace_back(" ");
						return ret;
					}

					if (ch == '\b') // Backspace pressed
					{
						if (!ret.empty() && cursor > 0)
						{
							ret.erase(cursor - 1, 1);
							cursor--;
							// Redraw the entire line
							std::cout << "\rInput expression: " << ret << ' ';
							std::cout << "\rInput expression: " << std::string(ret.begin(), ret.begin() + cursor);
							message_window.replace_back(ret);
						}
					}
					else if (ch == -32) // Special keys (like arrow keys)
					{
						int arrow = _getch(); // Get the second character

						switch (arrow)
						{
						case 72: // Up arrow
							if (!history.empty())
							{
								future.push_back(ret);
								std::cout << "\rInput expression: " << std::string(ret.length() * 2, ' ') << std::string(ret.length() * 2, '\b'); // Clear the line
								ret = history.back();
								history.pop_back();
								// Redraw the entire line
								// Move cursor back to correct position
								cursor = (int)ret.size();
								std::cout << "\rInput expression: " << std::string(ret.begin(), ret.end());
								message_window.replace_back(ret);
								message_window.set_cursor(cursor);
							}
							break;
						case 80: // Down arrow
						{
							// Save the current length for clearing the line
							int oldLength = (int) ret.length() * 2;

							if (!future.empty()) {
								history.push_back(ret);
								ret = future.back();
								future.pop_back();
							}
							else {
								ret.clear(); // Clear the current input if there's nothing in the future
							}

							cursor = (int)ret.size(); // Set cursor to the end of the new string
							int max = std::max(oldLength, (int)ret.length() * 2);

							// Clear the line: Print enough spaces to cover the old string and move cursor back
							std::cout << "\rInput expression: " << std::string(max, ' ') << std::string(max, '\b'); // Clear the line

							// Redraw the line with the new input
							std::cout << "\rInput expression: " << ret;
							// Redraw the entire line with the new input
							message_window.replace_back(ret);
							message_window.set_cursor(cursor);
							break;
						}
						case 75: // Left arrow
							if (cursor > 0)
							{
								cursor--;
								std::cout << "\b";
								message_window.set_cursor(cursor);
							}
							break;
						case 77: // Right arrow
							if (cursor < ret.size())
							{
								std::cout << ret[cursor];
								cursor++;
								message_window.set_cursor(cursor);
							}
							break;
						}
					}
					else if (ch >= 32 && ch <= 126) // Printable characters
					{
						ret.insert(cursor, 1, ch);
						cursor++;
						// Redraw the entire line
						std::cout << "\rInput expression: " << ret;
						// Move cursor back to correct position
						std::cout << "\rInput expression: " << std::string(ret.begin(), ret.begin() + cursor);
						message_window.replace_back(ret);
						message_window.set_cursor(cursor);
					}
				}
				key = GetCharPressed();
				while (key > 0) {
					if (key >= 32 && key <= 125 && ret.size() < MAX_INPUT_CHARS) {
						ret.insert(cursor, 1, char(key));
						cursor++;
						// Redraw the entire line
						std::cout << "\rInput expression: " << ret;
						// Move cursor back to correct position
						std::cout << "\rInput expression: " << std::string(ret.begin(), ret.begin() + cursor);
						message_window.replace_back(ret);
						message_window.set_cursor(cursor);
					}
					key = GetCharPressed();
				}

				// Handle special keys
				if (IsKeyPressed(KEY_BACKSPACE)) {
					if (!ret.empty() && cursor > 0)
					{
						ret.erase(cursor - 1, 1);
						cursor--;
						// Redraw the entire line
						std::cout << "\rInput expression: " << ret << ' ';
						std::cout << "\rInput expression: " << std::string(ret.begin(), ret.begin() + cursor);
						message_window.replace_back(ret);
					}
				}
				else if (IsKeyPressed(KEY_ENTER)) {
					history.push_back(ret);
					std::cout << "\n";
					message_window.add_message(ret);
					message_window.replace_back(" ");

					return ret;  // Return the input when Enter is pressed

				}
				else if (IsKeyPressed(KEY_UP)) // Up arrow
				{
					if (!history.empty())
					{
						future.push_back(ret);
						std::cout << "\rInput expression: " << std::string(ret.length() * 2, ' ') << std::string(ret.length() * 2, '\b'); // Clear the line
						ret = history.back();
						history.pop_back();
						// Redraw the entire line
						// Move cursor back to correct position
						cursor =  (int) ret.size();
						std::cout << "\rInput expression: " << std::string(ret.begin(), ret.end());
						message_window.replace_back(ret);
						message_window.set_cursor(cursor);
					}
				}
				else if (IsKeyPressed(KEY_DOWN)) // Down arrow
				{
					// Save the current length for clearing the line
					int oldLength = (int) ret.length() * 2;

					if (!future.empty()) {
						history.push_back(ret);
						ret = future.back();
						future.pop_back();
					}
					else {
						ret.clear(); // Clear the current input if there's nothing in the future
					}

					cursor =(int) ret.size(); // Set cursor to the end of the new string
					int max = std::max(oldLength, (int)ret.length() * 2);

					// Clear the line: Print enough spaces to cover the old string and move cursor back
					std::cout << "\rInput expression: " << std::string(max, ' ') << std::string(max, '\b'); // Clear the line

					// Redraw the line with the new input
					std::cout << "\rInput expression: " << ret;
					// Redraw the entire line with the new input
					message_window.replace_back(ret);
					message_window.set_cursor(cursor);
				}
				else if (IsKeyPressed(KEY_LEFT)) // Left arrow
				{
					if (cursor > 0)
					{
						cursor--;
						std::cout << "\b";
						message_window.set_cursor(cursor);
					}
				}
				else if (IsKeyPressed(KEY_RIGHT)) // Right arrow
				{
					if (cursor < ret.size())
					{
						std::cout << ret[cursor];
						cursor++;
						message_window.set_cursor(cursor);
					}
				}
				update_graph();
			}
		else
		{
			std::getline(std::cin, ret);
			std::cout << "\n";
		}
		history.push_back(ret);
		return ret;
	}

	std::string get_term_input(std::function<void()> optional_func = empty_func)
	{
		int cursor = 0;
		std::string ret;
		std::cout << "\nInput expression: ";
		if (window_open)
		{
			while (1)
			{
				if (_kbhit())
				{
					char ch = _getch();

					if (ch == '\r') // Enter key pressed
					{
						std::cout << "\n";
						history.push_back(ret);
						return ret;
					}

					if (ch == '\b') // Backspace pressed
					{
						if (!ret.empty() && cursor > 0)
						{
							ret.erase(cursor - 1, 1);
							cursor--;
							// Redraw the entire line
							std::cout << "\rInput expression: " << ret << ' ';
							std::cout << "\rInput expression: " << std::string(ret.begin(), ret.begin() + cursor);
						}
					}
					else if (ch == -32) // Special keys (like arrow keys)
					{
						int arrow = _getch(); // Get the second character

						switch (arrow)
						{
						case 72: // Up arrow
							if (!history.empty())
							{
								future.push_back(ret);
								std::cout << std::string(MAX_INPUT_CHARS - ret.length(), ' ') << std::string(MAX_INPUT_CHARS - ret.length(), '\b'); // Clear the line
								ret = history.back();
								history.pop_back();
								// Redraw the entire line
								// Move cursor back to correct position
								cursor = (int) ret.size();
								std::cout << "\rInput expression: " << std::string(ret.begin(), ret.end());
							}
							break;
						case 80: // Down arrow
							if (!future.empty())
							{
								history.push_back(ret);
								std::cout << std::string(MAX_INPUT_CHARS - ret.length(), ' ') << std::string(MAX_INPUT_CHARS - ret.length(), '\b'); // Clear the line
								ret = future.back();
								future.pop_back();
								cursor =  (int) ret.size(); // Set cursor to the end of the new string
								std::cout << "\rInput expression: " << std::string(ret.begin(), ret.end());
							}
							else
							{
								ret.clear(); // Clear the current input if there's nothing in the future
								cursor = 0;
								std::cout << "\rInput expression: " << ret; // Print the new input
							}
							// Redraw the entire line with the new input
							break;
						case 75: // Left arrow
							if (cursor > 0)
							{
								cursor--;
								std::cout << "\b";
							}
							break;
						case 77: // Right arrow
							if (cursor < ret.size())
							{
								std::cout << ret[cursor];
								cursor++;
							}
							break;
						}
					}
					else if (ch >= 32 && ch <= 126) // Printable characters
					{
						ret.insert(cursor, 1, ch);
						cursor++;
						// Redraw the entire line
						std::cout << "\rInput expression: " << ret;
						// Move cursor back to correct position
						std::cout << "\rInput expression: " << std::string(ret.begin(), ret.begin() + cursor);
					}
				}
				optional_func(); // Update the graph if the window is open
			}
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
		while (1)
		{
			update_graph();
			std::string input = get_input(std::bind(&Calculator::update_graph, this));
			handle_input(input);
		}
	}

	void plot(plot::Graph graph, std::string name)
	{
		if (!window_open) start_window();
		//internal_graph.clear();	
		internal_graph = graph;
		internal_graph_name = name;
		message_window.bg_color = internal_graph.get_bgcolor();
		BeginDrawing();
		ClearBackground(internal_graph.get_bgcolor());
		internal_graph.draw();
		message_window.draw();
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

	void set_fgcolor(Color color)
	{
		internal_graph.set_fgcolor(color);
	}

	Color get_bgcolor()
	{
		return internal_graph.get_bgcolor();
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

	std::pair<double, double> get_precision()
	{
		return std::make_pair(internal_graph.precision_x(), internal_graph.precision_y());
	}

	void alternate_colors()
	{
		alternate = !alternate;
	}

	Color get_next_color() {
		if (plot_color_iter == plot_color_map.end()) {
			plot_color_iter = plot_color_map.begin();
		}
		Color color = plot_color_iter->second;
		++plot_color_iter; // Move to the next color
		return color;
	}

	bool is_alternating()
	{
		return alternate;
	}
private:
	bool alternate = false;

	bool window_open = false;

	plot::Graph internal_graph;

	std::string internal_graph_name;

	const std::map<std::string, Color> color_map = {
	{"red", RED},
	{"blue", BLUE},
	{"green", GREEN},
	{"purple", PURPLE},
	{"darkgray", DARKGRAY},
	{"lightgray", LIGHTGRAY},
	{"yellow", YELLOW},
	{"gray", GRAY},
	{"maroon", MAROON},
	{"orange", ORANGE},
	{"beige", BEIGE},
	{"magenta", MAGENTA},
	{"gold", GOLD},
	{"pink", PINK},
	{"skyblue", SKYBLUE},
	{"lime", LIME},
	{"violet", VIOLET},
	{"darkgreen", DARKGREEN},
	{"darkblue", DARKBLUE},
	{"darkbrown", DARKBROWN},
	{"darkpurple", DARKPURPLE},
	{"brown", BROWN},
	{"white", WHITE},
	{"black", BLACK},
	{"raywhite", RAYWHITE},
	{"blank", BLANK},
	};

	const std::map<std::string, Color> plot_color_map = {
	{"red", RED},
	{"blue", BLUE},
	{"green", GREEN},
	{"purple", PURPLE},
	{"yellow", YELLOW},
	{"maroon", MAROON},
	{"orange", ORANGE},
	{"magenta", MAGENTA},
	{"gold", GOLD},
	{"pink", PINK},
	{"skyblue", SKYBLUE},
	{"lime", LIME},
	{"violet", VIOLET},
	{"white", WHITE},
	};

	std::map<std::string, Color>::const_iterator& plot_color_iter = plot_color_map.begin();

	Color get_color(std::string input)
	{
		auto it = color_map.find(input);
		if (it != color_map.end()) {
			return it->second;
		}
		else {
			return internal_graph.get_bgcolor();
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
			message_window.print(func::tabletostr() + "\n");
			return;
		}
		else if (input.find("test ") == 0)
		{
			input.erase(input.begin(), input.begin() + 5);
			test_parse_and_rpn(input, "", 0);
			message_window.print("Tested '" + input + "'");
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
			message_window.print("Debug mode " + std::string(rpn::debug ? "enabled" : "disabled"));
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
		}
		else if (input.find("setaxis ") == 0)
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
				message_window.print(it->first);
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
				points.insert(std::make_pair<>(it->first.start, it->second));
				points.insert(std::make_pair<>(it->first.end, it->second));
			}
			message_window.print(std::to_string(print_map_capacity(points)));
			return;
		}
		else if (input == "clear")
		{
			internal_graph.clear();
			return;
		}
		else if (input == "alternate")
		{
			alternate_colors();
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
			std::cout << "alternate - toggle alternating plot colors\n";
			std::cout << "help - print this help message\n";
			message_window.print("Commands:\nquit - quit the program\ntable - print the function table\ntest <expr> - test the parser and RPN sort on <expr>\ndump - dump the function table\ndebug - toggle debug mode\nsetfg <color> - set the foreground color\nsetbg <color> - set the background color\nsetgrid <color> - set the grid color\nsetaxis <color> - set the axis color\ncolors - print the available colors\nclear - clear the graph\nmem - print the approximate memory usage of the graph\nalternate - toggle alternating plot colors\nhelp - print this help message\n");
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
		message_window.load_monospace(500);
	}

	void update_graph()
	{
		if (!window_open) return;
		if (WindowShouldClose()) 
		{
			CloseWindow();
			exit(0);
		}
		plot(internal_graph, internal_graph_name);

	}

};
