#include <iostream>
#include "Parser.h"

using namespace std;

int main() {
    string folder = "/home/tmpark/ClionProjects/Adv_Compiler/test/";
    string graphFolder = "/home/tmpark/ClionProjects/Adv_Compiler/graph/";
    string sourceFileName = "test021";
    string sourceFileFormat = ".txt";
    string graphFileName = "graph.vcg";
    string xvcg = "xvcg -font rk24";
    RC rc = -1;



#if NO_PARSE
    Scanner *scanner = Scanner :: instance();
    rc = scanner->openFile(folder + sourceFileName);
    if(rc == -1)
        return 0;
    while(scanner->GetSym() != eofToken);
    scanner->closeFile();

#else
    Parser *parser = Parser :: instance();
    rc = parser->openFile(folder + sourceFileName + sourceFileFormat);
    if(rc == -1)
        return 0;
    parser->startParse();
    parser->printBlock();
    parser->createControlFlowGraph(graphFolder,sourceFileName);
    parser->createDominantGraph(graphFolder,sourceFileName);
    //string visualizeGraph = xvcg + " " + folder + graphFileName;
    //system(visualizeGraph.c_str());

    //parser->printIRCodes(parser->IRCodes); //Debug
    //parser->printSymbolTable(); //Debug
    parser->closeFile();

#endif

    return 0;
}