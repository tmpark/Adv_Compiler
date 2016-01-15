#include <iostream>
#include "Scanner.h"

using namespace std;

int main() {
    string folder = "/Users/taeminpark/ClionProjects/Adv_Compiler/test/";
    string fileName = "test031.txt";

#if NO_PARSE
    Scanner *scanner = Scanner :: instance();
    RC rc = scanner->openFile(folder + fileName);
    if(rc == -1)
        return 0;
    while(scanner->GetSym() != eofToken);
    scanner->closeFile();
    cout << "sibal";

#else
        cout << "No parse implemented" << endl;
#endif

    return 0;
}