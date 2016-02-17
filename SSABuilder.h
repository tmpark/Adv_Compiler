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
    shared_ptr<IRFormat> updatePhiFunction(Result x,int operandIndex, int IRpc);
    void startJoinBlock(BlockKind blockKind,int joinBlockNum);
    void endJoinBlock();
    //BasicBlock getJoinBlock(){return currentJoinBlock;}
    int getCurrentJoinBlockNum(){return currentJoinBlockNum;};
    int getPreviousDefinedInst(){return previousDef.getDefInst();};

    vector<shared_ptr<IRFormat>> getPhiCodes(){return currentPhiCodes;};
    stack<BlockKind> currentBlockKind;
    vector<shared_ptr<IRFormat>> currentPhiCodes;

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
    int currentJoinBlockNum;
    bool definitionExist;
    Result previousDef;


    //Phi related information
    stack<vector<shared_ptr<IRFormat>>> previousPhiCodes;
    stack<BlockKind> previousJoinBlockKind;
    stack<int> previousJoinBlockNum;
    //BasicBlock currentJoinBlock;
};


#endif //ADV_COMPILER_SSATRACE_H
