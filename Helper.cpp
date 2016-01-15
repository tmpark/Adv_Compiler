//
// Created by Taemin Park on 1/12/16.
//
#include "Helper.h"
#include <iostream>

using namespace std;

TokenType getTypeOfOneToken(char c)
{
    switch(c) {
        case '*':
            return timesToken;
        case '/':
            return divToken;
        case '+':
            return plusToken;
        case '-':
            return minusToken;
        case '.':
            return periodToken;
        case ',':
            return commaToken;
        case '[':
            return openbracketToken;
        case ']':
            return closebracketToken;
        case ')':
            return closeparenToken;
        case '(':
            return openparenToken;
        case ';':
            return semiToken;
        case '}':
            return endToken;
        case '{':
            return beginToken;
        case EOF:
            return eofToken;
        default:
            return errorToken;
    }
}



void printToken(TokenType tokenTypeReturned,std::string idConstructed)
{
    switch(tokenTypeReturned)
    {
        case timesToken:
            std::cout << "*" << std::endl;
            break;
        case divToken:
            std::cout << "/" << std::endl;
            break;
        case plusToken:
            std::cout << "+" << std::endl;
            break;
        case minusToken:
            std::cout << "-" << std::endl;
            break;
        case eqlToken:
            std::cout << "==" << std::endl;
            break;
        case neqToken:
            std::cout << "!=" << std::endl;
            break;
        case lssToken:
            std::cout << "<" << std::endl;
            break;
        case geqToken:
            std::cout << ">=" << std::endl;
            break;
        case leqToken:
            std::cout << "<=" << std::endl;
            break;
        case gtrToken:
            std::cout << ">" << std::endl;
            break;
        case periodToken:
            std::cout << "." << std::endl;
            break;
        case commaToken:
            std::cout << "," << std::endl;
            break;
        case openbracketToken:
            std::cout << "[" << std::endl;
            break;
        case closebracketToken:
            std::cout << "]" << std::endl;
            break;
        case closeparenToken:
            std::cout << ")" << std::endl;
            break;
        case becomesToken:
            std::cout << "<-" << std::endl;
            break;
        case openparenToken:
            std::cout << "(" << std::endl;
            break;
        case semiToken:
            std::cout << ";" << std::endl;
            break;
        case endToken:
            std::cout << "}" << std::endl;
            break;
        case beginToken:
            std::cout << "{" << std::endl;
            break;
        case eofToken:
            std::cout << "end of file"<< std::endl;
            break;
        case doToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case odToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case fiToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case elseToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case letToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case callToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case ifToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case whileToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case returnToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case varToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case arrToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case funcToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case procToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case mainToken:
            std::cout << "reserved word: "<< idConstructed << std::endl;
            break;
        case errorToken:
            std::cout << "ERROR: " << idConstructed << std::endl;
            break;
        case numberToken:
            std::cout << "NUM, val= " << idConstructed << std::endl;
            break;
        case identToken:
            std::cout << "ID, name= " << idConstructed << std::endl;
            break;
        default:
            std::cout << "Unknown token: " << tokenTypeReturned << std::endl;

    }

}
