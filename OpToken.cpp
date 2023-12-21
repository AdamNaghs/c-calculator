#include "OpToken.h"
#include <sstream>

namespace tok{

std::ostream& operator<<(std::ostream& os, OpToken& token) {
    std::string tmp = "Token(";
    switch (token.GetType())
    {
    case TokenType::OPERATOR:
        os << tmp << "Operator:'";
        break;
    case TokenType::VALUE:
        os << tmp << "Value:'";
        break;
    case TokenType::FUNCTION:
        os << tmp << "Variable:'";
        break;
    default:
        os << "Error Converting Token to Stream.";
        break;
    }
    os << token.toString() << "')";
    return os;
}


std::vector<OpToken> str_to_optoks(std::string s)
{
    // remove all spaces
    s.erase(std::remove_if(s.begin(), s.end(), isspace), s.end());
    std::vector<OpToken> ret;
    ret.reserve(s.length());
    int start = 0, k;
    for (k = 0; k < s.length(); k++)
    {
        char c = s.at(k);

        if (c == '-' && (k == 0 || cmn::chtoop(s.at(k - 1)))) // '-' is part of a num if its the first char or if the last char was an op
            if (k == start) // if it's the start of a number
                continue; // just continue, '-' is part of the number
        if (cmn::chtoop(c) && (k != start || c != '-')) // skip '-' if it's part of a number
        {
            if (k > start)
            {
                std::string tmp = s.substr(start, k - start);
                char* end;
                if (!tmp.empty()) {
                    char* end;
                    if (isdigit(tmp.at(0)) || (tmp.at(0) == '-' && tmp.size() > 1 && isdigit(tmp.at(1)))) {
                        ret.push_back(OpToken(std::strtod(tmp.c_str(), &end)));
                    }
                    else {
                        // Handle variable or function names
                        ret.push_back(OpToken(tmp));
                    }
                }

            }
            start = k + 1;

            ret.push_back(OpToken(cmn::chtoop(c)));

        }
        else if (k == s.length() - 1) // add end of string
        {
            std::string tmp = s.substr(start, k - start + 1);
            char* end;
            if (!tmp.empty()) {
                char* end;
                if (isdigit(tmp.at(0)) || (tmp.at(0) == '-' && tmp.size() > 1 && isdigit(tmp.at(1)))) {
                    ret.push_back(OpToken(std::strtod(tmp.c_str(), &end)));
                }
                else {
                    // Handle variable or function names
                    ret.push_back(OpToken(tmp));
                }
            }

        }
    }
    return ret;
}

std::string vectodbgstr(std::vector<OpToken> v)
{
    std::stringstream s;
    for (auto t : v)
    {
        s << t << ' ';
    }
    return s.str();
}

std::string vectostr(std::vector<OpToken> v)
{
    std::stringstream s;
    bool last_was_excluded = false;
    for (int i = 0; i < v.size(); i++)
    {
        std::string tmp = v[i].toString();
        cmn::op op = v[i].GetOperator();
        if (op != cmn::COMMA && op != cmn::L_PAREN && op != cmn::R_PAREN && op != cmn::NONE)
        {
            tmp = ' ' + tmp + ' ';
        }
        s << tmp;
    }
    return s.str();
}

}
