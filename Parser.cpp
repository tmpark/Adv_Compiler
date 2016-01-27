//
// Created by Taemin Park on 1/14/16.
//

#include "Parser.h"
#include <iomanip>

Parser* Parser::_parser = 0;

Parser :: Parser()
{
//    pc = 0;
//    globalBase = 0;
    IRpc = 0;
    //FIXME
    numOfSym = 0;
}


Parser* Parser::instance() {
    if(!_parser)
        _parser = new Parser();
    return _parser;
}


RC Parser::openFile(const std::string &fileName) {
    RC rc;
    Scanner *scanner = Scanner::instance();
    rc = scanner->openFile(fileName);
    return rc;
}

RC Parser::closeFile() {
    RC rc;
    Scanner *scanner = Scanner::instance();
    rc = scanner->closeFile();
    return rc;
}

void Parser ::Next() {
    Scanner *scanner = Scanner::instance();
    scannerSym = scanner->GetSym();
}

void Parser :: startParse()
{
    Next();
    computation();
}
void Parser :: Error(std::string nonTerminal, std::initializer_list<std::string> missingTokens){
    std::cerr << "Parser error: in " << nonTerminal << ", " <<std::flush;

    for (auto i: missingTokens)
        std::cerr << "(" << i << ") "<<std::flush;

    std::cerr << "is missing" << std::endl<<std::flush;
}

/*
void Parser :: Error(std::string nonTerminal, std::string missingTerm){
        std::cerr << "Parser error: in " << nonTerminal << ", " << missingTerm << " is missing" << std::endl;
}

void Parser :: Error(std::string nonTerminal, std::string missingTerm, int numOfToken, ...){
    std::cerr << "Parser error: in " << nonTerminal << ", " << missingTerm;

    va_list tokenLists;
    va_start(tokenLists, numOfToken);
    for(unsigned i = 0 ; i < numOfToken ; i++)
    {
        int missingToken = va_arg(tokenLists, int);
        std::cerr << " or" << tokenStringMap.find(missingToken)->second;
    }
    va_end(tokenLists);

    std::cerr << " is missing" << std::endl;
}*/

//Push down automaton
void Parser::computation() {
    if(scannerSym == mainToken)
    {
        currentFunction.push("main"); //main function scope start

        Next(); //Consume main
        while(isVarDecl(scannerSym))
            varDecl();
        while(isFuncDecl(scannerSym))
            funcDecl();
        if(scannerSym == beginToken)
        {
            Next(); //Consume begin Token
            if(isStatSequence(scannerSym))
            {
                statSequence();
                if(scannerSym == endToken)
                {
                    Next(); //Consume end Token
                    if(scannerSym == periodToken) {
                        currentFunction.pop(); //end of main function
                        Next(); //Comsume period Token
                    }
                    else
                        Error("computation",{getTokenStr(periodToken)});
                }
                else
                    Error("computation",{"statSequence"});
            }
            else
                Error("computation",{"statSequence"});
        }
        else
            Error("computation",{getTokenStr(beginToken)});

    }
    else
        Error("computation",{getTokenStr(mainToken)});
}


void Parser:: funcBody(){
    while(isVarDecl(scannerSym))
    {
        varDecl();
    }
    if(scannerSym == beginToken)
    {
        Next();
        if(isStatSequence(scannerSym))
        {
            statSequence();
        }
        if(scannerSym == endToken)
        {
            Next();
        }
        else
            Error("funcBody",{getTokenStr(endToken)});
    }
    else
        Error("funcBody",{getTokenStr(beginToken)});

}


void Parser::formalParam(){
    Scanner *scanner = Scanner::instance();
    if(scannerSym == openparenToken)
    {
        Next();
        if(scannerSym == identToken)
        {
            addVarSymbol(scanner->id,varType,{}); //if declared add identifier to symbol table
            Next();
            while(scannerSym == commaToken)
            {
                Next();
                if(scannerSym == identToken)
                {
                    addVarSymbol(scanner->id,varType,{}); //if declared add identifier to symbol table
                    Next();
                }
                else
                    Error("formalParam",{getTokenStr(identToken)});
            }
        }

        if(scannerSym == closeparenToken)
        {
            Next();
        }
        else
            Error("formalParam",{getTokenStr(closeparenToken)});
    }
    else
        Error("formalParam",{getTokenStr(openparenToken)});
}

