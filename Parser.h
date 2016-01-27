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
#include "Scanner.h"

using namespace :: std;


typedef enum{
    errKind,constKind,varKind,instKind
}Kind;

typedef enum{
    errType, varType, arrayType, functionType, procedureType
}SymType;

typedef enum{
    var_err, var_ref, var_value
} VarType;


class Result{
public:
    Result(){kind = errKind; value = -1; variable = ""; var_type = var_err; instNo = -1; relOp = IR_err, fixLoc = -1;};
    void setKind(Kind arg){kind = arg;};
    Kind getKind(){return kind;};
    //Const
    void setConst(int arg){value = arg;};
    int getConst(){return value;};
    //Variable
    void setVariable(string arg1, VarType arg2){ variable = arg1;var_type = arg2;};
    string getVariable(){return variable;};
    bool isReferenceVar(){return (var_type == var_ref);};
    void setVariableType(VarType arg){var_type = arg;};
    //Instruction
    void setInst(int arg){instNo = arg;};
    int getInst(){return instNo;};
    void setRelOp(IROP arg){relOp = arg;}; //Only for relation
    IROP getRelOp(){return relOp;};  //Only for relation
    void setFixLoc(int arg){fixLoc = arg;};//Only for relation
    int getFixLoc(){return fixLoc;};//Only for relation
private:
    Kind kind;
    //constKind
    int value;
    //varKind
    std::string variable;
    VarType var_type;
    //instKind
    int instNo;
    IROP relOp ;
    int fixLoc;
};

class SymInfo
{

public:
    SymInfo(){symType = errType; arrayCapacity = {}; varAssignedAddr = {};}
    SymInfo(SymType symType, int baseAddr){this->symType = symType; this->baseAddr = baseAddr;};
    SymInfo(SymType symType, int baseAddr, vector<int> arrayCapacity){this->symType = symType; this->baseAddr = baseAddr; this->arrayCapacity = arrayCapacity;};
    void setSymType(SymType arg){symType = arg;};
    SymType getSymType(){return symType;};
    void setBaseAddr(int arg){baseAddr = arg;};
    int getBaseAddr(){return baseAddr;};

    std::vector<int> arrayCapacity; //only for array : capacity and dimension
    std::vector<int> varAssignedAddr; //only for variable assigned information

private:
    SymType symType; //var, array, function, procedure
    int baseAddr; //Location of symbol

};

class IRFormat
{
public:
    IRFormat(){lineNo = -1; ir_op = IR_err;}

    void setLineNo(int arg){lineNo = arg;};
    int getLineNo(){return lineNo;};

    void setIROP(IROP arg){ir_op = arg;};
    IROP getIROP(){return ir_op;};
    std::vector<Result> operands;

private:
    int lineNo;
    IROP ir_op;

};

class Parser {

public:
    Parser();
    static Parser* instance();
    RC openFile(const std::string &fileName);
    void startParse();
    RC closeFile();
    void printIRCodes();
    void printSymbolTable();

private:

    //Error message
    void Error(std::string nonTerminal, std::initializer_list<std::string> missingTokens);

    //Next token for a explicit token
    void Next();

    //Non terminals
    void computation();
    void funcBody();
    void formalParam();
    void funcDecl();
    void varDecl();
    SymInfo typeDecl();
    void statSequence();
    void statement();
    void returnStatement();
    void whileStatement();
    void ifStatement();
    void funcCall();
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


    int IRpc;
    std::vector<IRFormat> IRcodes;
    std::stack<std::string> currentFunction;
    std::unordered_map<std::string,SymInfo> symTable;
    void addFuncSymbol(std::string symbol, SymType symType);
    void addVarSymbol(std::string symbol, SymType symType, std::vector<int> arrayCapacity);

    int addSymInTable(){return numOfSym++;}; //Fixme: fake implementation
    SymInfo symTableLookup(std::string symbol);


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
