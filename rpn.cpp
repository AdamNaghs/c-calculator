#include <vector>
#include <string>
#include <iostream>
#include "rpn.h"
#include "OpToken.h"


namespace rpn
{
	bool debug = false;

	// sorting removes commas and parenthesis
	void sort(std::vector<tok::OpToken>& v)
	{
		std::vector<tok::OpToken> ops; // use as stack
		std::vector<tok::OpToken> ret;
		ops.reserve(v.size());
		ret.reserve(v.size());
		if (debug)
		{
			std::cout << "Sorting: " << tok::vectostr(v) << '\n' << std::endl;
			mw::MessageWindow::getInstance().print("Sorting: " + tok::vectostr(v));
		}
		tok::OpToken tok;
		for (int i = 0; i < v.size();i++)
		{
			tok = v[i];
			// if digit push to ret
			cmn::op op = tok.GetOperator();
			switch (op)
			{
			case cmn::op::NONE:
			{
				ret.push_back(tok);
				break;
			}
			case cmn::op::L_PAREN:
			{
				ops.push_back(tok);
				break;
			}
			case cmn::op::R_PAREN:
			{
				if (ops.empty()) break;
				tok::OpToken tmp = ops.back();
				ops.pop_back();
				while (!ops.empty() && (tmp.GetOperator() != cmn::op::L_PAREN))
				{
					ret.push_back(tmp);
					tmp = ops.back();
					ops.pop_back();
				}

				break;
			}
			case cmn::op::ADD:
			case cmn::op::SUB:
			case cmn::op::MULT:
			case cmn::op::DIV:
			case cmn::op::POW:
			{
				// While there are operators on the ops stack and the operator at the top of the ops stack has greater or equal precedence
				while (!ops.empty() && cmn::opcmp(ops.back().GetOperator(), tok.GetOperator()) != -1)
				{
					// Pop operators from the ops stack onto the ret stack
					ret.push_back(ops.back());
					ops.pop_back();
				}
				// Push the current operator onto the ops stack
				ops.push_back(tok);
				break;
			}
			default:
				// not a value or a operator
				ret.push_back(tok); // add to ret incase is was desired by user
				break;
			}

			// cout
			if (debug)
			{
				std::cout << i << ": " << tok << '\n';
				mw::MessageWindow::getInstance().print(std::to_string(i) + ": " + tok.toString() + '\n');
				if (!ret.empty())
				{
					std::vector<tok::OpToken> tmp;
					std::cout << "Ret: ";
					mw::MessageWindow::getInstance().print("Ret: ");
					for (auto dump = ret; !dump.empty(); dump.pop_back())
						tmp.insert(tmp.begin(), dump.back());
					for (auto t : tmp)
					{
						std::cout << t << " ";
						mw::MessageWindow::getInstance().print(t.toString() + " ");
					}
					std::cout << '\n';
					mw::MessageWindow::getInstance().print("\n");
				}
				else std::cout << "Ret: Empty\n";
				mw::MessageWindow::getInstance().print("Ret: Empty\n");
				if (!ops.empty())
				{
					std::cout << "Ops: ";
					mw::MessageWindow::getInstance().print("Ops: ");
					std::vector<tok::OpToken> tmp;
					for (auto dump = ops; !dump.empty(); dump.pop_back())
						tmp.insert(tmp.begin(), dump.back());
					for (auto t : tmp)
					{
						std::cout << t << " ";
						mw::MessageWindow::getInstance().print(t.toString() + " ");
					}
					std::cout << '\n';
					mw::MessageWindow::getInstance().print("\n");
				}
				else std::cout << "Ops: Empty\n";
				mw::MessageWindow::getInstance().print("Ops: Empty\n\n");

				std::cout << std::endl;
			}
			// check if at the top of 
		   // the stack there is higher precedence
		}
		while (!ops.empty())
		{
			ret.push_back(ops.back());
			ops.pop_back();
		}
		v = ret;
	}

	cmn::value eval(const std::vector<tok::OpToken>& tokens) {
		std::vector<cmn::value> operandStack;
		operandStack.reserve(tokens.size());
		for (const tok::OpToken& token : tokens) {
			if (!token.IsOperator()) {
				// Push number onto the stack
				if (token.GetType() == tok::FUNCTION) 
				{
					// functions are handled by collapse_functions before evaluation
					std::cerr << "Invalid input. Function '" << token.GetName() <<"' not defined.\n";
					mw::MessageWindow::getInstance().print("Invalid input. Function '" + token.GetName() + "' not defined.\n");
					return 0;
				}
				operandStack.push_back(token.GetValue());
			}
			else {
				// Pop operands from the stack
				if (operandStack.empty()) goto not_enough_operands;
				cmn::value right = operandStack.back();
				operandStack.pop_back();
				if (operandStack.empty()) return right;
				cmn::value left = operandStack.back();
				operandStack.pop_back();

				// Apply the operator
				switch (token.GetOperator()) {
				case cmn::op::ADD:
					operandStack.push_back(left + right);
					break;
				case cmn::op::SUB:
					operandStack.push_back(left - right);
					break;
				case cmn::op::MULT:
					operandStack.push_back(left * right);
					break;
				case cmn::op::DIV:;
					operandStack.push_back(left / right); // Ensure division by zero is handled
					break;
				case cmn::op::POW:
					operandStack.push_back(std::pow(left, right)); // Requires <cmath>
					break;
					// Add cases for other operators as needed
				}
			}
		}

		// The final result is on the top of the stack
		if (operandStack.empty()) 
		{
			not_enough_operands:
			std::cerr << "Invalid input. Not enough operands, cannot solve expression.\n";
			mw::MessageWindow::getInstance().print("Invalid input. Not enough operands, cannot solve expression.\n");
			return 0;
		}
		return operandStack.back();
	}

}
