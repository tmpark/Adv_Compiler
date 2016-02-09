//
// Created by Taemin Park on 1/14/16.
//

#ifndef ADV_COMPILER_PARSER_H
#define ADV_COMPILER_PARSER_H
//#define CODEBUFSIZE 128 //128 * uint32 = 4096
//#define NUMOFREGS 32
#define RETURN_IN_STACK -1
#define PRE_FP_IN_STACK 0
#define PARAM_IN_STACK 1
#define LOCAL_IN_STACK -1 //Same as return pointer(variable should be saved after moving)
#define FRAMEPOINTER 28
#define STACKPOINTER 29
#define RETURNADDRESS 31

#include <array>
#include <vector>
#include <stack>
#include <map>
#include <memory>
#include "Scanner.h"
#include "GraphDrawer.h"

using namespace :: std;


typedef enum{
    errKind,constKind,varKind,instKind,blockKind, regKind
}Kind;

typedef enum{
    errType, varType, arrayType,paramType, functionType, procedureType
}SymType;

typedef enum{
    blk_normal, blk_cond
}BlockKind;



class Result{
public:
    Result(){kind = errKind; value = -1; variable = ""; instNo = -1; relOp = IR_err, fixLoc = -1;}
    void setKind(Kind arg){kind = arg;};
    Kind getKind(){return kind;};
    //Const
    void setConst(int arg){kind = constKind;value = arg;};
    int getConst(){return value;};
    //Variable
    void setVariable(string arg1){kind = varKind; variable = arg1;};
    string getVariable(){return variable;};
    //Instruction
    void setInst(int arg){kind = instKind; instNo = arg;};
    int getInst(){return instNo;};
    void setBlock(int arg){kind = blockKind; blockNo = arg;};
    int getBlock(){return blockNo;};
    void setRelOp(IROP arg){relOp = arg;}; //Only for relation
    IROP getRelOp(){return relOp;};  //Only for relation
    void setFixLoc(int arg){fixLoc = arg;};//Only for relation
    int getFixLoc(){return fixLoc;};//Only for relation
    void setReg(int arg){kind = regKind; regNo = arg;};
    int getReg(){return regNo;};
private:
    Kind kind;
    //constKind
    int value;
    //varKind
    std::string variable;
    //instKind
    int instNo;
    int blockNo;
    IROP relOp ;
    int fixLoc;
    int regNo;
};



class Symbol
{

public:
    Symbol(){ symType = errType; arrayCapacity = {}; definedInstr;
        numOfParam = 0;}
    Symbol(SymType symType, int loc)
    { this->symType = symType;this->symBaseAddr = loc;
        numOfParam = 0;definedInstr;};
    Symbol(SymType symType, int loc, vector<int> arrayCapacity)
    { this->symType = symType; this->symBaseAddr = loc; this->arrayCapacity = arrayCapacity;
        numOfParam = 0;definedInstr;};
    Symbol(SymType symType, int loc, int numOfParam) //For fucntion symbol
    { this->symType = symType; this->symBaseAddr = loc; this->numOfParam = numOfParam;definedInstr;};
    void setSymType(SymType arg){symType = arg;};
    SymType getSymType(){return symType;};
    void setBaseAddr(int arg){ symBaseAddr = arg;};
    int getBaseAddr(){return symBaseAddr;};
    void setNumOfParam(int arg){ this->numOfParam = numOfParam;};
    int getNumOfParam(){ return numOfParam;};
    void setDefinedInstr(int arg){definedInstr = arg;};
    int getDefinedInstr(){return definedInstr;};

    std::vector<int> arrayCapacity; //only for array : capacity and dimension
    //std::vector<int> symAssignedInst; //only for variable assigned information

private:
    SymType symType; //var, array, function, procedure
    int symBaseAddr; //Location of symbol
    int numOfParam; //Only for function
    int definedInstr;

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
    unordered_map<string,Symbol> symbolList;
    int getLocalVarTop(){return localVarTop;};
    void setLocalVarTop(int arg){ localVarTop = arg;};
private:
    string parentSymTableName;
    int localVarTop;
};

class IRFormat
{
public:
    IRFormat(){ instNo = -1; ir_op = IR_err;}

    void setLineNo(int arg){ instNo = arg;};
    int getLineNo(){return instNo;};

    void setIROP(IROP arg){ir_op = arg;};
    IROP getIROP(){return ir_op;};
    std::vector<Result> operands;

private:
    int instNo;
    IROP ir_op;

};


class BasicBlock
{
public:
    BasicBlock(){blockNum = 0;blkKind = blk_normal;trueEdge = -1;};
    BasicBlock(int blockNum){this->blockNum = blockNum;blkKind = blk_normal;trueEdge = -1;};
    BasicBlock(int blockNum,string blockName){this->blockNum = blockNum;this->blockName = blockName;blkKind = blk_normal;trueEdge = -1;};
    int getBlockNum(){return blockNum;};
    bool isTrueEdge(int edge){return edge == trueEdge;};
    bool isCondBlock(){return blkKind == blk_cond;};
    void setTrueEdge(int edge){blkKind = blk_cond; trueEdge = edge;};
    string getBlockName(){return blockName;};
    void setBlockName(string arg){blockName = arg;};

    vector<IRFormat> irCodes;
    vector<int> CFGForwardEdges;
    vector<int> DTForwardEdges;
private:
    int blockNum;
    BlockKind blkKind;
    string blockName;
    int trueEdge;
};


class Parser {

public:
    Parser();
    static Parser* instance();
    RC openFile(const std::string &fileName);
    RC closeFile();
    void startParse();
    void printIRCodes(vector<IRFormat> codes);
    void printSymbolTable();
    void printBlock();
    void createControlFlowGraph(const string &graphFolder,const string &sourceFileName);
    void createDominantGraph(const string &graphFolder,const string &sourceFileName);

    string getCodeString(IRFormat code);
private:

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
    Result emitIntermediate(IROP irOp,std::initializer_list<Result> x);
    void CondJF(Result &x);
    void UnCJF(Result &x);
    void Fixup(unsigned long loc);
    void FixLink(unsigned long loc);
    void predefinedFunc();
    Result getAddressInStack(int location);



    int IRpc;
    std::vector<IRFormat> IRCodes;
    std::stack<std::string> scopeStack;
    //std::stack<int> DTStack;
    std::unordered_map<std::string,SymTable> symTableList;
    //std::vector<BasicBlock> basicBlockList;
    std::unordered_map<std::string,unordered_map<int,BasicBlock>> functionList;
    BasicBlock currentBlock;
    unordered_map<int,int> instructionBlockPair;
    bool finalizeAndStartNewBlock(string currentBlockName, bool isCurrentCond, bool directFlowExist, bool dominate);
    void updateBasicBlock(BasicBlock block);
    void insertBasicBlock(BasicBlock block);
    void updateBlockForDT(int dominatingBlockNum);
    BasicBlock getBlockFromNum(int blockNum);


    int numOfBlock;

    void addFuncSymbol(SymType symType, std::string symbolName, unsigned long numOfParam);
    void addVarSymbol(std::string symbol, SymType symType, std::vector<int> arrayCapacity);
    void addParamSymbol(std::string symbol, size_t numOfParam, int index);

    int addSymInTable(){return numOfSym++;}; //Fixme: fake implementation
    Symbol symTableLookup(std::string symbol);
    void symbolTableUpdate(string var,Symbol varSym);


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
