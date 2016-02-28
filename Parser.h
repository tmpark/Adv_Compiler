//
// Created by Taemin Park on 1/14/16.
//

#ifndef ADV_COMPILER_PARSER_H
#define ADV_COMPILER_PARSER_H
//#define CODEBUFSIZE 128 //128 * uint32 = 4096
//#define NUMOFREGS 32


#include <array>
#include <vector>
#include <stack>
#include <map>
#include <algorithm>
#include "Scanner.h"
#include "GraphDrawer.h"
#include "SSABuilder.h"
#include "BasicBlock.h"
#include "CSETracker.h"

using namespace std;


typedef enum{
    sym_err, sym_var, sym_param, sym_array, sym_func, sym_proc
}SymType;



class Symbol
{

public:
    Symbol(){ symType = sym_err; arrayCapacity = {};
        numOfParam = 0;};
    Symbol(SymType symType, int loc)
    { this->symType = symType;this->symBaseAddr = loc;
        numOfParam = 0;};
    Symbol(SymType symType, int loc, vector<int> arrayCapacity)
    { this->symType = symType; this->symBaseAddr = loc; this->arrayCapacity = arrayCapacity;
        numOfParam = 0;};
    Symbol(SymType symType, int loc, int numOfParam) //For fucntion symbol
    { this->symType = symType; this->symBaseAddr = loc;
        this->numOfParam = numOfParam;};
    void setSymType(SymType arg){symType = arg;};
    SymType getSymType(){return symType;};
    void setBaseAddr(int arg){ symBaseAddr = arg;};
    int getBaseAddr(){return symBaseAddr;};
    void setNumOfParam(int arg){ this->numOfParam = numOfParam;};
    int getNumOfParam(){ return numOfParam;};



    std::vector<int> arrayCapacity; //only for array : capacity and dimension

    //std::vector<int> symAssignedInst; //only for variable assigned information

private:
    SymType symType; //var, array, function, procedure
    int symBaseAddr; //Location of symbol
    int numOfParam; //Only for function

};


class SymTable
{
public:
    SymTable(){parentSymTableName = "";
        localVarTop = LOCAL_IN_STACK;};
    SymTable(string parentSymTableName)
    { this->parentSymTableName = parentSymTableName;
        localVarTop = LOCAL_IN_STACK;};
    void setParent(string arg){this->parentSymTableName = arg;};
    string getParent(){return parentSymTableName;};
    unordered_map<string,Symbol> varSymbolList;
    unordered_map<string,Symbol> funcSymbolList;
    int getLocalVarTop(){return localVarTop;};
    void setLocalVarTop(int arg){ localVarTop = arg;};
private:
    string parentSymTableName;
    int localVarTop;
};




class Parser {

public:
    Parser();
    ~Parser(){
        Scanner *scanner = Scanner::instance();
        delete scanner;
        GraphDrawer *graphDrawer = GraphDrawer::instance();
        delete graphDrawer;
        _parser = 0;
    };
    static Parser* instance();
    RC openFile(const std::string &folder, const std::string &sourceFileName, const std::string &sourceFileFormat);    RC closeFile();
    void startParse();
    void printIRCodes(vector<shared_ptr<IRFormat>> codes);
    void printSymbolTable();
    void printBlock();
    void createControlFlowGraph(const string &graphFolder,const string &sourceFileName);
    void createDominantGraph(const string &graphFolder,const string &sourceFileName);

    string getCodeString(shared_ptr<IRFormat> code);
    std::unordered_map<std::string,vector<shared_ptr<BasicBlock>>> getFuncList(){return functionList;};


private:

    string fileName;
    //Error message
    void Error(std::string nonTerminal, std::initializer_list<std::string> missingTokens);

    //Next token for a explicit token
    void Next();

    //Non terminals
    void computation();
    void funcBody();
    vector<string> formalParam();
    void funcDecl();
    void varDecl();
    Symbol typeDecl();
    void statSequence();
    void statement();
    void returnStatement();
    void whileStatement();
    void ifStatement();
    Result funcCall();
    Result assignment();
    Result relation();
    Result expression();
    Result term();
    Result factor();
    Result designator();
    IROP relOp();


    //Intermediate Code emittion
    Result emitIntermediate(IROP irOp,vector<Result> x);
    void emitOrUpdatePhi(string x, Result defined);
    void CondJF(Result &x);
    void UnCJF(Result &x);
    void Fixup(unsigned long loc);
    void FixLink(unsigned long loc);
    void predefinedFunc();
    Result getAddressInStack(int location);



    int IRpc;
    int depth;
    std::vector<shared_ptr<IRFormat>> IRCodes; //index is correspond to line Num
    std::stack<std::string> scopeStack;
    std::unordered_map<int,stack<int>> dominatedByInfo;
    std::unordered_map<std::string,SymTable> symTableList;
    //std::vector<BasicBlock> basicBlockList;
    std::unordered_map<std::string,vector<shared_ptr<BasicBlock>>> functionList;
    shared_ptr<BasicBlock> currentBlock;
    unordered_map<int,shared_ptr<BasicBlock>> instructionBlockPair;
    bool finalizeAndStartNewBlock(BlockKind newBlockKind, bool isCurrentCond, bool directFlowExist, bool dominate);
    void updatePhiInBB(int modifiedBlockNum, vector<shared_ptr<IRFormat>> codes);
    void insertBasicBlock(shared_ptr<BasicBlock> block);
    void updateBlockForDT(int dominatingBlockNum);
    shared_ptr<BasicBlock> getBlockFromNum(int blockNum);
    bool isDominate(int dominatingBlockNum, int dominatedBlockNum);

    int numOfBlock;

    void addFuncSymbol(SymType symType, std::string symbolName, unsigned long numOfParam);
    void addVarSymbol(std::string symbol, SymType symType, std::vector<int> arrayCapacity);
    void addParamSymbol(std::string symbol, size_t numOfParam, int index);

    int addSymInTable(){return numOfSym++;}; //Fixme: fake implementation
    Symbol symTableLookup(std::string symbol,SymType symType);
    void symbolTableUpdate(string var,Symbol varSym);

    //SSA
    SSABuilder ssaBuilder;

    //CSE
    CSETracker cseTracker;
    void cseForLoad(int dominatingBlockNum);
/*
    //Code emit related
    void PutF1(int op, int a, int b, int c);
    void Load(Result &x);
    int AllocateReg();
    void DeAllocateReg(int regNum);
    void Compute(TokenType computeToken, Result &x, Result &y);
*/

    TokenType scannerSym;

    //std::array<int32_t, CODEBUFSIZE> buf;
    //std::array<bool,NUMOFREGS> regsBusy;

    //int pc;
   // int globalBase;
    int numOfSym;


    static Parser *_parser;

};


#endif //ADV_COMPILER_PARSER_H