void Parser::funcDecl(){
    Scanner *scanner = Scanner::instance();
    if(scannerSym == funcToken || scannerSym == procToken)
    {
        SymType symType;
        if(scannerSym == funcToken)symType = functionType; else symType = procedureType;

        Next();
        if(scannerSym == identToken)
        {
            addFuncSymbol(scanner->id,symType); //if declared add identifier to symbol table

            currentFunction.push(scanner->id); //Current Scope set
            Next();
            if(isFormalParam(scannerSym))
                formalParam();
            if(scannerSym == semiToken)
            {
                Next();
                if(isFuncBody(scannerSym))
                {
                    funcBody();
                    if(scannerSym == semiToken)
                    {
                        currentFunction.pop(); //Go out of function
                        Next();
                    }
                    else
                        Error("funcDecl",{getTokenStr(semiToken)});
                }
                else
                    Error("funcDecl",{"funcBody"});

            }
            else
                Error("funcDecl",{getTokenStr(semiToken)});

        }
        else
            Error("funcDecl",{getTokenStr(identToken)});
    }
    else
        Error("funcDecl",{getTokenStr(funcToken),getTokenStr(procToken)});

}

void Parser::varDecl() {

    Scanner *scanner = Scanner::instance();
    if(isTypeDecl(scannerSym))
    {
        SymInfo x = typeDecl();
        if(scannerSym == identToken)
        {
            addVarSymbol(scanner->id,x.getSymType(),x.arrayCapacity);
            Next();
            while(scannerSym == commaToken)
            {
                Next(); // Consume comma
                if(scannerSym == identToken) //Array do not allow multiple declaration. So it's just variable
                {
                    addVarSymbol(scanner->id,varType,{});
                    Next();
                }
                else
                    Error("varDecl",{getTokenStr(identToken)});

            }
            if(scannerSym == semiToken)
            {
                Next();
            }
            else
                Error("varDecl",{getTokenStr(semiToken)});
        }
        else
            Error("varDecl",{getTokenStr(identToken)});
    }
    else
        Error("varDecl",{"typeDecl"});
}

SymInfo Parser::typeDecl() {

    Scanner *scanner = Scanner::instance();
    SymInfo result;
    if(scannerSym == varToken)
    {
        result.setSymType(varType); //Var type
        Next(); //Consume var Token
    }
    else if(scannerSym == arrToken)
    {
        result.setSymType(arrayType); // Array type
        Next(); //Consume var Token

        if(scannerSym == openbracketToken)
        {
            Next(); //Consume open bracket Token
            if(scannerSym == numberToken)
            {
                result.arrayCapacity.push_back(scanner->number); // first dimension capacity
                Next();
                if(scannerSym == closebracketToken)
                {
                    Next();
                    while(scannerSym == openbracketToken) {

                        Next(); //Consume open bracket Token

                        if (scannerSym == numberToken) {
                            result.arrayCapacity.push_back(scanner->number); //subsequent dimension capacity
                            Next();
                            if (scannerSym == closebracketToken) {
                                Next();
                            }
                            else
                                Error("typeDecl",{getTokenStr(closebracketToken)});
                        }
                        else
                            Error("typeDecl",{getTokenStr(numberToken)});
                    }
                }
                else
                    Error("typeDecl",{getTokenStr(closebracketToken)});
            }
            else
                Error("typeDecl",{getTokenStr(numberToken)});
        }
        else
            Error("typeDecl",{getTokenStr(openbracketToken)});
    }
    else
        Error("typeDecl",{getTokenStr(varToken),getTokenStr(arrToken)});
    return result;
}

void Parser::statSequence() {
    if(isStatement(scannerSym))
    {
        statement();
        while(scannerSym == semiToken)
        {
            Next();
            if(isStatement(scannerSym))
            {
                statement();
            }
            else
                Error("statSequence",{"statement"});
        }

    }
    else
        Error("statSequence",{"statement"});
}

