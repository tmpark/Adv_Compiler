//
// Created by Taemin Park on 1/11/16.
//

#ifndef ADV_COMPILER_HELPER_H
#define ADV_COMPILER_HELPER_H

#define SOURCE_CODE_PRINT false
#define TraceScan false
#define NO_PARSE false

#define RETURN_IN_STACK -1
#define PRE_FP_IN_STACK 0
#define PARAM_IN_STACK 1
#define LOCAL_IN_STACK -1
#define REG_0 0
#define REG_TEMP 24
#define REG_PROXY 25
#define NUM_OF_PROXY_REG 2
#define REG_RET_VAL 27
#define REG_FP 28
#define REG_SP 29
#define REG_GP 30
#define REG_RET 31
#define REG_VIRTUAL 32
#define REG_CALLER_SAVED 1
#define REG_DATA 4
#define REG_PARAM 1
#define NUM_OF_PARAM_REGS 3
#define NUM_OF_CALLER_SAVED 4
#define REG_CALLEE_SAVED 5
#define NUM_OF_CALlEE_SAVED 4
#define NUM_OF_DATA_REGS 5
#define MAX_NUMS_OF_REGS 128
#define GLOBAL_SCOPE_NAME "main"




#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <iostream>
#include <memory>
#include <stack>

using namespace std;

typedef int RC;

typedef enum {
    errToken = 0,
    timesToken = 1, divToken = 2,
    plusToken = 11, minusToken = 12,
    eqlToken = 20, neqToken = 21, lssToken = 22, geqToken = 23, leqToken = 24, gtrToken = 25,
    periodToken = 30, commaToken = 31, openbracketToken = 32, closebracketToken = 33, closeparenToken = 35,
    becomesToken = 40, thenToken = 41, doToken = 42,
    openparenToken = 50,
    numberToken = 60, identToken = 61,
    semiToken = 70,
    endToken = 80, odToken = 81, fiToken = 82,
    elseToken = 90,
    letToken = 100, callToken = 101, ifToken = 102, whileToken = 103, returnToken = 104,
    varToken = 110, arrToken = 111, funcToken = 112, procToken = 113,
    beginToken = 150, mainToken = 200, eofToken = 255, commentToken
}TokenType;

typedef enum{
    OP_F1, OP_F2, OP_F3
}OPFORMAT;


static std::unordered_map<int,std::string> tokenStringMap = {{errToken,       "error"}, {timesToken, "*"}, {divToken, "/"},
                                                             {plusToken,      "+"}, {minusToken, "-"}, {eqlToken, "=="}, {neqToken, "!="},
                                                             {lssToken,       "<"}, {geqToken, ">="}, {leqToken, "<="}, {gtrToken, ">"},
                                                             {periodToken,    "."}, {commaToken, ","}, {openbracketToken, "["}, {closebracketToken, "]"}, {closeparenToken, ")"},
                                                             {becomesToken,   "<-"}, {thenToken, "then"}, {doToken, "do"},
                                                             {openparenToken, "("}, {numberToken, "number"}, {identToken, "identifier"}, {semiToken, ";"},
                                                             {endToken,       "}"}, {odToken, "od"}, {fiToken, "fi"}, {elseToken,"else"},
                                                             {letToken,"let"}, {callToken, "call"}, {ifToken, "if"}, {whileToken,"while"}, {returnToken,"return"},
                                                             {varToken,"var"}, {arrToken,"array"}, {funcToken, "function"}, {procToken,"procedure"},
                                                             {beginToken,"{"}, {mainToken,"main"}, {eofToken,"end of file"}, {commentToken,"comment"}};


std::string getTokenStr(TokenType token);


static std::unordered_map<std::string,TokenType> reservedWord = {{"then",thenToken},{"do",doToken},{"od",odToken},
                                  {"fi",fiToken},{"else",elseToken},{"let",letToken},
                                  {"call",callToken},{"if",ifToken},{"while",whileToken},
                                  {"return",returnToken},{"var",varToken},{"array",arrToken},
                                  {"function",funcToken},{"procedure",procToken},{"main",mainToken}};

