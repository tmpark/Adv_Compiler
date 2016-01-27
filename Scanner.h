//
// Created by Taemin Park on 1/11/16.
//

#ifndef ADV_COMPILER_SCANNER_H
#define ADV_COMPILER_SCANNER_H


#include "FileReader.h"


typedef enum {
    STAT_START, STAT_NUM, STAT_ID, STAT_EQ, STAT_NE, STAT_L, STAT_R, STAT_DONE, STAT_COMMENT, STAT_SLASH
}StateType;



class Scanner {

public:

    Scanner(){number = 0; id = errToken;};
    static Scanner* instance();
    TokenType GetSym();
    int number;
    std::string id;
    void Error(std::string state, std::string missingChar);
    RC openFile(const std::string &fileName);
    RC closeFile();

private:
    char inputSym;
    void Next();
    void Previous();
    static Scanner *_scanner;

};


#endif //ADV_COMPILER_SCANNER_H