void  Parser::statement()
{
    if(isAssignment(scannerSym))
    {
        assignment();
    }
    else if(isFuncCall(scannerSym))
    {
        funcCall();
    }
    else if(isIfStatement(scannerSym))
    {
        ifStatement();
    }
    else if(isWhileStatement(scannerSym))
    {
        whileStatement();
    }
    else if(isReturnStatement(scannerSym))
    {
        returnStatement();
    }
    else
        Error("statement",{"assignment","funcCall","ifStatement","whileStatement","returnStatement"});
}

void Parser::returnStatement() {
    if(scannerSym == returnToken)
    {
        Next();
        if(isExpression(scannerSym))
        {
            expression();
        }

    }
    else
        Error("returnStatement",{getTokenStr(returnToken)});
}

void Parser::whileStatement() {

    Result x,follow;
    if(scannerSym == whileToken)
    {
        Next();
        if(isRelation(scannerSym))
        {
            x = relation(); //Result is instruction with relational operator
            CondJF(x); // x.fixloc indicate that the destination should be fixed
            if(scannerSym == doToken)
            {
                Next();
                if(isStatSequence(scannerSym))
                {
                    statSequence();
                    if(scannerSym == odToken)
                    {

                        Next();
                        //Next to od token
                        follow = x;
                        UnCJF(follow); //unconditional branch to x.fixloc
                        Fixup((unsigned long)x.getFixLoc()); //fix so that while branch here
                    }
                    else
                        Error("whileStatement",{getTokenStr(odToken)});
                }
                else
                    Error("whileStatement",{"statSequence"});
            }
            else
                Error("whileStatement",{getTokenStr(doToken)});
        }
        else
            Error("whileStatement",{"relation"});
    }
    else
        Error("whileStatement",{getTokenStr(whileToken)});
}

void Parser::ifStatement() {
    Result x, follow;
    if(scannerSym == ifToken)
    {
        Next();
        if(isRelation(scannerSym))
        {
            x = relation();
            CondJF(x);
            if(scannerSym == thenToken)
            {
                Next();
                follow.setKind(instKind);
                follow.setFixLoc(0);
                if(isStatSequence(scannerSym))
                {
                    statSequence();

                    if(scannerSym == elseToken)
                    {
                        Next();
                        UnCJF(follow);
                        Fixup((unsigned long)x.getFixLoc());
                        if(isStatSequence(scannerSym))
                        {
                            statSequence();
                        }
                        else
                            Error("ifStatement",{"statSequence"});
                    }
                    else
                        Fixup((unsigned long)x.getFixLoc());
                    if(scannerSym == fiToken)
                    {
                        Next();
                        FixLink((unsigned long)follow.getFixLoc());
                    }
                    else
                        Error("ifStatement",{getTokenStr(fiToken)});
                }
                else
                    Error("ifStatement",{"statSequence"});
            }
            else
                Error("ifStatement",{getTokenStr(thenToken)});
        }
        else
            Error("ifStatement",{"relation"});
    }
    else
        Error("ifStatement",{getTokenStr(ifToken)});
}

void Parser::funcCall() {
    Scanner *scanner = Scanner::instance();
    std::string functionName;
    std::vector<Result> arguments;

    if(scannerSym == callToken)
    {
        Next();
        if(scannerSym == identToken)
        {
            functionName = scanner->id;
            Next();
            if(scannerSym == openparenToken)
            {
                Next();
                if(isExpression((scannerSym)))
                {
                    Result x;
                    x = expression();
                    arguments.push_back(x);
                    while(scannerSym == commaToken)
                    {
                        Next();
                        if(isExpression(scannerSym))
                        {
                            x = expression();
                            arguments.push_back(x);
                        }
                        else
                            Error("funcCall",{"expression"});
                    }
                }

                if(scannerSym == closeparenToken)
                {
                    //Predefined function
                    if(functionName == std::string("OutputNum"))
                    {
                        emitIntermediate(IR_write,{arguments.at(0)});
                    }
                    else if(functionName == std::string("OutputNewLine"))
                    {

                    }
                    else if(functionName == std::string("InputNum"))
                    {

                    }

                    Next();
                }
                else
                    Error("funcCall",{getTokenStr(closeparenToken)});
            }

        }
        else
            Error("funcCall",{getTokenStr(identToken)});
    }
    else
        Error("funcCall",{getTokenStr(callToken)});
}

