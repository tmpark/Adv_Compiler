//
// Created by Taemin Park on 1/14/16.
//

#ifndef ADV_COMPILER_PARSER_H
#define ADV_COMPILER_PARSER_H


#include "Scanner.h"

class Parser {

public:
    Parser(){};
    static Parser* instance();
    void Error(std::string nonTerminal, std::initializer_list<std::string> missingTokens);
    //void Error(std::string nonTerminal, std::string missingTerm);
    //void Error(std::string nonTerminal, std::string missingTerm, int numOfToken, ...);
    RC openFile(const std::string &fileName);
    void startParse();
    RC closeFile();

private:
    TokenType scannerSym;
    void Next();
    static Parser *_parser;

    void computation();
    void funcBody();
    void formalParam();
    void funcDecl();
    void varDecl();
    void typeDecl();
    void statSequence();
    void statement();
    void returnStatement();
    void whileStatement();
    void ifStatement();
    void funcCall();
    void assignment();
    void relation();
    void expression();
    void term();
    void factor();
    void designator();
    void relOp();

};


#endif //ADV_COMPILER_PARSER_H
