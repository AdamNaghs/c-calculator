#pragma once
#include <string>

namespace cmn {

	typedef double value;

enum op
{
	NONE,
	ADD,
	SUB,
	DIV,
	MULT,
	POW,
    L_PAREN,
    R_PAREN,
	COMMA, // not meant to be used except internally
	EQUAL // not meant to be used except internally
};
char optoch(enum op o);
enum op chtoop(char c);
int opcmp(enum op a, enum op b);
int getPrecedence(enum op o);
bool do_paren_match(std::string input);

}