Result Parser::assignment() {

    Result x, y, result;
    if(scannerSym == letToken)
    {
        Next();
        if(isDesignator(scannerSym))
        {
            x = designator();
            if(scannerSym == becomesToken)
            {
                Next();
                if(isExpression(scannerSym))
                {
                     y = expression();
                     result = emitIntermediate(IR_move,{y,x});
                }
                else
                    Error("assignment",{"expression"});
            }
            else
                Error("assignment",{getTokenStr(becomesToken)});
        }
        else
            Error("assignment",{"designator"});
    }
    else
        Error("assignment",{getTokenStr(letToken)});

    return result;

}

Result Parser::relation() {
    Result x,y,result;
    if(isExpression(scannerSym))
    {
        x = expression();
        if(isRelOp(scannerSym))
        {
            IROP relop = relOp();
            if(isExpression(scannerSym))
            {
                y = expression();
                result = emitIntermediate(IR_cmp,{x,y});
                result.setRelOp(relop);
            }
            else
                Error("relation",{"expression"});
        }
        else
            Error("relation",{"relOp"});
    }
    else
        Error("relation",{"expression"});
    return result;
}

Result Parser::expression() {
    Result x,y, result;
    if(isTerm(scannerSym))
    {
        x = term();
        result = x;
        while(scannerSym == plusToken || scannerSym == minusToken)
        {
            IROP irOp;if(scannerSym == plusToken)irOp = IR_add;else irOp = IR_sub; //Choose compute operation
            Next();
            if(isTerm(scannerSym))
            {
                y = term();
                result = emitIntermediate(irOp,{x,y});
            }
            else
                Error("expression",{"term"});

        }
    }
    else
        Error("expression",{"term"});

    return result;

}

Result Parser::term()
{
    Result x,y,result;
    if(isFactor(scannerSym))
    {
        x = factor();
        result = x;
        while(scannerSym == timesToken || scannerSym == divToken)
        {
            IROP irOp;if(scannerSym == timesToken)irOp = IR_mul;else irOp = IR_div; //Choose compute operation
            Next();
            if(isFactor(scannerSym))
            {
                y = factor();
                result = emitIntermediate(irOp,{x,y});
            }
            else
                Error("term",{"factor"});
        }
    }
    else
        Error("term",{"factor"});

    return result;
}

//Fixme:: function call code emission should be implemented
Result Parser::factor()
{
    Scanner *scanner = Scanner::instance();
    Result result, x;
    if(isDesignator(scannerSym))
    {
        result = designator();
    }
    else if (scannerSym == numberToken) {
        x.setKind(constKind);
        x.setConst(scanner->number);
        result = x;
        Next();
    }
    else if(scannerSym == openparenToken)
    {
        Next();
        if(isExpression(scannerSym))
        {
            result = expression();
            if(scannerSym == closeparenToken)
            {
                Next();
            }
            else
                Error("factor",{getTokenStr(closeparenToken)});
        }
        else
            Error("factor",{"expression"});
    }
    else if(isFuncCall(scannerSym))
    {
        funcCall();
    }
    else
        Error("factor",{"designator","funcCall",getTokenStr(numberToken), getTokenStr(openparenToken)});

    return result;
}

Result Parser::designator() {

    Result x,y,updatedIndex,result;
    Scanner *scanner = Scanner::instance();

    if(scannerSym == identToken)
    {
        SymInfo symInfo = symTableLookup(scanner->id);
        x.setKind(varKind);
        x.setVariable(scanner->id, var_value);

        Next();

        int i = 0;
        while(scannerSym == openbracketToken)
        {
            Next();

            if(isExpression(scannerSym))
            {
                if(i > 0) {
                    Result capacityOfArray;
                    capacityOfArray.setKind(constKind);
                    capacityOfArray.setConst(symInfo.arrayCapacity.at((unsigned long)i)); //current capacity
                    updatedIndex = emitIntermediate(IR_mul,{updatedIndex,capacityOfArray});
                }
                Result tempIndex;
                tempIndex = expression(); //z(index)

                if(i > 0)
                    updatedIndex = emitIntermediate(IR_add,{updatedIndex,tempIndex});
                else
                    updatedIndex = tempIndex;

                if(scannerSym == closebracketToken)
                {
                    Next();
                }
                else
                    Error("designator",{getTokenStr(closebracketToken)});
            }
            else
                Error("designator",{"expression"});
            i++;
        }
        if(symInfo.getSymType() == arrayType)
        {
            x.setVariableType(var_ref);//x is array. So we have to provide address of x and index

            Result indexAdjust;indexAdjust.setKind(constKind);indexAdjust.setConst(4);
            updatedIndex = emitIntermediate(IR_mul,{updatedIndex,indexAdjust}); // index * 4(word size)

            y.setKind(varKind); //Frame pointer
            y.setVariable("FP",var_value);
            y = emitIntermediate(IR_add,{y,x});//y: base address(FP + x's base)
            y = emitIntermediate(IR_adda,{updatedIndex,y});//base address + index
            result = emitIntermediate(IR_load,{y});//load value of the index
        }
        else
            result = x;
    }
    else
        Error("designator",{getTokenStr(identToken)});

    return result;
}

