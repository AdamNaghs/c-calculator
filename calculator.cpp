// calculator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

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
#include "calculator.h"

Calculator calc;

	// should sort before running to remove parenthesis
int check_param_types(std::vector<std::vector<tok::OpToken>> vec, std::vector<tok::OpToken> types)
{
	for (int i = 0; i < std::min(types.size(), vec.size()); i++)
	{
		if (vec[i][0].GetType() != types[i].GetType())
		{
			//std::cerr << "Invalid input. Expected type " << types[i].GetType() << " at index " << i << ", not " << vec[i].GetType() << "\n";
			return i;
		}
	}
	return -1;
}
#define check_first_param_type(vec) if (vec[0][0].GetType() == tok::FUNCTION && func::table.find(vec[0][0].GetName()) == func::table.end()) {return vec[0][0];}

#define POINT_THRESHOLD 0.125 // 15% of the y-axis range

void load_builtin_functions(void)
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
			rpn::sort(s[0]);
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token }))) return s[tmp][0];
			return tok::OpToken((cmn::value)std::cos(rpn::eval(s[0])));
		});
	func::add_builtin_func("sin", 1, [](std::vector<std::vector<tok::OpToken>> s)
		{
			rpn::sort(s[0]);
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token }))) return s[tmp][0];
			return tok::OpToken((cmn::value)std::sin(rpn::eval(s[0])));
		});
	func::add_builtin_func("tan", 1, [](std::vector<std::vector<tok::OpToken>> s)
		{
			rpn::sort(s[0]);
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token }))) return s[tmp][0];
			return tok::OpToken((cmn::value)std::tan(rpn::eval(s[0])));
		});	func::add_builtin_func("acos", 1, [](std::vector<std::vector<tok::OpToken>> s)
		{
			rpn::sort(s[0]);
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token }))) return s[tmp][0];
			return tok::OpToken((cmn::value)std::acos(rpn::eval(s[0])));
		});
	func::add_builtin_func("asin", 1, [](std::vector<std::vector<tok::OpToken>> s)
		{
			rpn::sort(s[0]);
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token }))) return s[tmp][0];
			return tok::OpToken((cmn::value)std::asin(rpn::eval(s[0])));
		});
	func::add_builtin_func("atan", 1, [](std::vector<std::vector<tok::OpToken>> s)
		{
			rpn::sort(s[0]);
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token }))) return s[tmp][0];
			return tok::OpToken((cmn::value)std::atan(rpn::eval(s[0])));
		});
	func::add_builtin_func("log", 2, [](std::vector<std::vector<tok::OpToken>> s)
		{
			check_first_param_type(s);
			rpn::sort(s[0]);
			rpn::sort(s[1]);
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token, tok::val_token }))) return s[tmp][0];
			return tok::OpToken((cmn::value)log(rpn::eval(s[0])) / log(rpn::eval(s[1])));
		});
	func::add_builtin_func("ln", 1, [](std::vector<std::vector<tok::OpToken>> s)
		{
			check_first_param_type(s);
			rpn::sort(s[0]);
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token }))) return s[tmp][0];
			return tok::OpToken((cmn::value)log(rpn::eval(s[0])));
		});
	func::add_builtin_func("sqrt", 1, [](std::vector<std::vector<tok::OpToken>> s)
		{
			rpn::sort(s[0]);
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token }))) return s[tmp][0];
			check_first_param_type(s);
			return tok::OpToken(powl(s[0][0].GetValue(), (double)1 / (double)2));
		});
	func::add_builtin_func("fact", 1, [](std::vector<std::vector<tok::OpToken>> s)
		{
			rpn::sort(s[0]);
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token }))) return s[tmp][0];
			check_first_param_type(s);
			cmn::value v = rpn::eval(s[0]);
			int64_t dist = (size_t)v;
			for (int64_t i = dist - 1; i > 0; i--)
			{
				v *= (cmn::value)i;
			}
			return tok::OpToken(v);
		});
	func::add_builtin_func("root", 2, [](std::vector<std::vector<tok::OpToken>> s)
		{
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token,tok::val_token }))) return s[tmp][0];
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
			rpn::sort(s[0]);
			rpn::sort(s[1]);
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token , tok::val_token }))) return s[tmp][0]; // not checking last two params because can be any type
			cmn::value end = rpn::eval(s[1]);
			cmn::value start = rpn::eval(s[0]);
			std::vector<size_t> idxs;
			std::vector<tok::OpToken> expr_vec = s[3];
			for (int i = 0; i < s[3].size(); i++)
			{
				if (s[3][i].GetType() == tok::FUNCTION && s[2][0].GetType() == tok::FUNCTION && s[3][i].GetName() == s[2][0].GetName()) idxs.emplace_back(i);
			}
			cmn::value ret = 0;
			for (size_t n = (size_t)start; (start > end) ? (n > end) : (n < end);(start > end) ? (n--) : (n++))
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
	// takes range begin, end, sole variable name, expression;
	func::add_builtin_func("list", 4, [](std::vector<std::vector<tok::OpToken>> s)
		{
			rpn::sort(s[0]);
			rpn::sort(s[1]);
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token , tok::val_token }))) return s[tmp][0]; // not checking last two params because can be any type
			check_first_param_type(s);
			rpn::sort(s[2]);
			std::vector<size_t> idxs;
			std::vector<tok::OpToken> expr_vec = s[3];
			std::vector<tok::OpToken> var_vec = s[2];
			for (int i = 0; i < expr_vec.size(); i++)
			{
				if (expr_vec[i].GetType() == tok::FUNCTION && expr_vec[i].GetName() == var_vec[0].GetName()) idxs.emplace_back(i);
			}
			cmn::value ret = 0;
			cmn::value end = rpn::eval(s[1]);
			cmn::value start = rpn::eval(s[0]);
			std::cout << "{\n";
			mw::MessageWindow::getInstance().print("{\n");
			for (size_t n = ((size_t)start); (start > end) ? (n > end) : (n < end);(start > end) ? (n--) : (n++))
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
				mw::MessageWindow::getInstance().print("(x:" + std::to_string(n) + ", y:" + std::to_string(ret) + "),\n");
			}
			std::cout << "\b}\n";
			mw::MessageWindow::getInstance().print("\b}\n");
			return tok::OpToken(ret);
		});
	// takes x-axist start & end, y-axis start & end, sole variable name, expression;
	func::add_builtin_func("plot", 6, [](std::vector<std::vector<tok::OpToken>> s)
		{
			auto start_time = std::chrono::high_resolution_clock::now();
			for (int i = 0; i < 4; i++)
			{
				bool error = false;
				auto collapse = func::collapse_function(s[i], error);
				rpn::sort(s[i]);
				s[i][0] = tok::OpToken(rpn::eval(s[i]));
			}
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token , tok::val_token, tok::val_token, tok::val_token }))) return s[tmp][0]; // not checking last two params because can be any type
			check_first_param_type(s);
			std::vector<size_t> idxs;
			std::vector<tok::OpToken> expr_vec = s[5];
			std::vector<tok::OpToken> var_vec = s[4];
			for (int i = 0; i < expr_vec.size(); i++)
			{
				if (expr_vec[i].GetType() == tok::FUNCTION && expr_vec[i].GetName() == var_vec[0].GetName()) idxs.emplace_back(i);
			}
			cmn::value ret = 0;
			cmn::value start = s[0][0].GetValue();
			cmn::value end = s[1][0].GetValue();
			plot::Graph g(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, std::min((int)start, (int)end), std::max((int)start, (int)end), std::min((int)s[2][0].GetValue(), (int)s[3][0].GetValue()), std::max((int)s[2][0].GetValue(), (int)s[3][0].GetValue()));
			if (calc.is_alternating())
				g.set_fgcolor(calc.get_next_color());
			std::string name = "plot(";
			for (auto& v : s)
			{
				name.append(tok::vectostr(v) + ",");
			}
			name.append("\b)");
			calc.plot(g, name);
			double step = g.precision_x();
			plot::Point last;
			bool first = true;
			if (rpn::debug)
			{
				std::cout << "{\n";
				mw::MessageWindow::getInstance().print("{\n");
			}
			double y_axis_range = calc.get_graph().get_y_end() - calc.get_graph().get_y_start();
			// Set the threshold as a small percentage of the y-axis range
			const double threshold = POINT_THRESHOLD * y_axis_range; // Example: 5% of the y-axis range

			g.set_bgcolor(calc.get_bgcolor());
			#pragma omp parallel for
			for (double n = start; (start > end) ? (n > end) : (n < end); n += (start > end) ? (-step) : (step))
			{
				auto expr_copy = expr_vec;
				for (size_t idx : idxs)
				{
					expr_copy[idx] = tok::OpToken((cmn::value)n);
				}
				bool error = false;
				auto collapse = func::collapse_function(expr_copy, error);
				if (error)
				{
					return tok::OpToken(0);
				}
				rpn::sort(collapse);
				ret = rpn::eval(collapse);
				plot::Point tmp = plot::Point(n, ret);
				if (!first && (std::abs(ret - last.y) < threshold)) {
					// Check if the difference between consecutive points is below the threshold
					calc.add_line(plot::LineSegment(last, plot::Point(n, ret)));
				}
				first = false;
				last = plot::Point(n, ret);
			
				if (rpn::debug)
				{
					std::cout << "(x:" << n << ", y:" << ret << "),\n";
					mw::MessageWindow::getInstance().print("(x:" + std::to_string(n) + ", y:" + std::to_string(ret) + "),\n");
				}
			}
			if (rpn::debug)
			{
				std::cout << "\b}\n";
				mw::MessageWindow::getInstance().print("\b}\n");
			}
			auto end_time = std::chrono::high_resolution_clock::now();
			calc.draw();
			std::chrono::duration<double, std::milli> duration = end_time - start_time;
			std::cout << "Plot took " << duration.count() << "ms\n";
			mw::MessageWindow::getInstance().print("Plot took " + std::to_string(duration.count()) + "ms\n");
			return tok::OpToken(ret);
		});
	func::add_builtin_func("plot_add", 2, [](std::vector<std::vector<tok::OpToken>> s)
		{
			auto start_time = std::chrono::high_resolution_clock::now();
			std::vector<size_t> idxs;
			std::vector<tok::OpToken> expr_vec = s[1];
			std::vector<tok::OpToken> var_vec = s[0];
			for (int i = 0; i < expr_vec.size(); i++)
			{
				if (expr_vec[i].GetType() == tok::FUNCTION && expr_vec[i].GetName() == var_vec[0].GetName()) idxs.emplace_back(i);
			}
			cmn::value ret = 0;
			auto pair = calc.get_y_axis();
			cmn::value start = pair.first;
			cmn::value end = pair.second;
			double step = calc.get_precision().first;
			std::string name = "plot_add(";
			for (auto& v : s)
			{
				name.append(tok::vectostr(v) + ",");
			}
			name.append("\b)");
			plot::Point last;
			bool first = true;
			double y_axis_range = calc.get_graph().get_y_end() - calc.get_graph().get_y_start();
			// Set the threshold as a small percentage of the y-axis range
			const double threshold = POINT_THRESHOLD * y_axis_range; // Example: 5% of the y-axis range

			if (rpn::debug)
			{
				std::cout << "{\n";
				mw::MessageWindow::getInstance().print("{\n");
			}
			if (calc.is_alternating())
				calc.set_fgcolor(calc.get_next_color());
			#pragma omp parallel for
			for (cmn::value n = start; (start > end) ? (n > end) : (n < end); n += (start > end) ? (-step) : (step))
			{
				auto expr_copy = expr_vec;
				for (size_t idx : idxs)
				{
					expr_copy[idx] = tok::OpToken((cmn::value)n);
				}
				bool error = false;
				bool ran = false;
				std::vector<tok::OpToken> collapse;//= func::collapse_function(expr_copy, error);
				for (auto& v : expr_copy)
				{
					if (v.GetType() == tok::FUNCTION)
					{
						collapse = func::collapse_function(expr_copy, error);
						ran = true;
						break;
					}
				}
				if (error)
				{
					return tok::OpToken(0);
				}
				if (!ran) collapse = expr_copy;
				rpn::sort(collapse);
				ret = rpn::eval(collapse);
				plot::Point tmp = plot::Point(n, ret);
				if (!first && (std::abs(ret - last.y) < threshold)) {
					// Check if the difference between consecutive points is below the threshold
					calc.add_line(plot::LineSegment(last, tmp));
				}
				first = false;
				last = tmp;
				if (rpn::debug)
				{
					std::cout << "(x:" << n << ", y:" << ret << "),\n";
					mw::MessageWindow::getInstance().print("(x:" + std::to_string(n) + ", y:" + std::to_string(ret) + "),\n");
				}

				//calc.add_point(plot::Point(n, ret));
			}
			if (rpn::debug)
			{
				std::cout << "\b}\n";
				mw::MessageWindow::getInstance().print("\b}\n");
			}
			auto end_time = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> duration = end_time - start_time;
			std::cout << "Plot took " << duration.count() << "ms\n";
			mw::MessageWindow::getInstance().print("Plot took " + std::to_string(duration.count()) + "ms\n");
			return tok::OpToken(ret);
		});	func::add_builtin_func("plot_addr", 4, [](std::vector<std::vector<tok::OpToken>> s)
		{
			auto start_time = std::chrono::high_resolution_clock::now();
			std::vector<size_t> idxs;
			std::vector<tok::OpToken> expr_vec = s[3];
			std::vector<tok::OpToken> var_vec = s[2];
			for (int i = 0; i < expr_vec.size(); i++)
			{
				if (expr_vec[i].GetType() == tok::FUNCTION && expr_vec[i].GetName() == var_vec[0].GetName()) idxs.emplace_back(i);
			}
			for (int i = 0; i < 2; i++)
			{
				rpn::sort(s[i]);
				s[i][0] = tok::OpToken(rpn::eval(s[i]));
			}
			cmn::value ret = 0;
			cmn::value start = s[0][0].GetValue();
			cmn::value end = s[1][0].GetValue();
			double step = calc.get_precision().first;
			std::string name = "plot_addr(";
			for (auto& v : s)
			{
				name.append(tok::vectostr(v) + ",");
			}
			name.append("\b)");
			plot::Point last;
			bool first = true;
			double y_axis_range = abs(start) + abs(end);
			// Set the threshold as a small percentage of the y-axis range
			const double threshold = POINT_THRESHOLD * y_axis_range; // Example: 5% of the y-axis range

			if (rpn::debug)
			{
				std::cout << "{\n";
				mw::MessageWindow::getInstance().print("{\n");
			}
			if (calc.is_alternating())
				calc.set_fgcolor(calc.get_next_color());
			#pragma omp parallel for
			for (cmn::value n = start; (start > end) ? (n > end) : (n < end); n += (start > end) ? (-step) : (step))
			{
				auto expr_copy = expr_vec;
				for (size_t idx : idxs)
				{
					expr_copy[idx] = tok::OpToken((cmn::value)n);
				}
				bool error = false;
				bool ran = false;
				std::vector<tok::OpToken> collapse;//= func::collapse_function(expr_copy, error);
				for (auto& v : expr_copy)
				{
					if (v.GetType() == tok::FUNCTION)
					{
						collapse = func::collapse_function(expr_copy, error);
						ran = true;
						break;
					}
				}
				if (error)
				{
					return tok::OpToken(0);
				}
				if (!ran) collapse = expr_copy;
				rpn::sort(collapse);
				ret = rpn::eval(collapse);
				plot::Point tmp = plot::Point(n, ret);
				if (!first && (std::abs(ret - last.y) < threshold)) {
					// Check if the difference between consecutive points is below the threshold
					calc.add_line(plot::LineSegment(last, tmp));
				}
				first = false;
				last = tmp;
				if (rpn::debug)
				{
					std::cout << "(x:" << n << ", y:" << ret << "),\n";
					mw::MessageWindow::getInstance().print("(x:" + std::to_string(n) + ", y:" + std::to_string(ret) + "),\n");
				}

				//calc.add_point(plot::Point(n, ret));
			}
			if (rpn::debug)
			{
				std::cout << "\b}\n";
				mw::MessageWindow::getInstance().print("\b}\n");
			}
			auto end_time = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> duration = end_time - start_time;
			std::cout << "Plot took " << duration.count() << "ms\n";
			mw::MessageWindow::getInstance().print("Plot took " + std::to_string(duration.count()) + "ms\n");
			return tok::OpToken(ret);
		});

	func::add_builtin_func("plot_addx", 2, [](std::vector<std::vector<tok::OpToken>> s)
		{
			auto start_time = std::chrono::high_resolution_clock::now();
			std::vector<size_t> idxs;
			std::vector<tok::OpToken> var_vec = s[0];
			std::vector<tok::OpToken> expr_vec = s[1];
			for (int i = 0; i < expr_vec.size(); i++)
			{
				if (expr_vec[i].GetType() == tok::FUNCTION && expr_vec[i].GetName() == var_vec[0].GetName())
				{
					idxs.emplace_back(i);
				}
			}
			cmn::value ret = 0;
			auto pair = calc.get_x_axis();
			cmn::value start = pair.first;
			cmn::value end = pair.second;
			double step = calc.get_precision().first;
			std::string name = "plot_addx(";
			for (auto& v : s)
			{
				name.append(tok::vectostr(v) + ",");
			}
			name.append("\b)");
			plot::Point last;
			bool first = true;
			double x_axis_range = calc.get_graph().get_x_end() - calc.get_graph().get_x_start();
			// Set the threshold as a small percentage of the y-axis range
			const double threshold = POINT_THRESHOLD * x_axis_range; // Example: 5% of the y-axis range

			if (rpn::debug)
			{
				std::cout << "{\n";
				mw::MessageWindow::getInstance().print("{\n");
			}
			if (calc.is_alternating())
				calc.set_fgcolor(calc.get_next_color());
			#pragma omp parallel for
			for (cmn::value n = start; (start > end) ? (n > end) : (n < end); n += (start > end) ? (-step) : (step))
			{
				auto expr_copy = expr_vec;
				for (size_t idx : idxs)
				{
					expr_copy[idx] = tok::OpToken((cmn::value)n);
				}
				bool error = false;
				bool ran = false;
				std::vector<tok::OpToken> collapse;//= func::collapse_function(expr_copy, error);
				for (auto& v : expr_copy)
				{
					if (v.GetType() == tok::FUNCTION)
					{
						collapse = func::collapse_function(expr_copy, error);
						ran = true;
						break;
					}
				}
				if (error)
				{
					return tok::OpToken(0);
				}
				if (!ran) collapse = expr_copy;
				rpn::sort(collapse);
				ret = rpn::eval(collapse);
				plot::Point tmp = plot::Point(ret, n);
				if (!first && (std::abs(ret - last.x) < threshold)) {
					// Check if the difference between consecutive points is below the threshold
					
					calc.add_line(plot::LineSegment(last, tmp));
				}
				first = false;
				last = tmp;
				if (rpn::debug)
				{
					std::cout << "(x:" << n << ", y:" << ret << "),\n";
					mw::MessageWindow::getInstance().print("(x:" + std::to_string(n) + ", y:" + std::to_string(ret) + "),\n");
				}
				//calc.add_point(plot::Point(n, ret));
			}
			if (rpn::debug)
			{
				std::cout << "\b}\n";
				mw::MessageWindow::getInstance().print("\b}\n");
			}
			auto end_time = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> duration = end_time - start_time;
			std::cout << "Plot took " << duration.count() << "ms\n";
			mw::MessageWindow::getInstance().print("Plot took " + std::to_string(duration.count()) + "ms\n");
			return tok::OpToken(ret);
		});	func::add_builtin_func("plot_addxr", 4, [](std::vector<std::vector<tok::OpToken>> s)
		{
			auto start_time = std::chrono::high_resolution_clock::now();
			std::vector<size_t> idxs;
			std::vector<tok::OpToken> var_vec = s[2];
			std::vector<tok::OpToken> expr_vec = s[3];
			for (int i = 0; i < expr_vec.size(); i++)
			{
				if (expr_vec[i].GetType() == tok::FUNCTION && expr_vec[i].GetName() == var_vec[0].GetName())
				{
					idxs.emplace_back(i);
				}
			}
			for (int i = 0; i < 2; i++)
			{
				rpn::sort(s[i]);
				s[i][0] = tok::OpToken(rpn::eval(s[i]));
			}
			cmn::value ret = 0;
			cmn::value start = s[0][0].GetValue();
			cmn::value end = s[1][0].GetValue();
			double step = calc.get_precision().first;
			std::string name = "plot_addxr(";
			for (auto& v : s)
			{
				name.append(tok::vectostr(v) + ",");
			}
			name.append("\b)");
			plot::Point last;
			bool first = true;
			double x_axis_range = calc.get_graph().get_x_end() - calc.get_graph().get_x_start();
			// Set the threshold as a small percentage of the y-axis range
			const double threshold = POINT_THRESHOLD * x_axis_range; // Example: 5% of the y-axis range

			if (rpn::debug)
			{
				std::cout << "{\n";
				mw::MessageWindow::getInstance().print("{\n");
			}
			if (calc.is_alternating())
				calc.set_fgcolor(calc.get_next_color());
			#pragma omp parallel for
			for (cmn::value n = start; (start > end) ? (n > end) : (n < end); n += (start > end) ? (-step) : (step))
			{
				auto expr_copy = expr_vec;
				for (size_t idx : idxs)
				{
					expr_copy[idx] = tok::OpToken((cmn::value)n);
				}
				bool error = false;
				bool ran = false;
				std::vector<tok::OpToken> collapse;//= func::collapse_function(expr_copy, error);
				for (auto& v : expr_copy)
				{
					if (v.GetType() == tok::FUNCTION)
					{
						collapse = func::collapse_function(expr_copy, error);
						ran = true;
						break;
					}
				}
				if (error)
				{
					return tok::OpToken(0);
				}
				if (!ran) collapse = expr_copy;
				rpn::sort(collapse);
				ret = rpn::eval(collapse);
				plot::Point tmp = plot::Point(ret, n);
				if (!first && (std::abs(ret - last.x) < threshold)) {
					// Check if the difference between consecutive points is below the threshold
					
					calc.add_line(plot::LineSegment(last, tmp));
				}
				first = false;
				last = tmp;
				if (rpn::debug)
				{
					std::cout << "(x:" << n << ", y:" << ret << "),\n";
					mw::MessageWindow::getInstance().print("(x:" + std::to_string(n) + ", y:" + std::to_string(ret) + "),\n");
				}
				//calc.add_point(plot::Point(n, ret));
			}
			if (rpn::debug)
			{
				std::cout << "\b}\n";
				mw::MessageWindow::getInstance().print("\b}\n");
			}
			auto end_time = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> duration = end_time - start_time;
			std::cout << "Plot took " << duration.count() << "ms\n";
			mw::MessageWindow::getInstance().print("Plot took " + std::to_string(duration.count()) + "ms\n");
			return tok::OpToken(ret);
		});
	// takes range begin, end, step, sole variable name, expression;
	func::add_builtin_func("step", 5, [](std::vector<std::vector<tok::OpToken>> s)
		{
			check_first_param_type(s);
			for (int i = 0; i < 3; i++)
			{
				rpn::sort(s[i]);
			}
			int tmp;
			if (-1 != (tmp = check_param_types(s, { tok::val_token , tok::val_token, tok::val_token }))) return s[tmp][0]; // not checking last two params because can be any type
			cmn::value step = s[2][0].GetValue();
			if (step <= 0)
			{
				std::cerr << "Invalid input. Step must be greater than 0.\n";
				mw::MessageWindow::getInstance().print("Invalid input. Step must be greater than 0.\n");
				return tok::OpToken(0);
			}
			std::vector<size_t> idxs;
			std::vector<tok::OpToken> expr_vec = s[4];
			for (int i = 0; i < expr_vec.size(); i++)
			{
				if (expr_vec[i].GetType() == tok::FUNCTION && expr_vec[i].GetName() == s[3][0].GetName()) idxs.emplace_back(i);
			}
			cmn::value ret = 0;
			cmn::value end = rpn::eval(s[1]);
			cmn::value start = rpn::eval(s[0]);
			std::cout << "{\n";
			mw::MessageWindow::getInstance().print("{\n");
			for (double n = start; (start > end) ? (n > end) : (n < end); n += (start > end) ? (-step) : (step))
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
				mw::MessageWindow::getInstance().print("(x:" + std::to_string(n) + ", y:" + std::to_string(ret) + "),\n");
			}
			std::cout << "\b}\n";
			mw::MessageWindow::getInstance().print("\b}\n");
			return tok::OpToken(ret);
		});
}