typedef enum{

    OP_ADD = 0, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_CMP,
    OP_OR = 8, OP_AND, OP_BIC, OP_XOR, OP_LSH, OP_ASH, OP_CHK,
    OP_ADDI = 16, OP_SUBI, OP_MULI, OP_DIVI, OP_MODI, OP_CMPI,
    OP_ORI = 24, OP_ANDI, OP_BICI, OP_XORI, OP_LSHI, OP_ASHI, OP_CHKI,
    OP_LDW = 32, OP_LDX, OP_POP,
    OP_STW = 36, OP_STX, OP_PSH,
    OP_BEQ = 40, OP_BNE, OP_BLT, OP_BGE, OP_BLE, OP_BGT, OP_BSR,
    OP_JSR = 48, OP_RET, OP_RDD, OP_WRD, OP_WRH, OP_WRL, OP_ERR
} OpCode;

typedef enum{
    IR_err, IR_neg, IR_add, IR_sub, IR_mul, IR_div, IR_cmp, IR_adda, IR_load, IR_store, IR_move, IR_phi, IR_miu, IR_end,
    IR_bra, IR_bne, IR_beq, IR_ble, IR_blt, IR_bge, IR_bgt, IR_read, IR_write, IR_writeNL
}IROP;

typedef enum{
    edge_normal, edge_true, edge_false
}EDGETYPE;

typedef enum{
    graph_CFG, graph_DT, graph_IG
}GRAPHTYPE;



typedef enum{
    errKind,constKind,varKind,instKind,blockKind,regKind,reloadKind
}Kind;

typedef enum{
    blk_entry, blk_while_cond,blk_while_body,blk_while_end, blk_if_then, blk_if_else, blk_if_end, blk_move
}BlockKind;

typedef enum{
    sym_err, sym_var, sym_param, sym_array, sym_func, sym_proc
}SymType;



class IRFormat;
class Symbol;
class Result;
class DefinedInfo;



class IRFormat
{
public:
    IRFormat(){blkNo = -1; instNo = -1; ir_op = IR_err;previousSameOpInst = NULL;cost = 0;regNo = -1;elimiated = false;};
    /*
    IRFormat(int blkNo, int instNo, IROP ir_op, Result operand0)
    {
        this->blkNo = blkNo; this->instNo = instNo;this->ir_op = ir_op, operands = {operand0};previousSameOpInst = NULL;cost = 0;regNo = -1;;elimiated = false;
    };
    IRFormat(int blkNo, int instNo, IROP ir_op, Result operand0, Result operand1)
    {
        this->blkNo = blkNo;this->instNo = instNo;this->ir_op = ir_op, operands = {operand0,operand1};previousSameOpInst = NULL;cost = 0;regNo = -1;;elimiated = false;
    };
    IRFormat(int blkNo, int instNo, IROP ir_op, Result operand0, Result operand1, Result operand2)
    {
        this->blkNo = blkNo;this->instNo = instNo;this->ir_op = ir_op, operands = {operand0,operand1,operand2};previousSameOpInst = NULL;cost = 0;regNo = -1;;elimiated = false;
    };*/

    void setBlkNo(int arg){blkNo = arg;};
    int getBlkNo(){return blkNo;};

    void setLineNo(int arg){ instNo = arg;};
    int getLineNo(){return instNo;};

    void setIROP(IROP arg){ir_op = arg;};
    IROP getIROP(){return ir_op;};

    void setCost(int arg){cost = arg;};
    int getCost(){return cost;};

    shared_ptr<IRFormat>  getPreviousSameOpInst(){return previousSameOpInst;};
    void setPreviousSameOpInst(shared_ptr<IRFormat> arg){previousSameOpInst = arg;};
    std::vector<Result> operands;

    void setRegNo(int arg){regNo = arg;};
    int getRegNo(){return regNo;};

    bool isElimiated(){return elimiated;};
    void eliminate(){ elimiated = true;};

private:
    bool elimiated;
    int blkNo;
    int instNo;
    int regNo;
    IROP ir_op;
    int cost;
    shared_ptr<IRFormat> previousSameOpInst;

};

class Symbol
{

public:
    Symbol(){ symType = sym_err; cost = 0;
        numOfParam = 0;paramIndex = -1;};
    Symbol(SymType symType, int loc)
    { this->symType = symType;this->symBaseAddr = loc;
        numOfParam = 0;cost = 0;paramIndex = -1;};
    Symbol(SymType symType, int loc, vector<int> arrayCapacity)
    { this->symType = symType; this->symBaseAddr = loc; this->arrayCapacity = arrayCapacity;
        numOfParam = 0;cost = 0;paramIndex = -1;};
    Symbol(SymType symType, int loc, int numOfParam) //For fucntion symbol
    { this->symType = symType; this->symBaseAddr = loc;
        this->numOfParam = numOfParam;cost = 0;paramIndex = -1;};
    void setSymType(SymType arg){symType = arg;};
    SymType getSymType(){return symType;};
    void setBaseAddr(int arg){ symBaseAddr = arg;};
    int getBaseAddr(){return symBaseAddr;};
    void setNumOfParam(int arg){ this->numOfParam = numOfParam;};
    int getNumOfParam(){ return numOfParam;};
    void setCost(int arg){cost = arg;};
    int getCost(){return cost;};
    void setReg(string arg1, int arg2){regList.insert({arg1,arg2});};
    int getRegNo(string arg){
        auto regListIter = regList.find(arg);
        if(regListIter == regList.end())
            return -1;
        else
            return regListIter->second;};
    void setParamIndex(int arg){paramIndex = arg;};
    int getParamIndex(){return paramIndex;};