IROP Parser::relOp() {

    IROP relOp = IR_err;
    if(isRelOp(scannerSym))
    {
        relOp = getIRopFromToken(scannerSym);
        Next();
    }
    else
        Error("relOp",{"relOp"});
    return relOp;
}


Result Parser::emitIntermediate(IROP irOp,std::initializer_list<Result> x)
{

    Result result;
    result.setKind(instKind);
    result.setInst(IRpc);

    IRFormat ir_line;
    ir_line.setLineNo(IRpc);
    ir_line.setIROP(irOp);
    for(auto& x_i : x)
        ir_line.operands.push_back(x_i);

    IRcodes.push_back(ir_line);

    IRpc++;

    return result;
}

void Parser::printIRCodes()
{

    for(auto code : IRcodes) {


        std::string op = getIROperatorString(code.getIROP());
        std::cout << code.getLineNo() << "\t" << op << "\t";

        int index = 0;
        for(auto operand : code.operands)
        {
            if(operand.getKind() == errKind)
                break;

            if(operand.getKind() == constKind)
            {
                std::cout << operand.getConst();// << "\t";
            }
            else if (operand.getKind() == varKind)
            {
                if(operand.isReferenceVar())
                    std::cout << "&" << operand.getVariable();// << "\t"; //address of that variable
                else
                    std::cout << operand.getVariable();// << "\t";
            }
            else if(operand.getKind() == instKind) //Result of that instruction
            {
                std::cout << "(" << operand.getInst() << ")";// << "\t";
            }
            else {
                std::cerr << std::endl<< "No valid operand x"<< index << " : "<< operand.getKind() << std::endl;
            }
            if(index == 0)
                std::cout << "\t";
            index++;
        }

        std::cout << std::endl;
    }

}

void Parser::printSymbolTable()
{
    std::cout << "Symbol Name\t" << "Type\t"  << "Addresses" << std::endl;
    for(auto func : symTable)
    {
        //std::string symName = func.first;
        std::cout << func.first << "\t" << (func.second).getSymType() << "\t" << (func.second).getBaseAddr();

        /*
        for(auto sym : (func.second).varAssignedAddr)
        {
            std::cout << sym << " ";
        }
         */
        std::cout << std::endl;
    }

}


void Parser :: addFuncSymbol(std::string symbol, SymType symType)
{
    std::string encompassingFunc = currentFunction.top();
    std::string symName = encompassingFunc + ";" + symbol;
    auto symEntry = symTable.find(symName);
    if(symEntry == symTable.end())
    {
        SymInfo symInfo(symType,IRpc);
        symTable.insert({symName,symInfo});
    }
    else
    {
        std::cerr << "Same symbol already declared" << std::endl;
    }
}

void Parser::addVarSymbol(std::string symbol, SymType symType, std::vector<int> arrayCapacity)
{
    std::string encompassingFunc = currentFunction.top();
    std::string symName = encompassingFunc + ";" + symbol;
    auto symEntry = symTable.find(symName);
    if(symEntry == symTable.end())
    {
        //Fixme: Symbol Table insertion is fake
        int symAddr = addSymInTable();
        SymInfo symInfo(symType,symAddr,arrayCapacity);
        symTable.insert({symName,symInfo});
    }
    else
    {
        std::cerr << "Same symbol already declared" << std::endl;
    }
}

