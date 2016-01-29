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

#include <array>
#include <vector>
#include <stack>
#include <map>
#include <memory>
#include "Scanner.h"

using namespace :: std;


typedef enum{
    errKind,constKind,varKind,instKind
}Kind;

typedef enum{
    errType, varType, arrayType,paramType, functionType, procedureType
}SymType;

typedef enum{
    var_err, var_ref, var_value
} VarType;


class Result{
public:
    Result(){kind = errKind; value = -1; variable = ""; var_type = var_err; instNo = -1; relOp = IR_err, fixLoc = -1;}
    void setKind(Kind arg){kind = arg;};
    Kind getKind(){return kind;};
    //Const
    void setConst(int arg){kind = constKind;value = arg;};
    int getConst(){return value;};
    //Variable
    void setVariable(string arg1, VarType arg2){kind = varKind; variable = arg1;var_type = arg2;};
    string getVariable(){return variable;};
    bool isReferenceVar(){return (var_type == var_ref);};
    void setVariableType(VarType arg){var_type = arg;};
    //Instruction
    void setInst(int arg){kind = instKind; instNo = arg;};
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



class Symbol
{

public:
    Symbol(){ symType = errType; arrayCapacity = {}; symAssignedInst = {};
        numOfParam = 0;}
    Symbol(SymType symType, int loc)
    { this->symType = symType;this->symLoc = loc;
        numOfParam = 0;};
    Symbol(SymType symType, int loc, vector<int> arrayCapacity)
    { this->symType = symType; this->symLoc = loc; this->arrayCapacity = arrayCapacity;
        numOfParam = 0;};
    Symbol(SymType symType, int loc, int numOfParam) //For fucntion symbol
    { this->symType = symType; this->symLoc = loc; this->numOfParam = numOfParam;};
    void setSymType(SymType arg){symType = arg;};
    SymType getSymType(){return symType;};
    void setLocation(int arg){ symLoc = arg;};
    int getLocation(){return symLoc;};
    void setNumOfParam(int arg){ this->numOfParam = numOfParam;};
    int getNumOfParam(){ return numOfParam;};

    std::vector<int> arrayCapacity; //only for array : capacity and dimension
    std::vector<int> symAssignedInst; //only for variable assigned information

private:
    SymType symType; //var, array, function, procedure
    int symLoc; //Location of symbol
    int numOfParam; //Only for function

};


class SymTable
{
public:
    SymTable(){parentSymTableName = "";};
    SymTable(string parentSymTableName)
    { this->parentSymTableName = parentSymTableName;};
    void setParent(string arg){this->parentSymTableName = arg;};
    string getParent(){return parentSymTableName;};
    unordered_map<string,Symbol> symbolList;
private:
    string parentSymTableName;
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
    void LoadParam(unsigned long numOfParam, int paramIndex, Result param);
    void predefinedFunc();
    Result getAddressInStack(int location);


    int IRpc;
    std::vector<IRFormat> IRcodes;
    std::stack<std::string> scopeStack;
    //std::unordered_map<std::string,Symbol> symTable;
    std::unordered_map<std::string,SymTable> symTableList;
    void addFuncSymbol(SymType symType, std::string symbolName, unsigned long numOfParam);
    void addVarSymbol(std::string symbol, SymType symType, int loc, std::vector<int> arrayCapacity);

    int addSymInTable(){return numOfSym++;}; //Fixme: fake implementation
    Symbol symTableLookup(std::string symbol);


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
