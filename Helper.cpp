//
// Created by Taemin Park on 1/12/16.
//
#include "Helper.h"
#include <iostream>

using namespace std;



std::string getTokenStr(TokenType token)
{
    return tokenStringMap.find(token)->second;
}

TokenType getTypeOfOneToken(char c)
{
    switch(c) {
        case '*':
            return timesToken;
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
            std::cout << "*" << std::endl << std::flush;
            break;
        case divToken:
            std::cout << "/" << std::endl<< std::flush;
            break;
        case plusToken:
            std::cout << "+" << std::endl<< std::flush;
            break;
        case minusToken:
            std::cout << "-" << std::endl<< std::flush;
            break;
        case eqlToken:
            std::cout << "==" << std::endl<< std::flush;
            break;
        case neqToken:
            std::cout << "!=" << std::endl<< std::flush;
            break;
        case lssToken:
            std::cout << "<" << std::endl<< std::flush;
            break;
        case geqToken:
            std::cout << ">=" << std::endl<< std::flush;
            break;
        case leqToken:
            std::cout << "<=" << std::endl<< std::flush;
            break;
        case gtrToken:
            std::cout << ">" << std::endl<< std::flush;
            break;
        case periodToken:
            std::cout << "." << std::endl<< std::flush;
            break;
        case commaToken:
            std::cout << "," << std::endl<< std::flush;
            break;
        case openbracketToken:
            std::cout << "[" << std::endl<< std::flush;
            break;
        case closebracketToken:
            std::cout << "]" << std::endl<< std::flush;
            break;
        case closeparenToken:
            std::cout << ")" << std::endl<< std::flush;
            break;
        case becomesToken:
            std::cout << "<-" << std::endl<< std::flush;
            break;
        case openparenToken:
            std::cout << "(" << std::endl<< std::flush;
            break;
        case semiToken:
            std::cout << ";" << std::endl<< std::flush;
            break;
        case endToken:
            std::cout << "}" << std::endl<< std::flush;
            break;
        case beginToken:
            std::cout << "{" << std::endl<< std::flush;
            break;
        case eofToken:
            std::cout << "end of file"<< std::endl<< std::flush;
            break;
        case thenToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case doToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case odToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case fiToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case elseToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case letToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case callToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case ifToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case whileToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case returnToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case varToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case arrToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case funcToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case procToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case mainToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case errorToken:
            std::cout << "ERROR: " << idConstructed << std::endl<< std::flush;
            break;
        case numberToken:
            std::cout << "NUM, val= " << idConstructed << std::endl<< std::flush;
            break;
        case identToken:
            std::cout << "ID, name= " << idConstructed << std::endl<< std::flush;
            break;
        default:
            std::cout << "Unknown token: " << tokenTypeReturned << std::endl<< std::flush;

    }

}

bool isRelOp(TokenType scannerSym)
{
    return scannerSym == eqlToken || scannerSym == neqToken || scannerSym ==lssToken || scannerSym ==leqToken || scannerSym ==gtrToken || scannerSym ==geqToken;
}

bool isDesignator(TokenType scannerSym)
{

    return (scannerSym == identToken);
}
bool isFactor(TokenType scannerSym)
{
    return isDesignator(scannerSym) || scannerSym == numberToken || scannerSym == openparenToken || isFuncCall(scannerSym);
}
bool isTerm(TokenType scannerSym)
{
    return isFactor(scannerSym);
}
bool isExpression(TokenType scannerSym)
{
    return isTerm(scannerSym);
}
bool isRelation(TokenType scannerSym)
{
    return isExpression(scannerSym);
}
bool isAssignment(TokenType scannerSym)
{
    return scannerSym == letToken;
}
bool isFuncCall(TokenType scannerSym)
{
    return scannerSym == callToken;
}
bool isIfStatement(TokenType scannerSym)
{
    return scannerSym == ifToken;
}
bool isWhileStatement(TokenType scannerSym)
{
    return scannerSym == whileToken;
}
bool isReturnStatement(TokenType scannerSym)
{
    return scannerSym == returnToken;
}
bool isStatement(TokenType scannerSym)
{
    return isAssignment(scannerSym) || isFuncCall(scannerSym) || isIfStatement(scannerSym) || isWhileStatement(scannerSym) || isReturnStatement(scannerSym);
}
bool isStatSequence(TokenType scannerSym)
{
    return isStatement(scannerSym);

}
bool isTypeDecl(TokenType scannerSym)
{
    return scannerSym == varToken || scannerSym == arrToken;
}

bool isVarDecl(TokenType scannerSym)
{
    return isTypeDecl(scannerSym);
}
bool isFuncDecl(TokenType scannerSym)
{
    return scannerSym == funcToken || scannerSym == procToken;
}
bool isFormalParam(TokenType scannerSym)
{
    return scannerSym ==  openparenToken;
}

bool isFuncBody(TokenType scannerSym)
{
    return isVarDecl(scannerSym) || scannerSym == beginToken;
}

