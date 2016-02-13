//
// Created by tmpark on 2/10/16.
//
#include "Helper.h"
#include "BasicBlock.h"
#include <stack>

#ifndef ADV_COMPILER_SSATRACE_H
#define ADV_COMPILER_SSATRACE_H

using namespace std;

typedef struct{
    int blockNum;
    int instNum;
} DefinedLoc;

class SSABuilder {
public:
    SSABuilder(string functionName, int startBlock, int startInst);
    SSABuilder(){};
    void insertDefinedInstr();
    int getDefinedInstr();
    void prepareForProcess(string var,int blockNum, int instrNum);
    void revertToOuter(int blockNum);
    //Return true there is new phi
    IRFormat updatePhiFunction(Result x,int operandIndex, int IRpc);

    void startJoinBlock(BlockKind blockKind, vector<IRFormat> codes);
    void endJoinBlock();
    //BasicBlock getJoinBlock(){return currentJoinBlock;}
    vector<IRFormat> getJoinBlockCodes(){return currentJoinBlockCodes;};
    vector<IRFormat> getPhiCodes(){return currentPhiCodes;};
    stack<BlockKind> currentBlockKind;
    vector<IRFormat> currentPhiCodes;
    vector<IRFormat> currentJoinBlockCodes;

private:
    //Function wide information
    string functionName;
    int startBlock;
    int startInst;
    BlockKind currentJoinBlockKind;
    unordered_map<string,stack<DefinedLoc>> definedLocTable;


    //Var wide information
    string varName;
    stack<DefinedLoc> definedLocList;
    int currentBlockNum;
    int currentInstrNum;
    bool definitionExist;
    Result previousDef;


    //Phi related information
    stack<vector<IRFormat>> previousPhiCodes;
    stack<vector<IRFormat>> previousJoinBlockCodes;
    stack<BlockKind> previousJoinBlockKind;
    //BasicBlock currentJoinBlock;
};


#endif //ADV_COMPILER_SSATRACE_H
