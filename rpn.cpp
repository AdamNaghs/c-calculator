#include <stack>
#include <string>
#include <iostream>
#include "rpn.h"
#include "OpToken.h"

namespace rpn
{
	bool debug = false;

	void sort(std::vector<tok::OpToken>& v)
	{
		std::stack<tok::OpToken> ops;
		std::stack<tok::OpToken> ret;


		for (tok::OpToken tok : v)
		{
			// if digit push to ret
			cmn::op op = tok.GetOperator();
			switch (op)
			{
			case cmn::op::NONE:
			{
				ret.push(tok);
				break;
			}
			case cmn::op::L_PAREN:
			{
				ops.push(tok);
				break;
			}
			case cmn::op::R_PAREN:
			{
				if (ops.empty()) break;
				tok::OpToken tmp = ops.top();
				ops.pop();
				while (!ops.empty() && (tmp.GetOperator() != cmn::op::L_PAREN))
				{
					ret.push(tmp);
					tmp = ops.top();
					ops.pop();
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
				while (!ops.empty() && cmn::opcmp(ops.top().GetOperator(), tok.GetOperator()) != -1)
				{
					// Pop operators from the ops stack onto the ret stack
					ret.push(ops.top());
					ops.pop();
				}
				// Push the current operator onto the ops stack
				ops.push(tok);
				break;
			}
			default:
				// not a value or a operator
				ret.push(tok); // add to ret incase is was desired by user
				break;
			}

			// cout
			if (debug)
			{
				if (!ret.empty())
				{
					std::vector<tok::OpToken> tmp;
					std::cout << "Ret: ";
					for (auto dump = ret; !dump.empty(); dump.pop())
						tmp.insert(tmp.begin(), dump.top());
					for (auto t : tmp)
					{
						std::cout << t << " ";
					}
					std::cout << '\n';
				}
				else std::cout << "Ret: Empty\n";
				if (!ops.empty())
				{
					std::cout << "Ops: ";
					std::vector<tok::OpToken> tmp;
					for (auto dump = ops; !dump.empty(); dump.pop())
						tmp.insert(tmp.begin(), dump.top());
					for (auto t : tmp)
					{
						std::cout << t << " ";
					}
					std::cout << '\n';
				}
				else std::cout << "Ops: Empty\n";

				std::cout << std::endl;
			}
			// check if at the top of 
		   // the stack there is higher precedence
		}
		// manually convert stack to vec
		while (!ops.empty())
		{
			ret.push(ops.top());
			ops.pop();
		}
		v.clear();
		v.reserve(ret.size());
		while (!ret.empty())
		{
			v.insert(v.begin(), ret.top());
			ret.pop();
		}

	}

	cmn::value eval(const std::vector<tok::OpToken>& tokens) {
		std::stack<cmn::value> operandStack;

		for (const auto& token : tokens) {
			if (!token.IsOperator()) {
				// Push number onto the stack
				operandStack.push(token.GetValue());
			}
			else {
				// Pop operands from the stack
				if (operandStack.empty()) goto not_enough_operands;
				cmn::value right = operandStack.top();
				operandStack.pop();
				if (operandStack.empty()) return right;
				cmn::value left = operandStack.top();
				operandStack.pop();

				// Apply the operator
				switch (token.GetOperator()) {
				case cmn::op::ADD:
					operandStack.push(left + right);
					break;
				case cmn::op::SUB:
					operandStack.push(left - right);
					break;
				case cmn::op::MULT:
					operandStack.push(left * right);
					break;
				case cmn::op::DIV:;
					operandStack.push(left / right); // Ensure division by zero is handled
					break;
				case cmn::op::POW:
					operandStack.push(std::pow(left, right)); // Requires <cmath>
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
			return 0;
		}
		return operandStack.top();
	}

	void eval(const std::vector<std::vector<tok::OpToken>>& unsorted)
	{
		for (std::vector<tok::OpToken> v : unsorted)
		{
			sort(v);
			eval(v);
		}

	}

	cmn::value eval_str(std::string input)
	{
		if (!cmn::do_paren_match(input))
		{
			std::cerr << "Invalid input. Parenthesis do not match.\n";
			return 0;
		}
		std::vector<tok::OpToken> vec = tok::str_to_optoks(input);
		sort(vec);
		return eval(vec);
	}

}
