#include <iostream>
#include "Parser.h"

using namespace std;

int main() {
    string folder = "/home/tmpark/ClionProjects/Adv_Compiler/test/";
    string graphFolder = "/home/tmpark/ClionProjects/Adv_Compiler/graph/";
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
    for(int i = 1 ; i < 32 ; i++)
    {
        string testNum = to_string(i);
        size_t numOfChar = testNum.size();
        if(numOfChar == 1)
        {
            testNum =  "00" + testNum;
        }
        else if(numOfChar == 2)
        {
            testNum = "0" + testNum;
        }
        string sourceFileName = "test" + testNum;
        Parser *parser = Parser :: instance();
        rc = parser->openFile(folder + sourceFileName + sourceFileFormat);
        if(rc == -1)
            return 0;
        parser->startParse();
        parser->printBlock();
        parser->createControlFlowGraph(graphFolder+sourceFileName + "/",sourceFileName);
        parser->createDominantGraph(graphFolder+sourceFileName + "/",sourceFileName);
        //string visualizeGraph = xvcg + " " + folder + graphFileName;
        //system(visualizeGraph.c_str());
        //parser->printIRCodes(parser->IRCodes); //Debug
        //parser->printSymbolTable(); //Debug
        parser->closeFile();
        delete parser;
    }


#endif

    return 0;
}