int main(void)
{
	load_builtin_functions();
	calc.alternate_colors();
	calc.parse_expr("plot(-1 * pi,pi,-1,1,n,cos(n))");
	calc.parse_expr("plot_add(x,x)");
	calc.parse_expr("list(0,5,x,cos(x))");
	calc.parse_expr("e");
	calc.parse_expr("pi");
	calc.parse_expr("cos(pi)");
	calc.parse_expr("sin(1)");
	calc.parse_expr("tan(1)");
	calc.parse_expr("ln(e)");
	calc.parse_expr("sqrt(4)");
	calc.parse_expr("fact(4)");
	calc.parse_expr("log(2,4)");
	calc.parse_expr("root(2,4)");
	calc.parse_expr("list(0,10,x,ln(x))");
	calc.parse_expr("sum(0,10,x,ln(x))");
	calc.parse_expr("step(0,10,0.1,x,ln(x))");
	calc.parse_expr("v() = 7");
	calc.parse_expr("v");
	calc.parse_expr("f(x) = x^fact(x) / fact(x)");
	calc.parse_expr("step(0,8,0.1,n,f(n))");
	calc.parse_expr("step(0,8,.1,n,f(n))");
	calc.parse_expr("list(0, 10, n, f(n))");
	calc.parse_expr("sum(0,1,2,add(1,1))");
	calc.parse_expr("sum(1,10,n,ln(n))");
	calc.parse_expr("sum(1,10,n+1,ln(n))");
	calc.parse_expr("!");
	calc.parse_expr("hypot_len(a,b) = root(2,a^2 + b^2)");
	calc.parse_expr("hypot_len(3,4)");
	calc.parse_expr("hypot_len(1,pi)");
	calc.parse_expr("add(x,y) = x + y");
	calc.parse_expr("add(1,add(1,1))");
	calc.parse_expr("x(y) = y");
	calc.parse_expr("x");
	calc.parse_expr("x = y");
	calc.parse_expr("y = x");
	calc.parse_expr("x");
	calc.parse_expr("list(0, 100, n, ln(n))");
	calc.parse_expr("pi * cos(0)");
	calc.parse_expr("var(x,y,z) = 15+x");
	calc.parse_expr("Func(x) = x^3");
	calc.parse_expr("var(x) = 15+x");
	calc.parse_expr("Larc(x) = x*Func(x/Func(x))");
	calc.parse_expr("Larc(x) = x * Func (x)");
	calc.parse_expr("vec(1)");
	calc.parse_expr("1s5");
	calc.parse_expr("vec = 1 + x");
	calc.parse_expr("x = 1 + vec");
	calc.parse_expr("vec");
	calc.parse_expr("Larc(3)");
	calc.parse_expr("sum(0,2,x)");
	calc.parse_expr("c(s = 1");
	calc.parse_expr("c((s) = 1");
	calc.parse_expr("c((s");
	calc.parse_expr("c))");
	func::dump_table();
	calc.parse_expr("plot(-5,5,-5,5,n,cos(n))");
	calc.parse_expr("plot_add(n,ln(n))");
	calc.parse_expr("plot_add(n,sin(n))");
	calc.parse_expr("plot_add(n,-1*sin(n))");	
	calc.parse_expr("plot_addx(n,sin(n))");
	calc.parse_expr("plot_addx(n,-1*sin(n))");
	calc.parse_expr("plot_add(n,cos(n))");
	calc.parse_expr("plot_add(n,-1*cos(n))");	
	calc.parse_expr("plot_addx(n,cos(n))");
	calc.parse_expr("plot_addx(n,-1*cos(n))");
	calc.parse_expr("plot_add(n,tan(n))");
	calc.parse_expr("plot_add(n,-1*tan(n))");
	calc.parse_expr("plot_addx(n,tan(n))");
	calc.parse_expr("plot_addx(n,-1*tan(n))");

	calc.parse_expr("plot_addx(n,tan(n))");
	//calc.parse_expr("plot(-1000,1000,0,100,x,ln(x))");

	calc.input_loop();

	//func::dump_table();
	return 0;
}