SymInfo Parser:: symTableLookup(std::string symbol)//Fixme: fake implementation
{
    std::stack<std::string> tempFunction = currentFunction;

    while(!tempFunction.empty()) {
        std::string symbolName = tempFunction.top() + ";" + symbol;
        auto symInfo = symTable.find(symbolName);
        if (symInfo != symTable.end())
            return symInfo->second;
        tempFunction.pop();
    }

    std::cerr << "There is no symbol like " << symbol << std::endl;
    SymInfo nullInfo;
    nullInfo.setSymType(errType);
    return nullInfo; //There is no symbol
}


void Parser :: CondJF(Result &x)
{
    Result y; //temporary blank y: later fixed up
    y.setKind(constKind);
    y.setConst(0);
    emitIntermediate(negateCondition(x.getRelOp()),{x,y});
    x.setFixLoc(IRpc -1);
}

void Parser :: UnCJF(Result &x)
{
    Result temp1;
    temp1.setKind(constKind);
    temp1.setConst(0);

    Result temp2;
    temp2.setKind(constKind);
    temp2.setConst(0);

    Result resultInstr = emitIntermediate(IR_cmp,{temp1,temp2});
    temp2.setConst(x.getFixLoc());
    emitIntermediate(IR_beq, {resultInstr, temp2});
    x.setFixLoc(IRpc  - 1);
}

//Fixme: some line is skipped depending on this code
void Parser ::  Fixup(unsigned long loc)
{
    IRFormat operationToChange = IRcodes.at(loc);
    operationToChange.operands.at(1).setKind(constKind);
    operationToChange.operands.at(1).setConst(IRpc);
    IRcodes.at(loc) = operationToChange;
}

//Fixme: until 0 is not valid. another mechanism needed
void Parser :: FixLink(unsigned long loc)
{
    unsigned long next;
    while(loc != 0)
    {
        IRFormat operationToChange = IRcodes.at(loc);
        next = (unsigned long)operationToChange.operands.at(1).getConst();
        Fixup(loc);
        loc = next;
    }
}

/*
void Parser::PutF1(int op, int a, int b, int c) {
    buf.at(pc) = op << 26 |
                a << 21 |
                b << 16 |
                c & 0x0000FFFF;
    pc++;

}

//value of x or value in address of x will be stored in a register
void Parser::Load(Result &x)
{
    if(x.kind == constKind)
    {
        x.regNo = AllocateReg();
        //Emit code for (value in reg0 + x.value -> value in x.regNo)
        PutF1(ADDI_OP, x.regNo,0,x.value);
        x.kind = regKind;
    }
    else if(x.kind == varKind)
    {
        x.regNo = AllocateReg();
        //Emit code for (value in Mem[globalBase + x.address] -> value in x.regNo)
        PutF1(LDW_OP,x.regNo,globalBase,x.address);
    }
}


int Parser:: AllocateReg()
{
    int index = 0;
    for(auto& reg : regsBusy)
    {
        if(reg == false)
        {
            reg = true;
            return index;
        }
        index++;

    }
    return -1;//Not enough register
}

void Parser:: DeAllocateReg(int regNum)
{
    regsBusy.at(regNum) = false;
}

//the computed result will be returned in x
//Result could be constant or register
void Parser:: Compute(TokenType computeToken, Result &x, Result &y) {

    int op = 0;

    if(computeToken == plusToken)
        op = ADD_OP;
    else if (computeToken == minusToken)
        op = SUB_OP;
    else if (computeToken == timesToken)
        op = MUL_OP;
    else if (computeToken == divToken)
        op = DIV_OP;
    else {
        std::cerr << "Only+,-,*,/ token can be allowed" << std::endl;
        return;
    }

    if (x.kind == constKind && y.kind == constKind)
    {
        if (op == ADD_OP)
            x.value += y.value;
        else if(op == SUB_OP)
            x.value -= y.value;
        else if(op == MUL_OP)
            x.value *= y.value;
        else if(op == DIV_OP)
            x.value /= y.value;
    }
    else if(y.kind == constKind)
    {
        Load(x); //value of x will be stored in a register
        PutF1(lmmOp((Opcode)op),x.regNo, x.regNo, y.value);
    }
        //
    else
    {
        Load(x);Load(y);
        PutF1(op,x.regNo,x.regNo,y.regNo);
        DeAllocateReg(y.regNo);
    }

}*/