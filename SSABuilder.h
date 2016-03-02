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


class DefinedInfo
{
public:
    DefinedInfo(){};
    DefinedInfo(int blkNum, string symbol){ this->blkNum = blkNum;this->symbol = symbol;preserved = false;};
    void setInst(int arg1, shared_ptr<IRFormat> arg2){kind = instKind; instNum = arg1; inst = arg2;};
    int getInstNum(){return instNum;};
    shared_ptr<IRFormat> getInst(){return inst;};
    void setVar(string arg0, shared_ptr<Symbol> arg1, int arg2){ kind = varKind;var = arg0;definedInst = arg2;varSym = arg1;};
    string getVar(){return var;};
    int getDefinedInstOfVar(){return definedInst;};
    void setConst(int arg){ kind = constKind; constVal = arg;};
    int getConst(){return constVal;};
    Kind getKind(){return kind;};
    int getBlkNum(){return blkNum;};
    void setPreserved(bool arg){preserved = arg;};
    bool ispreserved(){return preserved;};
    shared_ptr<Symbol> getVarSym(){return varSym;};

private:
    int blkNum;
    string symbol;
    Kind kind;
    int instNum;
    shared_ptr<IRFormat> inst;
    string var;
    shared_ptr<Symbol> varSym;
    int definedInst;
    int constVal;
    bool preserved;
};


class SSABuilder {
public:
    SSABuilder(string functionName, int startBlock, int startInst);
    SSABuilder(){};
    void insertDefinedInstr();
    DefinedInfo getDefinedInfo();
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
    void preserveOuter();
    int getStartBlock(){return startBlock;};

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
    Result defBeforeInserted;


    //Phi related information
    stack<vector<shared_ptr<IRFormat>>> previousPhiCodes;
    stack<BlockKind> previousJoinBlockKind;
    stack<int> previousJoinBlockNum;

    //BasicBlock currentJoinBlock;
};


#endif //ADV_COMPILER_SSATRACE_H