    std::vector<int> arrayCapacity; //only for array : capacity and dimension

    //std::vector<int> symAssignedInst; //only for variable assigned information

private:
    SymType symType; //var, array, function, procedure
    int symBaseAddr; //Location of symbol
    int paramIndex;
    int numOfParam; //Only for function
    int cost; //Only for var
    unordered_map<string,int> regList;
};




class DefinedInfo
{
public:
    DefinedInfo(){blkNum = -1;kind = errKind;instNum = -1;definedInst = -1;constVal = -1;locked = false;};
    DefinedInfo(int blkNum, string symbol){ this->blkNum = blkNum;this->symbol = symbol;
        locked = false;};
    void setInst(int arg1, shared_ptr<IRFormat> arg2){kind = instKind; instNum = arg1; inst = arg2;};
    int getInstNum(){return instNum;};
    shared_ptr<IRFormat> getInst(){return inst;};
    void setVar(string arg0, shared_ptr<Symbol> arg1, int arg2){ kind = varKind;var = arg0;definedInst = arg2;varSym = arg1;};
    string getVar(){return var;};
    int getDefinedInstOfVar(){return definedInst;};
    void setConst(int arg){ kind = constKind; constVal = arg;};
    void setReload(){kind = reloadKind;};
    int getConst(){return constVal;};
    Kind getKind(){return kind;};
    int getBlkNum(){return blkNum;};
    void setlock(bool arg){ locked = arg;};
    bool isLocked(){return locked;};
    shared_ptr<Symbol> getVarSym(){return varSym;};
//    int getAllocatedReg(){
//        if(kind == instKind)
//            return inst->getRegNo();
//        else if(kind == varKind)
//            return varSym->getRegNo();
//    };

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
    bool locked;
};

class SymTable
{
public:
    SymTable(){parentSymTableName = "";
        localVarSize = 0;numOfSymVar= 0;numOfSymParam = 0;numOfVirtualRegs=0;};
    SymTable(string parentSymTableName)
    { this->parentSymTableName = parentSymTableName;
        localVarSize = 0;numOfSymVar= 0;numOfSymParam = 0;numOfVirtualRegs=0;};
    void setParent(string arg){this->parentSymTableName = arg;};
    string getParent(){return parentSymTableName;};
    void insertVarSym(string symName, shared_ptr<Symbol> sym){
        varSymbolList.insert({symName,sym});
        if(sym->getSymType() == sym_var)
            numOfSymVar++;
        if(sym->getSymType() == sym_param)
            numOfSymParam++;
    };
    int getNumOfVar(){return numOfSymVar;};
    int getNumOfParam(){return numOfSymParam;};

    unordered_map<string,shared_ptr<Symbol>> varSymbolList;
    unordered_map<string,shared_ptr<Symbol>> funcSymbolList;
    int getLocalVarSize(){return localVarSize;};
    void setLocalVarSize(int arg){ localVarSize = arg;};
    int getNumOfVirtualRegs(){return numOfVirtualRegs;};
    void setNumOfVirtualRegs(int arg){numOfVirtualRegs = arg;};
    vector<int> regUsedList;
    unordered_map<string,DefinedInfo> definedGlobalVal;

private:
    string parentSymTableName;
    int numOfSymVar;
    int numOfSymParam;
    int localVarSize;
    int numOfVirtualRegs;
};



typedef struct{
    shared_ptr<Symbol> symbol;
    DefinedInfo defInfo;
}GlobalDefInfo;


