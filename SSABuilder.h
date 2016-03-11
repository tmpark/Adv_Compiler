//
// Created by tmpark on 2/10/16.
//
#include "Helper.h"
#include "BasicBlock.h"
#include <stack>

#ifndef ADV_COMPILER_SSATRACE_H
#define ADV_COMPILER_SSATRACE_H

using namespace std;

/*
typedef struct{
    int blkNum;
    int instNum;
} DefinedInfo;
*/





class SSABuilder {
public:
    SSABuilder(string functionName, int startBlock, int startInst);
    SSABuilder(){};
    void insertDefinedInstr();
    DefinedInfo getDefinedInfo();//Only after preparing(Never return Null)
    DefinedInfo getDefinedInfo(string var);//just get it(Can return bull shit)
    void prepareForProcess(string var,shared_ptr<Symbol> sym, DefinedInfo defInfo);
    void revertToOuter(int blockNum);
    //Return true there is new phi
    shared_ptr<IRFormat> updatePhiFunction(string x, shared_ptr<Symbol> x_sym,Result defined,int operandIndex, int IRpc);
    void startJoinBlock(BlockKind blockKind,int joinBlockNum);
    void endJoinBlock();
    //BasicBlock getJoinBlock(){return currentJoinBlock;}
    int getCurrentJoinBlockNum(){return currentJoinBlockNum;};
    Result getDefBeforeInserted(){return defBeforeInserted;};

    vector<shared_ptr<IRFormat>> getPhiCodes(){return currentPhiCodes;};
    stack<BlockKind> currentBlockKind;
    vector<shared_ptr<IRFormat>> currentPhiCodes;

    int getStartBlock(){return startBlock;};
    void protectDef();

private:
    //Function wide information
    string functionName;
    int startBlock;
    int startInst;
    BlockKind currentJoinBlockKind;
    unordered_map<string,stack<DefinedInfo>> definedInfoTable;


    //Var wide information
    string varName;
    shared_ptr<Symbol> varSym;
    stack<DefinedInfo> definedInfoList;
    //int currentBlockNum;
    //int currentInstrNum;
    int currentJoinBlockNum;
    DefinedInfo currentDefInfo;
    bool definitionExist;
    bool definitionEmpty;
    Result defBeforeInserted;


    //Phi related information
    stack<vector<shared_ptr<IRFormat>>> previousPhiCodes;
    stack<BlockKind> previousJoinBlockKind;
    stack<int> previousJoinBlockNum;

    //BasicBlock currentJoinBlock;
};


#endif //ADV_COMPILER_SSATRACE_H
