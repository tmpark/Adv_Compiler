#include <iostream>
#include "RegAllocation.h"

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
    //Issue
    //Test4 not matched number of parameters, wrong array indexing -> intentional error
    //Test13 function is changed to procedure (Michael's mistake)
    //Test15 var name should be allowed

    //Test17,18,19->Copy Propagation Test
    //Test26 -> CSE Test

    //1~35
    for(int i = 1 ; i < 35 ; i++)
    {
        if(i == 4)
            continue;

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

        if(i == 32)
            sourceFileName = "cell";
        else if(i == 33)
            sourceFileName = "factorial";

        Parser *parser = Parser :: instance();
        rc = parser->openFile(folder,sourceFileName,sourceFileFormat);
        if(rc == -1)
            return 0;
        parser->startParse();
        //parser->printBlock();
        parser->createControlFlowGraph(graphFolder+sourceFileName + "/",sourceFileName);
        parser->createDominantGraph(graphFolder+sourceFileName + "/",sourceFileName);

        std::unordered_map<std::string,vector<shared_ptr<BasicBlock>>> functionList = parser->getFuncList();
        for(auto function : functionList)
        {
            int numOfRegs = 8;
            RegAllocation regAlloc(function.second);
            regAlloc.doRegAllocation();
            regAlloc.createInterferenceGraph(graphFolder+sourceFileName + "/",sourceFileName,function.first);
        }


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