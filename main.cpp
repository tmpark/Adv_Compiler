#include <iostream>
#include "CodeGeneration.h"

using namespace std;

int main() {
    string folder = "/home/tmpark/ClionProjects/Adv_Compiler/test/";
    string graphFolder = "/home/tmpark/ClionProjects/Adv_Compiler/graph/";
    string binaryFolder = "/home/tmpark/ClionProjects/Adv_Compiler/binary/";
    string simulator = "DLX";
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

    //shift test
    /*
    int a = 12431;
    int b = a >> 1;
    int aRest = a & 1;
    int c = -a;
    int cRest = c & 1;
    int d = c >> 1;
    cout << a << "\t" << b << "\t"<< aRest <<"\t"<< c << "\t" << d<< "\t" << cRest << endl;*/
    //Issue
    //Test4 not matched number of parameters, wrong array indexing -> intentional error
    //Test13 function is changed to procedure (Michael's mistake)
    //Test15 var name should be allowed

    //Test17,18,19->Copy Propagation Test
    //Test26 -> CSE Test

    //1~35
    //9:inner if
    //10:inner while(unlimited iteration)
    //11:inner if-while  and vice versa(unlimited iteration)
    //1,5,7,8,9,10,11,12,14,17,19,21,22,23,24,25,26,27,28,29,30,31 : No func Call

    for(int i = 1 ; i < 34 ; i++)
    {
        //if(i == 4)
        //    continue;

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
        parser->createControlFlowGraph(graphFolder+sourceFileName + "/" + "CFG_Original" + "/",sourceFileName,"Original");
        parser->createDominantGraph(graphFolder+sourceFileName + "/" + "DT" + "/",sourceFileName);

        std::unordered_map<std::string,vector<shared_ptr<BasicBlock>>> functionList = parser->getFuncList();

        int numOfGlobalVar = parser->getNumOfVarInFunction(GLOBAL_SCOPE_NAME);
        vector<shared_ptr<RegAllocation>> regAllocList;
        vector<string> functionNameList;
        for(auto function : functionList)
        {
            int numOfVar = parser->getNumOfVarInFunction(function.first);
            if(function.first != GLOBAL_SCOPE_NAME)
                numOfVar = numOfVar + numOfGlobalVar;
            int numOfParam = parser->getNumOfParamInFunction(function.first);
            shared_ptr<RegAllocation> regAlloc(new RegAllocation(function.first, function.second, numOfVar + numOfParam));
            regAlloc->doRegAllocation();
            regAllocList.push_back(regAlloc);
            functionNameList.push_back(function.first);
        }

        parser->createControlFlowGraph(graphFolder+sourceFileName + "/" + "CFG_RegAlloc" + "/",sourceFileName,"RegAlloc");

        int index = 0;
        for(auto regAlloc : regAllocList)
        {
            regAlloc->createInterferenceGraph(graphFolder+sourceFileName + "/" + "IG" + "/",sourceFileName,functionNameList.at(index));
            regAlloc.reset();
            index++;
        }


        CodeGeneration codeGeneration(functionList);
        codeGeneration.doCodeGen();
        codeGeneration.writeOutCode(binaryFolder+sourceFileName + "/",sourceFileName);

        string executable = binaryFolder + sourceFileName + "/" + sourceFileName + ".out";
        string execCommand = "java -classpath " + binaryFolder + " " + simulator + " " + executable;

        cout << sourceFileName << "'s output: " <<endl;
        system(execCommand.c_str());
        cout << endl;

        //parser->printIRCodes(parser->IRCodes); //Debug
        //parser->printSymbolTable(); //Debug
        parser->closeFile();
        delete parser;
    }


#endif

    return 0;
}