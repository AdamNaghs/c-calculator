#pragma once
#include <vector>
#include "common.h"
#include "OpToken.h"

//#define RPN_DEBUG

namespace rpn {

	extern bool debug;

	void sort(std::vector<tok::OpToken>& v);

	cmn::value eval(const std::vector<tok::OpToken>& sorted_tokens);

	void eval(const std::vector<std::vector<tok::OpToken>>& UNsorted_tokens);
	
	cmn::value eval_str(const std::string);
}
