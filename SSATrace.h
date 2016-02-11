//
// Created by tmpark on 2/10/16.
//
#include "Helper.h"
#include <stack>

#ifndef ADV_COMPILER_SSATRACE_H
#define ADV_COMPILER_SSATRACE_H

using namespace std;

typedef struct{
    int blockNum;
    int instNum;
} DefinedLoc;

class SSATrace {
public:
    SSATrace(string functionName, int startBlock, int startInst);
    SSATrace(){};
    void insertDefinedInstr();
    int getDefinedInstr();
    void traceBack();
    void prepareForProcess(string var,int blockNum, int instrNum);

private:
    //Function wide information
    string functionName;
    int startBlock;
    int startInst;
    unordered_map<string,stack<DefinedLoc>> definedLocTable;


    //Var wide information
    string varName;
    stack<DefinedLoc> definedLocList;
    int currentBlockNum;
    int currentInstrNum;
    bool definitionExist;
};


#endif //ADV_COMPILER_SSATRACE_H
