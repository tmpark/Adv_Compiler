#include <iostream>
#include "Parser.h"

using namespace std;

int main() {
    string folder = "/Users/taeminpark/ClionProjects/Adv_Compiler/test/";
    string fileName = "big.txt";
    RC rc = -1;

#if NO_PARSE
    Scanner *scanner = Scanner :: instance();
    rc = scanner->openFile(folder + fileName);
    if(rc == -1)
        return 0;
    while(scanner->GetSym() != eofToken);
    scanner->closeFile();

#else
    Parser *parser = Parser :: instance();
    rc = parser->openFile(folder + fileName);
    if(rc == -1)
        return 0;
    parser->startParse();
    parser->closeFile();

#endif

    return 0;
}