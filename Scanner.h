//
// Created by Taemin Park on 1/11/16.
//

#ifndef ADV_COMPILER_SCANNER_H
#define ADV_COMPILER_SCANNER_H


#include "FileReader.h"


typedef enum {
    STAT_START, STAT_NUM, STAT_ID, STAT_EQ, STAT_NE, STAT_L, STAT_R, STAT_DONE
}StateType;





class Scanner {

public:

    Scanner(){};
    static Scanner* instance();
    TokenType GetSym();
    int number;
    TokenType id;
    void Error(std::string errorMsg){}
    RC openFile(const std::string &fileName);
    RC closeFile();

private:
    char inputSym;
    void Next();
    void Previous();
    static Scanner *_scanner;

};


#endif //ADV_COMPILER_SCANNER_H
