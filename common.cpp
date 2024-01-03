#include "common.h"

namespace cmn
{

    int getPrecedence(enum op o)
    {
        switch (o)
        {
        case POW:
            return 3;
        case MULT:
        case DIV:
            return 2;
        case ADD:
        case SUB:
            return 1;
        case L_PAREN:
        case R_PAREN:
            return 0;
        case NONE:
        case COMMA:
        default:
            return -1;
        }
    }

    int opcmp(enum op a, enum op b)
    {

        int aPrecedence = getPrecedence(a);
        int bPrecedence = getPrecedence(b);

        if (aPrecedence > bPrecedence)
            return 1;
        else if (aPrecedence < bPrecedence)
            return -1;
        else
            return 0;
    }
    
    enum op chtoop(char c)
    {
        switch (c)
        {
        case '^':
            return POW;
        case '*':
            return MULT;
        case '/':
            return DIV;
        case '+':
            return ADD;
        case '-':
            return SUB;
        case '(':
            return L_PAREN;
        case ')':
            return R_PAREN;
        case ',':
            return COMMA;
        case '=':
            return EQUAL;
        default:
            return NONE;
        }
    }

    char optoch(enum op o) {
        switch (o)
        {
        case ADD:
            return '+';
        case SUB:
            return '-';
        case MULT:
            return '*';
        case DIV:
            return '/';
        case POW:
            return '^';
        case L_PAREN:
            return '(';
        case R_PAREN:
            return ')';
        case COMMA:
            return ',';
        case EQUAL:
            return '=';
        default:
            return '@';
        }
    }
}