class Result{
public:
    Result(){kind = errKind; value = -1; variable = ""; relOp = IR_err; fixLoc = -1;defInst = -1;array = false;
        diffFuncLoc = false;jumpLoc = 0;returnAddr = false;};
    void setKind(Kind arg){kind = arg;};
    Kind getKind(){return kind;};
    //Const
    void setConst(int arg){kind = constKind;value = arg;};
    int getConst(){return value;};
    //Variable
    void setVariable(string arg1, shared_ptr<Symbol> arg2){kind = varKind; variable = arg1;varSym = arg2;};
    string getVariableName(){return variable;};
    shared_ptr<Symbol> getVarSym(){return varSym;};
    //Instruction
    void setInst(shared_ptr<IRFormat> arg){kind = instKind;inst = arg;};
    shared_ptr<IRFormat> getInst(){return inst;};
    void setBlock(int arg){kind = blockKind; blockNo = arg;};
    int getBlockNo(){return blockNo;};
    void setRelOp(IROP arg){relOp = arg;}; //Only for relation
    IROP getRelOp(){return relOp;};  //Only for relation
    void setFixLoc(int arg){fixLoc = arg;};//Only for relation
    int getFixLoc(){return fixLoc;};//Only for relation
    void setReg(int arg){kind = regKind; regNo = arg;};
    int getReg(string arg){//argument is only for RegisterAllocation
        if(kind == varKind)
            return varSym->getRegNo(arg);
        else if(kind == instKind)
            return inst->getRegNo();
        else
            return regNo;};
    void setDefInst(int arg){defInst = arg;};
    int getDefInst(){return defInst;};
    void setArrayInst(string arrayVarName){array = true;variable = arrayVarName;};
    bool isArrayInst(){return array;};
    void setConstPropVar(string arg){constPropVar = arg;};
    string getConstPropVar(){return constPropVar;};
    void setDiffFuncLoc(string name, shared_ptr<Symbol> sym){ diffFuncLoc = true; funcName = name; funcSym = sym;};
    bool isDiffFuncLoc(){return diffFuncLoc;};
    string getDiffFuncName(){return funcName;};
    bool hasReturnVal(){
        if(functionType == sym_func)return true;
        else if(functionType == sym_proc)
            return false;
    };
    void setFunctionType(SymType arg){functionType = arg;};
    unordered_map<string,GlobalDefInfo> globalDefInfo;//Only for branch location to other function
    void setJumpLoc(int arg){jumpLoc = arg;};
    int getJumpLoc(){return jumpLoc;};
    void setReturnAddr(){returnAddr = true;};
    bool isReturnAddr(){return returnAddr;};

private:
    Kind kind;
    //constKind
    int value;
    string constPropVar;
    //varKind
    std::string variable;
    shared_ptr<Symbol> varSym;
    int defInst;
    //instKind
    shared_ptr<IRFormat> inst;
    bool array;
    int blockNo;
    bool diffFuncLoc;
    int jumpLoc;
    bool returnAddr;
    SymType functionType;
    string funcName;
    shared_ptr<Symbol> funcSym;

    IROP relOp ;
    int fixLoc;
    int regNo;
};



TokenType getTypeOfOneToken(char c);
void printToken(TokenType tokenTypeReturned,std::string idConstructed);

bool isRelOp(TokenType scannerSym);
bool isDesignator(TokenType scannerSym);
bool isFactor(TokenType scannerSym);
bool isTerm(TokenType scannerSym);
bool isExpression(TokenType scannerSym);
bool isRelation(TokenType scannerSym);
bool isAssignment(TokenType scannerSym);
bool isFuncCall(TokenType scannerSym);
bool isIfStatement(TokenType scannerSym);
bool isWhileStatement(TokenType scannerSym);
bool isReturnStatement(TokenType scannerSym);
bool isStatement(TokenType scannerSym);
bool isStatSequence(TokenType scannerSym);
bool isTypeDecl(TokenType scannerSym);
bool isVarDecl(TokenType scannerSym);
bool isFuncDecl(TokenType scannerSym);
bool isFormalParam(TokenType scannerSym);
bool isFuncBody(TokenType scannerSym);
OpCode lmmOp(OpCode op);
std::string getIROperatorString(IROP irOp);
IROP getIRopFromToken(TokenType scannerSym);
IROP negateCondition(IROP ir_op);
std::vector<std::string> splitString(std::string stringToSplit);
bool isBranchCond(IROP op);
bool isSameOperand(Result x, Result y);
bool isInnerBlock(BlockKind blkKind);
bool isDefInstr(shared_ptr<IRFormat> code);
OpCode irToOp(IROP irOp, OPFORMAT opFormat);
void getInfoForCodeGen(OpCode opCode,int &numOfArgs, OPFORMAT &opFormat);
#endif //ADV_COMPILER_HELPER_H
