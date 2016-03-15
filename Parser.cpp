//
// Created by Taemin Park on 1/14/16.
//

#include "Parser.h"
#include "Helper.h"
#include <iomanip>

Parser* Parser::_parser = 0;

Parser :: Parser()
{
//    pc = 0;
//    globalBase = 0;
    IRpc = 0;
    //FIXME
    numOfSym = 0;
    numOfBlock = 0;
    loopDepth = 0;
    currentBlock = make_shared<BasicBlock> (0,blk_entry);
}


Parser* Parser::instance() {
    if(!_parser)
        _parser = new Parser();
    return _parser;
}


RC Parser::openFile(const std::string &folder, const std::string &sourceFileName, const std::string &sourceFileFormat) {
    RC rc;
    fileName = sourceFileName;
    Scanner *scanner = Scanner::instance();
    rc = scanner->openFile(folder + sourceFileName + sourceFileFormat);
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
    std::cerr << fileName << ": Parser error in " << nonTerminal << ", " <<std::flush;

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

    SymTable newSymTable;
    symTableList.insert({"main",newSymTable});
    //Result tempJumpLoc;tempJumpLoc.setConst(0);
    //Default block for jump to the main
    //finalizeAndStartNewBlock(false);

    if(scannerSym == mainToken)
    {
        scopeStack.push("main"); //main function scope start
        vector<shared_ptr<BasicBlock>> emptyBasicBlockList;
        functionList.insert({"main",emptyBasicBlockList});

        Next(); //Consume main
        while(isVarDecl(scannerSym))
            varDecl();
        predefinedFunc(); //Predefined functions
        while(isFuncDecl(scannerSym)) {
            //At the start of block in function, the start block dominates itself
            stack<int> dominatedBy;
            dominatedBy.push(currentBlock->getBlockNum());
            dominatedByInfo.insert({currentBlock->getBlockNum(),dominatedBy});
            funcDecl();
        }
        if(scannerSym == beginToken)
        {
            //At the start of block in main, the start block dominates itself
            stack<int> dominatedBy;
            dominatedBy.push(currentBlock->getBlockNum());
            dominatedByInfo.insert({currentBlock->getBlockNum(),dominatedBy});
            ssaBuilder = SSABuilder("main", currentBlock->getBlockNum(), IRpc);
            cseTracker = CSETracker();
            //Fixup(0);//Fix bra to first reach out here
            Next(); //Consume begin Token
            if(isStatSequence(scannerSym))
            {
                statSequence();
                if(scannerSym == endToken)
                {
                    Next(); //Consume end Token
                    if(scannerSym == periodToken) {
                        emitIntermediate(IR_end,{});
                        finalizeAndStartNewBlock(blk_entry, false,false,false);
                        scopeStack.pop(); //end of main function
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


vector<string> Parser::formalParam(){

    Scanner *scanner = Scanner::instance();
    vector<string> parameters;
    if(scannerSym == openparenToken)
    {
        Next();
        if(scannerSym == identToken)
        {
            parameters.push_back(scanner->id);
            Next();
            while(scannerSym == commaToken)
            {
                Next();
                if(scannerSym == identToken)
                {
                    parameters.push_back(scanner->id);
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
    return parameters;
}

void Parser::funcDecl(){
    Scanner *scanner = Scanner::instance();

    if(scannerSym == funcToken || scannerSym == procToken)
    {
        SymType symType;
        if(scannerSym == funcToken)symType = sym_func; else symType = sym_proc;

        Next();
        if(scannerSym == identToken)
        {
            string symName = scanner->id;
            vector<shared_ptr<BasicBlock>> emptyBasicBlockList;
            functionList.insert({symName,emptyBasicBlockList}); //New function inserted in function list

            Next();
            vector<string> parameters;
            if(isFormalParam(scannerSym))
                parameters = formalParam();
            if(scannerSym == semiToken)
            {
                Next();
                addFuncSymbol(symType,symName,parameters.size()); //if declared add identifier to symbol table
                if(isFuncBody(scannerSym))
                {
                    scopeStack.push(symName); //Current Scope set
                    ssaBuilder = SSABuilder(symName, currentBlock->getBlockNum(), IRpc);
                    cseTracker = CSETracker();
                    //Parameters become local variable

                    int index = 0;//index in stack
                    for(auto param : parameters)
                    {
                        addParamSymbol(param,parameters.size(),index);//add symbol table
                        index++;
                    }

                    funcBody();

                    if(symType == sym_proc) //Procedure type does not have explicit return
                    {
                        //Return code emission
                        //Result offset = getAddressInStack(RETURN_IN_STACK);
                        //emitIntermediate(IR_bra,{offset});
                    }

                    //At the end of function means end of the block
                    finalizeAndStartNewBlock(blk_entry, false,false,false);

                    //give information that defined inst of global variable
                    SymTable globalSymTable = symTableList.at(GLOBAL_SCOPE_NAME);
                    for(auto symIter : globalSymTable.varSymbolList)
                    {
                        string globalSymName = symIter.first;

                        SymTable currentSymTable = symTableList.at(symName);
                        auto currentVarSymIter = currentSymTable.varSymbolList.find(globalSymName);
                        if(currentSymTable.varSymbolList.end() != currentVarSymIter)//If local variable has the same name with a global variable -> skip it
                            continue;

                        shared_ptr<Symbol> globalSym = symIter.second;
                        if(globalSym->getSymType() == sym_var)
                        {
                            DefinedInfo globalDefInfo = ssaBuilder.getDefinedInfo(globalSymName);
                            Kind defKind = globalDefInfo.getKind();
                            if(defKind != errKind && defKind != reloadKind) //There was definition for global variable
                            {
                                SymTable *symTable = &symTableList.at(symName);
                                symTable->definedGlobalVal.insert({symIter.first,globalDefInfo});
                            }
                        }

                    }

                    scopeStack.pop(); //Go out of function

                    if(scannerSym == semiToken)
                    {
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
        shared_ptr<Symbol> x = typeDecl();
        if(scannerSym == identToken)
        {
            addVarSymbol(scanner->id,x->getSymType(),x->arrayCapacity);
            Next();
            while(scannerSym == commaToken)
            {
                Next(); // Consume comma
                if(scannerSym == identToken) //Array do not allow multiple declaration. So it's just variable
                {
                    addVarSymbol(scanner->id,x->getSymType(),x->arrayCapacity);
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

shared_ptr<Symbol> Parser::typeDecl() {

    Scanner *scanner = Scanner::instance();
    shared_ptr<Symbol> result = make_shared<Symbol>();
    if(scannerSym == varToken)
    {
        result->setSymType(sym_var); //Var type
        Next(); //Consume var Token
    }
    else if(scannerSym == arrToken)
    {
        result->setSymType(sym_array); // Array type
        Next(); //Consume var Token

        if(scannerSym == openbracketToken)
        {
            Next(); //Consume open bracket Token
            if(scannerSym == numberToken)
            {
                result->arrayCapacity.push_back(scanner->number); // first dimension capacity
                Next();
                if(scannerSym == closebracketToken)
                {
                    Next();
                    while(scannerSym == openbracketToken) {

                        Next(); //Consume open bracket Token

                        if (scannerSym == numberToken) {
                            result->arrayCapacity.push_back(scanner->number); //subsequent dimension capacity
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
    Result x;
    if(scannerSym == returnToken)
    {
        Next();
        if(isExpression(scannerSym))
        {
            x = expression();
            Result ret;ret.setReg(REG_RET_VAL);
            emitIntermediate(IR_miu,{x,ret});
            Result returnAddr;returnAddr.setReg(REG_RET);returnAddr.setReturnAddr();
            emitIntermediate(IR_bra,{returnAddr});
            //Result offset = getAddressInStack(RETURN_IN_STACK);
            //Result x = emitIntermediate(IR_load,{offset});
            //emitIntermediate(IR_bra,{x});
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
            //CSE should include loads in while body(actually after blk_while_cond)
            ssaBuilder.currentBlockKind.push(blk_while_body);

            //for block of which outer block is inner block
            BlockKind outerBlockKind = currentBlock->getOuterBlockKind();

            int cseRevertBlock = currentBlock->getBlockNum();
            //At the start of while, new Block starts
            if(!finalizeAndStartNewBlock(blk_while_cond, false, true,true)) //Already New block is made(just change the name of block
                currentBlock->setBlockKind(blk_while_cond);
            currentBlock->setOuterBlockKind(outerBlockKind);

            int dominatingBlockNum = currentBlock->getBlockNum();
            int conditionBlockNum = dominatingBlockNum;

            loopDepth++;//From condition block, instructions will loop
            x = relation(); //Result is instruction with relational operator
            CondJF(x); // x.fixloc indicate that the destination should be fixed

            ssaBuilder.startJoinBlock(currentBlock->getBlockKind(),currentBlock->getBlockNum());
            //After the condition, also new block starts
            finalizeAndStartNewBlock(blk_while_body, true, true,true); //while block automatically dominated by cond block
            ssaBuilder.protectDef();

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
                        follow.setFixLoc(follow.getFixLoc());//jump one instruction more because while should check condition for every iteration
                        //cmp <- follow.fixloc
                        //bsh <- x.fixloc

                        currentBlock->CFGForwardEdges.push_back(instructionBlockPair.at(follow.getFixLoc())); //Connect forward edge to the point to the unconditional branch
                        UnCJF(follow); //unconditional branch follow.fixloc


                        //After inner block, ssa numbering should be revert to the previous state
                        ssaBuilder.revertToOuter(dominatingBlockNum);
                        cseTracker.revertToOuter(cseRevertBlock,false);
                        if(finalizeAndStartNewBlock(blk_while_end, false, false,false))//After unconditional jump means going back without reservation
                            updateBlockForDT(dominatingBlockNum);
                        currentBlock->setOuterBlockKind(outerBlockKind);

                        vector<shared_ptr<IRFormat>> phiCodes = ssaBuilder.getPhiCodes();
                        //vector<IRFormat> irCodes = phiCodes;
                        //irCodes.insert(irCodes.end(),std::make_move_iterator(joinBlockCodes.begin()),std::make_move_iterator(joinBlockCodes.end()));
                        updatePhiInBB(conditionBlockNum, phiCodes);
                        ssaBuilder.currentBlockKind.pop();


                        Fixup((unsigned long)x.getFixLoc()); //fix so that while branch here
                        ssaBuilder.endJoinBlock();

                        for(auto code : phiCodes)
                        {
                            //Phi is also kind of definition(defined kind: inst)

                            string targetOperandName = code->operands.at(0).getVariableName();
                            shared_ptr<Symbol> targetOperandSym = code->operands.at(0).getVarSym();
                            Result definedOperand;
                            definedOperand.setInst(code);

                            DefinedInfo defInfo(currentBlock->getBlockNum(), targetOperandName);
                            defInfo.setInst(code->getLineNo(),code);

                            ssaBuilder.prepareForProcess(targetOperandName, targetOperandSym, defInfo);
                            for(auto &operand : code->operands)
                            {
                                if(operand.getKind() == errKind)
                                {
                                    DefinedInfo defJustBefore = ssaBuilder.getDefinedInfo();
                                    Kind kind = defJustBefore.getKind();
                                    if(kind == instKind)
                                        operand.setInst(defJustBefore.getInst());
                                    else if(kind == varKind) {
                                        operand.setVariable(defJustBefore.getVar(), defJustBefore.getVarSym());
                                        operand.setDefInst(defJustBefore.getDefinedInstOfVar());
                                    }
                                    else if(kind == constKind) {
                                        operand.setConst(defJustBefore.getConst());
                                        operand.setConstPropVar(targetOperandName);
                                    }
                                }
                            }
                            ssaBuilder.insertDefinedInstr();
                            //If there is outer join block propagate
                            emitOrUpdatePhi(targetOperandName, definedOperand);
                        }
                        loopDepth--;
                        //When completely out of while block, do the cse for load
                        //if(ssaBuilder.currentBlockKind.empty() || ssaBuilder.currentBlockKind.top() != blk_while_body) {
                        cseForWhileInst(dominatingBlockNum);
                            //cseForLoad(dominatingBlockNum); // Do for for innner block

                        //}
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
            int dominatingBlockNum = currentBlock->getBlockNum();

            //Join Block create
            ssaBuilder.startJoinBlock(currentBlock->getBlockKind(),currentBlock->getBlockNum());
            BlockKind outerBlockKind = currentBlock->getOuterBlockKind();
            //In if statement, after the condition new block starts
            if(!finalizeAndStartNewBlock(blk_if_then, true, true,true)) //Automatically dominated by previous block
                currentBlock->setBlockKind(blk_if_then);
            ssaBuilder.currentBlockKind.push(blk_if_then);
            ssaBuilder.protectDef();
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
                        //Until this point, still in the then block

                        //After inner block, ssa numbering should be revert to the previous state
                        ssaBuilder.revertToOuter(dominatingBlockNum);
                        cseTracker.revertToOuter(dominatingBlockNum,false);
                        //The start of else is new block
                        finalizeAndStartNewBlock(blk_if_else, false, false,false);//if then is performed it should avoid else
                        ssaBuilder.protectDef();
                        ssaBuilder.currentBlockKind.pop();
                        ssaBuilder.currentBlockKind.push(blk_if_else);
                        updateBlockForDT(dominatingBlockNum); //else should be dominated by condition block
                        Fixup((unsigned long)x.getFixLoc());
                        if(isStatSequence(scannerSym))
                        {
                            statSequence();
                        }
                        else
                            Error("ifStatement",{"statSequence"});
                        //After inner block, ssa numbering should be revert to the previous state
                        ssaBuilder.revertToOuter(dominatingBlockNum);
                        cseTracker.revertToOuter(dominatingBlockNum,true);
                        if(finalizeAndStartNewBlock(blk_if_end, false, true,false)) //After all if related statements end, new block start
                            updateBlockForDT(dominatingBlockNum);//if.end block should be dominated by condition
                    }
                    else {
                        //After inner block, ssa numbering should be revert to the previous state
                        ssaBuilder.revertToOuter(dominatingBlockNum);
                        cseTracker.revertToOuter(dominatingBlockNum,true);
                        if(finalizeAndStartNewBlock(blk_if_end, false, true,false))//After all if related statements end, new block start
                            updateBlockForDT(dominatingBlockNum);//if.end block should be dominated by condition
                        Fixup((unsigned long) x.getFixLoc());
                    }
                    currentBlock->setOuterBlockKind(outerBlockKind);
                    ssaBuilder.currentBlockKind.pop();
                    //updateBlockForDT(dominatingBlockNum);//if.end block should be dominated by condition

                    if(scannerSym == fiToken)
                    {

                        vector<shared_ptr<IRFormat>> phiCodes = ssaBuilder.getPhiCodes();
                        ssaBuilder.endJoinBlock(); //go back to outer joinBlock


                        for(auto code : phiCodes)
                        {

                            code->setBlkNo(currentBlock->getBlockNum());//Fix join block num which was not correct at first.
                            currentBlock->phiCodes.push_back(code);//contents of join block is copied

                            //Phi is also kind of definition(defined kind: inst)
                            string targetOperand = code->operands.at(0).getVariableName();
                            shared_ptr<Symbol> targetOperandSym = code->operands.at(0).getVarSym();

                            Result definedOperand;
                            definedOperand.setInst(code);

                            DefinedInfo defInfo(currentBlock->getBlockNum(),targetOperand);
                            defInfo.setInst(code->getLineNo(),code);

                            ssaBuilder.prepareForProcess(targetOperand, targetOperandSym,defInfo);
                            for(auto &operand : code->operands)
                            {
                                if(operand.getKind() == errKind)
                                {
                                    DefinedInfo defJustBefore = ssaBuilder.getDefinedInfo();
                                    Kind kind = defJustBefore.getKind();
                                    if(kind == instKind)
                                        operand.setInst(defJustBefore.getInst());
                                    else if(kind == varKind) {
                                        operand.setVariable(defJustBefore.getVar(), defJustBefore.getVarSym());
                                        operand.setDefInst(defJustBefore.getDefinedInstOfVar());
                                    }
                                    else if(kind == constKind)
                                        operand.setConst(defJustBefore.getConst());
                                }
                            }


                            ssaBuilder.insertDefinedInstr();
                            //If there is outer join block propagate
                            emitOrUpdatePhi(targetOperand,definedOperand);
                        }

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

Result Parser::funcCall() {
    Scanner *scanner = Scanner::instance();
    std::string functionName;
    Result x, result;
    int numOfParam = 0;
    int locationOfFunc = 0;
    //std::vector<Result> arguments;

    if(scannerSym == callToken)
    {
        Next();
        if(scannerSym == identToken)
        {
            functionName = scanner->id;
            shared_ptr<Symbol> functionSym = symTableLookup(scopeStack.top(),functionName, sym_func);
            numOfParam = functionSym->getNumOfParam(); //number of function parameter
            locationOfFunc = functionSym->getBaseAddr();  //function location(instruction number)
            Next();
            if(scannerSym == openparenToken)
            {
                Next();
                if(isExpression((scannerSym)))
                {
                    int i = 0;
                    x = expression();

                    //Because of predefined function
                    if(functionName != "OutputNum")
                    {
                        Result reg_param;reg_param.setReg(REG_PARAM);
                        emitIntermediate(IR_miu,{x,reg_param}); //We assume that SP is automatically adjusted (So SP is adjusted and then store them)
                        i++;
                        while(scannerSym == commaToken)
                        {
                            Next();
                            if(isExpression(scannerSym))
                            {
                                x = expression();
                                if(i < NUM_OF_PARAM_REGS)
                                {
                                    reg_param.setReg(REG_PARAM + i);
                                    emitIntermediate(IR_miu,{x,reg_param});
                                }
                                else{
                                    reg_param.setReg(REG_SP);
                                    emitIntermediate(IR_miu,{x,reg_param});
                                }
                                i++;
                            }
                            else
                                Error("funcCall",{"expression"});
                        }
                        if(i != numOfParam)
                            cerr << "Number of parameter not matched" << endl;
                    }
                }

                if(scannerSym == closeparenToken)
                {

                    Next();
                }
                else
                    Error("funcCall",{getTokenStr(closeparenToken)});
            }

            //Deal with predefined function
            if(functionName == "InputNum")
                result = emitIntermediate(IR_read,{});
            else if (functionName =="OutputNum")
                result = emitIntermediate(IR_write,{x});
            else if (functionName =="OutputNewLine")
                result = emitIntermediate(IR_writeNL,{});
                //General Fucntion
            else
            {

                //For all global variable
                SymTable mainSymTable = symTableList.at(GLOBAL_SCOPE_NAME);
                for(auto symbol : mainSymTable.varSymbolList)
                {
                    string symName = symbol.first;
                    shared_ptr<Symbol> sym = symbol.second;

                    SymTable currentSymTable = symTableList.at(functionName);
                    auto currentVarSymIter = currentSymTable.varSymbolList.find(symName);
                    if(currentSymTable.varSymbolList.end() != currentVarSymIter)//If local variable has the same name with a global variable -> skip it
                        continue;

                    DefinedInfo defInfo = ssaBuilder.getDefinedInfo(symName);
                    Result storedValue;
                    Kind defKind = defInfo.getKind();
                    //store globals having been defined
                    if(defKind != errKind && defKind != reloadKind)//without No definition or definition nullified case
                    {
                        if(defKind == constKind)
                        {
                            storedValue.setConst(defInfo.getConst());
                            storedValue.setConstPropVar(symName);
                        }
                        else if(defKind == varKind) {
                            storedValue.setVariable(defInfo.getVar(), defInfo.getVarSym());
                            storedValue.setDefInst(defInfo.getDefinedInstOfVar());
                        }
                        else if(defKind == instKind)
                            storedValue.setInst(defInfo.getInst());

                        int loc = sym->getBaseAddr()*4;
                        Result operandGP;operandGP.setReg(REG_GP);
                        Result operandLoc;operandLoc.setConst(loc);
                        Result addrToStore = emitIntermediate(IR_adda,{operandGP,operandLoc});
                        emitIntermediate(IR_store,{storedValue,addrToStore});
                    }
                }

                Result jumpLocation;jumpLocation.setConst(locationOfFunc);
                jumpLocation.setDiffFuncLoc(functionName, functionSym);
                SymType symType = functionSym->getSymType();
                jumpLocation.setFunctionType(symType);
                /*
                //For all global variable
                SymTable mainSymTable = symTableList.at(GLOBAL_SCOPE_NAME);
                for(auto symbol : mainSymTable.varSymbolList)
                {
                    DefinedInfo defInfo = ssaBuilder.getDefinedInfo(symbol.first);
                    if(defInfo.getKind() != errKind)
                    {
                        GlobalDefInfo gDefInfo = {symbol.second, defInfo};
                        jumpLocation.globalDefInfo.insert({symbol.first,gDefInfo});
                    }
                }*/
                result = emitIntermediate(IR_bra,{jumpLocation});
                Result returnValReg; returnValReg.setReg(REG_RET_VAL);
                if(symType == sym_func)
                    emitIntermediate(IR_miu,{returnValReg,result}); //Return value get

                //After branch update all global variable and make definition of it is load instruction
                for(auto symbol : mainSymTable.varSymbolList)
                {
                    string symName = symbol.first;
                    shared_ptr<Symbol> sym = symbol.second;

                    SymTable currentSymTable = symTableList.at(functionName);
                    auto currentVarSymIter = currentSymTable.varSymbolList.find(symName);
                    if(currentSymTable.varSymbolList.end() != currentVarSymIter)//If local variable has the same name with a global variable -> skip it
                        continue;

                    //After function call, all array load should be killed
                    if(sym->getSymType() == sym_array)
                    {
                        shared_ptr<IRFormat> irCode(new IRFormat);
                        irCode->setBlkNo(currentBlock->getBlockNum());
                        irCode->setLineNo(-1);
                        irCode->setIROP(IR_store);
                        Result definedVal;definedVal.setVariable(symName,sym);
                        irCode->operands = {Result(),definedVal};
                        shared_ptr<IRFormat> previousSameOpInst = cseTracker.getCurrentInstPtr(IR_store);
                        irCode->setPreviousSameOpInst(previousSameOpInst);
                        cseTracker.setCurrentInst(IR_store,irCode);
                    }
                    else//All global variable should be reinitialized
                    {
                        DefinedInfo defInfo;defInfo.setReload();
                        //int loc = sym->getBaseAddr()*4;
                        //Result operandGP;operandGP.setReg(REG_GP);
                        //Result operandLoc;operandLoc.setConst(loc);
                        //Result addrToLoad = emitIntermediate(IR_add,{operandGP,operandLoc});
                        //Result loadedVal = emitIntermediate(IR_load,{addrToLoad});
                        //defInfo.setInst(loadedVal.getInst()->getLineNo(),loadedVal.getInst());
                        ssaBuilder.prepareForProcess(symName,sym,defInfo);
                        ssaBuilder.insertDefinedInstr();
                    }

                }
            }
        }
        else
            Error("funcCall",{getTokenStr(identToken)});
    }
    else
        Error("funcCall",{getTokenStr(callToken)});

    return result;
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
                    if(x.isArrayInst()) {
                        Result indexResult;indexResult.setInst(x.getIndexInst());
                        string arrayString = x.getVariableName();
                        x = emitIntermediate(IR_adda,{x,indexResult});
                        x.setArrayInst(arrayString);
                        result = emitIntermediate(IR_store, {y, x});
                    }
                    else {

                        result = emitIntermediate(IR_move, {y,x});
                        //No move instruction more, but return y:which is x(variable)'s phi updated value
                        emitOrUpdatePhi(x.getVariableName(), result);//For variable x
                    }
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
                if(irOp == IR_sub)//if(y.getKind() == instKind && y.getInst()->getLineNo() == 15)
                {
                    int temp = 1;
                }
                //Find the same expression in the list

                result = emitIntermediate(irOp, {result, y});
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
                result = emitIntermediate(irOp,{result,y});
            }
            else
                Error("term",{"factor"});
        }
    }
    else
        Error("term",{"factor"});

    return result;
}

Result Parser::factor()
{
    Scanner *scanner = Scanner::instance();
    Result result, x;
    if(isDesignator(scannerSym))
    {
        result = designator();
        if(result.isArrayInst()) {
            Result indexResult;indexResult.setInst(result.getIndexInst());
            string arrayString = result.getVariableName();
            result = emitIntermediate(IR_adda,{result,indexResult});
            result.setArrayInst(arrayString);
            result = emitIntermediate(IR_load, {result});//load value of the index
        }
    }
    else if (scannerSym == numberToken) {
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
        result = funcCall();
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
        shared_ptr<Symbol> symInfo = symTableLookup(scopeStack.top(),scanner->id, sym_var);
        string variableName = scanner->id;
        x.setVariable(variableName,symInfo);

        Next();

        int i = 0;
        while(scannerSym == openbracketToken)
        {
            Next();
            if(isExpression(scannerSym))
            {
                if(i > 0) {
                    Result capacityOfArray;
                    capacityOfArray.setConst(symInfo->arrayCapacity.at((unsigned long)i)); //current capacity
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
        if(symInfo->getSymType() == sym_array)
        {
            //x is array. So we have to provide address of x and index
            shared_ptr<Symbol> xSymbol = symTableLookup(scopeStack.top(), x.getVariableName(), sym_array);
            x.setConst(xSymbol->getBaseAddr()*4);
            Result indexAdjust;indexAdjust.setConst(4);
            updatedIndex = emitIntermediate(IR_mul,{updatedIndex,indexAdjust}); // index * 4(word size)
            SymTable symTable = symTableList.at(scopeStack.top());
            auto symIter = symTable.varSymbolList.find(variableName);
            //if array not exist in local var: global array
            if(symIter == symTable.varSymbolList.end())
                y.setReg(REG_GP);
            else
                y.setReg(REG_FP);//Frame pointer

            result = emitIntermediate(IR_add,{y,x});//y: base address(FP + x's base)
            result.setIndexInst(updatedIndex.getInst());
            //result = emitIntermediate(IR_adda,{y,updatedIndex});//base address + index
            result.setArrayInst(variableName);
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


Result Parser::emitIntermediate(IROP irOp,vector<Result> x)
{
    //if one of operands are array, we should handle it
    int index = 0;

    if(irOp == IR_move) //Copy Propagation : every move should be replaced by first operand
    {
        string defVar = x.at(1).getVariableName();
        shared_ptr<Symbol> defVarSym= x.at(1).getVarSym();
        DefinedInfo defInfo(currentBlock->getBlockNum(),defVar);

        Kind definedKind = x.at(0).getKind();
        Result propagatedResult = x.at(0);

        if(definedKind == instKind)
            defInfo.setInst(x.at(0).getInst()->getLineNo(),x.at(0).getInst());
        else if(definedKind == varKind) {

            DefinedInfo defInfoOfDef;
            ssaBuilder.prepareForProcess(propagatedResult.getVariableName(),propagatedResult.getVarSym(), defInfoOfDef);
            defInfoOfDef = ssaBuilder.getDefinedInfo();
            //propagated only from other variable(trace until initialization)
            Kind kind =defInfoOfDef.getKind();
            if(kind == constKind)
                propagatedResult.setConst(defInfoOfDef.getConst());
            else if(kind == instKind)
                propagatedResult.setInst(defInfoOfDef.getInst());
            else if(kind == varKind) {
                string varName = defInfoOfDef.getVar();
                shared_ptr<Symbol> varSym = defInfoOfDef.getVarSym();
                propagatedResult.setVariable(varName, varSym);
                propagatedResult.setDefInst(defInfoOfDef.getDefinedInstOfVar());
            }

            defInfo = defInfoOfDef;//setVar(x.at(0).getVariableName(), defInfoOfDef.getDefinedInstOfVar());
        }
        else if(definedKind == constKind)
            defInfo.setConst(x.at(0).getConst());

        ssaBuilder.prepareForProcess(defVar, defVarSym,defInfo);
        ssaBuilder.insertDefinedInstr();

        return propagatedResult;
    }

    index = 0;
    vector<Result> operands;

    //Setting operands
    for(auto x_i : x) {

        if(x_i.getKind() == varKind)
        {
            string var = x_i.getVariableName();
            shared_ptr<Symbol> varSym = x_i.getVarSym();

            DefinedInfo definedInfo;
            ssaBuilder.prepareForProcess(var,varSym, definedInfo);
            definedInfo = ssaBuilder.getDefinedInfo();
            if(definedInfo.getKind() == instKind)
                x_i.setInst(definedInfo.getInst());
            else if(definedInfo.getKind() == constKind) {
                x_i.setConst(definedInfo.getConst());
                x_i.setConstPropVar(var);
            }
            else if(definedInfo.getKind() == varKind)
            {
                string varName = definedInfo.getVar();
                shared_ptr<Symbol> varSym = definedInfo.getVarSym();
                x_i.setVariable(varName,varSym);
                x_i.setDefInst(definedInfo.getDefinedInstOfVar());//previously defined instr
            }
            else if(definedInfo.getKind() == reloadKind) //for global variable reload
            {
                shared_ptr<Symbol> sym = symTableLookup(GLOBAL_SCOPE_NAME,var,sym_var);
                int loc = sym->getBaseAddr()*4;
                Result operandGP;operandGP.setReg(REG_GP);
                Result operandLoc;operandLoc.setConst(loc);
                Result addrToLoad = emitIntermediate(IR_adda,{operandGP,operandLoc});//addrToLoad.setArrayInst(var);
                Result loadedVal = emitIntermediate(IR_load,{addrToLoad});
                DefinedInfo defInfo;defInfo.setInst(loadedVal.getInst()->getLineNo(),loadedVal.getInst());
                ssaBuilder.prepareForProcess(var,sym,defInfo);
                ssaBuilder.insertDefinedInstr();
                x_i.setInst(defInfo.getInst());
            }
        }

        //Cost setting
        Kind operandKind = x_i.getKind();
        if(operandKind == instKind)
        {
            shared_ptr<IRFormat> operandInst = x_i.getInst();
            int cost = operandInst->getCost();
            cost = cost + pow(10,loopDepth);
            operandInst->setCost(cost);
        }
        else if(operandKind == varKind)
        {
            shared_ptr<Symbol> sym = symTableLookup(scopeStack.top(), x_i.getVariableName(), sym_var);
            int cost = sym->getCost();
            cost = cost + pow(10,loopDepth);
            sym->setCost(cost);
        }

        operands.push_back(x_i);
        index++;
    }
    //Normal IR generation

    shared_ptr<IRFormat> ir_line(new IRFormat);
    ir_line->setBlkNo(currentBlock->getBlockNum());
    ir_line->setLineNo(IRpc);
    ir_line->setIROP(irOp);
    ir_line->operands = operands;

    Result result;
    result.setInst(ir_line);
    instructionBlockPair.insert({IRpc, currentBlock});//it is emitted now in current block

    //Check whether elimination is possible(general case)
    if(irOp == IR_add || irOp == IR_mul || irOp == IR_div || irOp == IR_sub ||  irOp == IR_neg ||
            irOp == IR_cmp|| irOp == IR_load){

            if(!ssaBuilder.currentBlockKind.empty() && ssaBuilder.currentBlockKind.top() == blk_while_body)
            {

                cseTracker.candidateCSEInstructions.push_back(ir_line);
            }
            else
            {
                shared_ptr<IRFormat> existingCommonSub = cseTracker.findExistingCommonSub(ir_line);
                if (existingCommonSub != NULL) {
                    if(irOp == IR_load) //Remove also adda
                    {
                        //IRCodes.pop_back();//I don't know why?
                        currentBlock->irCodes.pop_back();
                    }
                    Result returnedVal;
                    returnedVal.setInst(existingCommonSub);
                    return returnedVal;
                }
            }
    }



    //For common subexpression tracking
    shared_ptr<IRFormat> previousSameOpInst = cseTracker.getCurrentInstPtr(irOp);

    ir_line->setPreviousSameOpInst(previousSameOpInst);

    cseTracker.setCurrentInst(irOp,ir_line);

    IRCodes.push_back(ir_line);

    //IR in BasicBlock generation
    //Operands in branch should point to a block
    if(isBranchCond(irOp))
    {
        Result *operandToChange;
        unsigned long operandIndex = 0;
        if(ir_line->operands.size() > 1)
            operandIndex = 1;
        operandToChange = &ir_line->operands.at(operandIndex);

        //direct jump(do not allow jump to location stored in address)
        if(operandToChange->getKind() == constKind)
        {
            int blockNum;
            if(operandToChange->isDiffFuncLoc())
            {
                string funcName = operandToChange->getDiffFuncName();
                vector<shared_ptr<BasicBlock>> diffFuncBlockList = functionList.at(funcName);
                blockNum = diffFuncBlockList.at(0)->getBlockNum();
            }
            else
            {
                shared_ptr<BasicBlock> tempBlock = instructionBlockPair.at(operandToChange->getConst());
                blockNum = tempBlock->getBlockNum();
            }
            operandToChange->setBlock(blockNum);
            //ir_line.operands.at(operandIndex) = operandToChange;
        }
    }

    currentBlock->irCodes.push_back(ir_line);

    IRpc++;
    return result;
}

string Parser :: getCodeString(string functionName,shared_ptr<IRFormat> code)
{
    string lineString;
    int regForCode = code->getRegNo();
    if(regForCode == -1)
        lineString = to_string(code->getLineNo());
    else
        lineString = "%" + to_string(regForCode);

    string tab = "  ";
    string result = lineString + tab + getIROperatorString(code->getIROP());
    int index = 0;
    for(auto operand : code->operands)
    {
        if(operand.getKind() == errKind)
            break;

        if(operand.getKind() == constKind)
        {
            result = result + tab + to_string(operand.getConst());// << "\t";
        }
        else if (operand.getKind() == varKind)
        {
            shared_ptr<Symbol> varSym = operand.getVarSym();
            int regNo = varSym->getRegNo(functionName);
            if(regNo == -1)
                result = result + tab + operand.getVariableName() + "_" + to_string(operand.getDefInst());// << "\t";
            else
                result = result + tab + "%" + to_string(regNo);
        }
        else if(operand.getKind() == instKind) //Result of that instruction
        {
            shared_ptr<IRFormat> inst = operand.getInst();
            int regNo = inst->getRegNo();
            //still not allocated a register
            if(regNo == -1)
                result = result + tab + "(" + to_string(inst->getLineNo()) + ")";// << "\t";
                //After register allocation
            else
                result = result + tab + "%" + to_string(regNo);
        }
        else if(operand.getKind() == blockKind) //Result of that instruction
        {
            result = result + tab + "[" + to_string(operand.getBlockNo()) + "]";// << "\t";
        }
        else if(operand.getKind() == regKind)
        {
            result = result + tab + "%" + to_string(operand.getReg(functionName));// << "\t";
        }
        else {
            std::cerr << std::endl<< "No valid operand x"<< index << " : "<< operand.getKind() << std::endl;
        }
        index++;
    }
    return result;
}


void Parser::createControlFlowGraph(const string &graphFolder,const string &sourceFileName,const string version)
{
    string formatName = ".dot";
    RC rc = -1;
    for(auto function : functionList) {
        string functionName = function.first;
        string fileName = graphFolder + "CFG_"+version+"_" + sourceFileName+ "_" + functionName + formatName;

        GraphDrawer *graphDrawer = GraphDrawer::instance();
        rc = graphDrawer->createFile(fileName);
        if (rc == -1)
            return;
        rc = graphDrawer->openFile(fileName);
        if (rc == -1) {
            graphDrawer->destroyFile(fileName);
            return;
        }

        graphDrawer->writePreliminary(graph_CFG,functionName);
        vector<shared_ptr<BasicBlock>> basicBlockList = function.second;
        for (auto blockPair : basicBlockList) {
            shared_ptr<BasicBlock> block = blockPair;
            graphDrawer->writeNodeStart(block->getBlockNum(), block->getBlockName());
            for (auto code : block->edgeCodes) {
                if(code->isElimiated())
                    continue;
                string codeString = getCodeString(function.first,code);
                graphDrawer->writeCode(codeString);
            }
            for (auto code : block->phiCodes) {
                if(code->isElimiated())
                    continue;
                string codeString = getCodeString(function.first,code);
                graphDrawer->writeCode(codeString);
            }
            for (auto code : block->irCodes) {
                if(code->isElimiated())
                    continue;
                string codeString = getCodeString(function.first,code);
                graphDrawer->writeCode(codeString);
            }
            if (block->isCondBlock()) {
                graphDrawer->writeCodeForCond();
            }
            graphDrawer->writeNodeEnd();

            for (auto dest : block->CFGForwardEdges) {
                EDGETYPE edgeType = edge_normal;
                if (block->isCondBlock()) {
                    if (block->isTrueEdge(dest->getBlockNum())) {
                        edgeType = edge_true;
                    }
                    else {
                        edgeType = edge_false;
                    }
                }
                graphDrawer->writeEdge(block->getBlockNum(), dest->getBlockNum(), edgeType,graph_CFG);
            }
        }
        graphDrawer->writeEnd();
        graphDrawer->closeFile();
    }
}


void Parser::createDominantGraph(const string &graphFolder,const string &sourceFileName)
{
    string formatName = ".dot";
    RC rc = -1;
    for(auto function : functionList) {
        string functionName = function.first;
        string fileName = graphFolder + "DT_"+ sourceFileName+ "_" + functionName + formatName;

        GraphDrawer *graphDrawer = GraphDrawer::instance();
        rc = graphDrawer->createFile(fileName);
        if (rc == -1)
            return;
        rc = graphDrawer->openFile(fileName);
        if (rc == -1) {
            graphDrawer->destroyFile(fileName);
            return;
        }

        graphDrawer->writePreliminary(graph_DT,functionName);
        vector<shared_ptr<BasicBlock>> basicBlockList = function.second;
        for (auto blockPair : basicBlockList) {
            shared_ptr<BasicBlock> block = blockPair;
            graphDrawer->writeNodeStart(block->getBlockNum(), block->getBlockName());
            for (auto code : block->phiCodes) {
                if(code->isElimiated())
                    continue;
                string codeString = getCodeString(function.first,code);
                graphDrawer->writeCode(codeString);
            }
            for (auto code : block->irCodes) {
                if(code->isElimiated())
                    continue;
                string codeString = getCodeString(function.first,code);
                graphDrawer->writeCode(codeString);
            }
            graphDrawer->writeNodeEnd();

            for (auto dest : block->DTForwardEdges) {
                EDGETYPE edgeType = edge_normal;
                graphDrawer->writeEdge(block->getBlockNum(), dest->getBlockNum(), edgeType,graph_DT);
            }
        }
        graphDrawer->writeEnd();
        graphDrawer->closeFile();
    }
}




void Parser::printBlock()
{
    for(auto function : functionList)
    {
        vector<shared_ptr<BasicBlock>> basicBlockList = function.second;
        for(auto blockPair : basicBlockList)
        {
            shared_ptr<BasicBlock> block = blockPair;
            cout<<"Block " << block->getBlockNum() << " -------------------------------------" << endl;
            printIRCodes(function.first,block->phiCodes);
            printIRCodes(function.first,block->irCodes);
            cout<<"Forward Edge to";
            for(auto edge : block->CFGForwardEdges)
                cout << " " << edge->getBlockNum();
            cout << endl;
            cout<< "--------------------------------------------" << endl;
        }
    }
/*
    for(auto dominated : dominatedByInfo)
    {
        cout << dominated.first << " : ";
        stack<int> dominatings = dominated.second;
        while(!dominatings.empty())
        {
            cout << dominatings.top() << " ";
            dominatings.pop();
        }
        cout << endl;
    }
*/
}


void Parser::printIRCodes(string functionName,vector<shared_ptr<IRFormat>> codes)
{

    for(auto code : codes) {
        std::string op = getIROperatorString(code->getIROP());
        std::cout << code->getLineNo() << "\t" << op << "\t";

        int index = 0;
        for(auto operand : code->operands)
        {
            if(operand.getKind() == errKind)
                break;

            if(operand.getKind() == constKind)
            {
                std::cout << operand.getConst();// << "\t";
            }
            else if (operand.getKind() == varKind)
            {
                std::cout << operand.getVariableName() << "_" << operand.getDefInst();// << "\t";
            }
            else if(operand.getKind() == instKind) //Result of that instruction
            {
                std::cout << "(" << operand.getInst()->getLineNo() << ")";// << "\t";
            }
            else if(operand.getKind() == blockKind) //Result of that instruction
            {
                std::cout << "[" << operand.getBlockNo() << "]";// << "\t";
            }
            else if(operand.getKind() == regKind)
            {
                std::cout << "%" << operand.getReg(functionName);// << "\t";
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
    for(auto symTableIter : symTableList)
    {
        cout << symTableIter.first << ":" << endl;
        SymTable symtable = symTableIter.second;
        for(auto symIter : symtable.varSymbolList)
        {
            shared_ptr<Symbol> sym = symIter.second;
            SymType symType = sym->getSymType();

            if(symType == sym_var)
            {
                cout << "\t" << "var";
                cout << " " << symIter.first;
                cout << "\t" << "Base Address: " << sym->getBaseAddr();
            }
            else if(symType == sym_param)
            {
                cout << "\t" << "param";
                cout << " " << symIter.first;
                cout << "\t" << "Base Address: " << sym->getBaseAddr();
            }
            else if(symType == sym_array)
            {
                cout << "\t" << "array ";
                for(auto cap : sym->arrayCapacity)
                {
                    cout << "[" << cap << "]";
                }
                cout << " " << symIter.first;
                cout << "\t" << "Base Address: " << sym->getBaseAddr();
            }

            else if(symType == sym_func)
            {
                cout << "\t" << "function";
                cout << "\t" << symIter.first;
                cout << "\t" << "Base Address: " << sym->getBaseAddr();
                cout << "\t" << "Number of Parameters: " << sym->getNumOfParam();
            }
            else if(symType == sym_proc)
            {
                cout << "\t" << "procedure";
                cout << "\t" << symIter.first;
                cout << "\t" << "Base Address: " << sym->getBaseAddr();
                cout << "\t" << "Number of Parameters: " << sym->getNumOfParam();
            }

            cout << endl;
        }

    }
    //std::cout << "Symbol Name\t" << "Type\t"  << "Location" << std::endl;

}


void Parser :: addFuncSymbol(SymType symType, std::string symbolName, unsigned long numOfParam)
{
    std::string currentScope = scopeStack.top();

    auto parentSymTableIter = symTableList.find(currentScope);
    if(parentSymTableIter == symTableList.end())
    {
        std::cerr << "Current scope is invalid" << std::endl;
        return;
    }
    SymTable *parentSymTable = &parentSymTableIter->second;
    auto symListIter = parentSymTable->funcSymbolList.find(symbolName);
    if(symListIter != parentSymTable->funcSymbolList.end())
    {
        std::cerr << "Same symbol already exist in symbol table" << std::endl;
        return;
    }

    auto symTblIter = symTableList.find(symbolName);
    if(symTblIter != symTableList.end())
    {
        std::cerr << "Same symbol already exist in symbol table list" << std::endl;
        return;
    }


    //Make symbol
    shared_ptr<Symbol> functionSymbol(new Symbol(symType,IRpc,(int)numOfParam));
    //insert this symbol to symbol table.
    parentSymTable->funcSymbolList.insert({symbolName, functionSymbol});

    //Make its own symbol table
    SymTable newSymTable(currentScope);
    //Insert new symbol table to symbol table list
    symTableList.insert({symbolName,newSymTable});

}

void Parser::addVarSymbol(std::string symbol, SymType symType, std::vector<int> arrayCapacity)
{
    int varSize = 1; // General variable size;
    for (auto cap : arrayCapacity)
    {
        varSize = varSize * cap;
    }//Array consumes lots of the capacity

    std::string currentScope = scopeStack.top();
    auto symTableIter = symTableList.find(currentScope);
    if(symTableIter == symTableList.end())
    {
        std::cerr << "Current scope is not valid" << std::endl;
        return;
    }
    SymTable *currentSymTable = &symTableIter->second;
    int localVarSize = currentSymTable->getLocalVarSize() + varSize;
    //Make symbol
    shared_ptr<Symbol> newSymbol(new Symbol(symType, LOCAL_IN_STACK - localVarSize, arrayCapacity));
    newSymbol->setVarSize(varSize);

    //Update symtable
    currentSymTable->insertVarSym(symbol,newSymbol);

    currentSymTable->setLocalVarSize(localVarSize);

}

void Parser::addParamSymbol(std::string symbol, size_t numOfParam, int index)
{
    std::string currentScope = scopeStack.top();
    auto symTableIter = symTableList.find(currentScope);
    if(symTableIter == symTableList.end())
    {
        std::cerr << "Current scope is not valid" << std::endl;
        return;
    }
    SymTable *currentSymTable = &symTableIter->second;
    //Make symbol
    shared_ptr<Symbol> newSymbol(new Symbol(sym_param, PARAM_IN_STACK + ((int)numOfParam - 1) - index));
    newSymbol->setParamIndex(index);
    //Update symtable
    currentSymTable->insertVarSym(symbol,newSymbol);
}

void Parser:: symbolTableUpdate(string var,shared_ptr<Symbol> varSym)
{
    string currentScope = scopeStack.top();
    while(currentScope != "")
    {
        SymTable *currentSymTable = &symTableList.at(currentScope);
        auto symIter = currentSymTable->varSymbolList.find(var);
        if(symIter != currentSymTable->varSymbolList.end()) {
            symIter->second = varSym;
            return;
        }
        currentScope = currentSymTable->getParent();
    }

    return; //There is no symbol
}



shared_ptr<Symbol> Parser:: symTableLookup(string scope, std::string symbol,SymType symType)
{
    //string scope = scopeStack.top();

    while(scope != "")
    {
        auto currentSymTableIter = symTableList.find(scope);
        if(currentSymTableIter == symTableList.end())
        {
            std::cerr << "Current scope is not valid" << std::endl;
            return NULL;
        }


        SymTable currentSymTable = currentSymTableIter->second;
        unordered_map<string,shared_ptr<Symbol>> symbolList;

        //Distinguish varsym and funcsym
        if(symType == sym_var || symType == sym_param || symType == sym_array)
            symbolList = currentSymTable.varSymbolList;
        else if (symType == sym_func || symType == sym_proc)
            symbolList = currentSymTable.funcSymbolList;

        auto symIter = symbolList.find(symbol);
        if(symIter != symbolList.end())
            return symIter->second;

        scope = currentSymTable.getParent();
    }


    std::cerr << "There is no symbol like " << symbol << std::endl;

    return NULL; //There is no symbol
}


void Parser :: CondJF(Result &x)
{
    Result y; //temporary blank y: later fixed up
    y.setConst(0);
    emitIntermediate(negateCondition(x.getRelOp()),{x,y});
    x.setFixLoc(IRpc -1);
}

void Parser :: UnCJF(Result &x)
{
    int destinationInst = x.getFixLoc();
    //int destinationBlock = instructionBlockPair.at(destinationInst);

    //Make forward edge to the block which include instr destinationInst
    //currentBlockNum.CFGForwardEdges.push_back(destinationBlock);

    Result temp;temp.setConst(destinationInst);
    //Result temp;temp.setBlock(destinationBlock);
    emitIntermediate(IR_bra, {temp});

    x.setFixLoc(IRpc - 1);
}

void Parser :: Fixup(unsigned long loc)
{
    shared_ptr<IRFormat> operationToChange = IRCodes.at(loc);
    unsigned long operandToFix = 0; //IR_bra
    if(operationToChange->getIROP() != IR_bra)
        operandToFix = 1;

    operationToChange->operands.at(operandToFix).setConst(IRpc);


    //Block with fixed instruction also has forward edge to current PC
    shared_ptr<BasicBlock> tempBlock = instructionBlockPair.at(loc);
    int targetBlockNum = tempBlock->getBlockNum();

    string currentScope = scopeStack.top();
    vector<shared_ptr<BasicBlock>> basicBlockList = functionList.at(currentScope);
    shared_ptr<BasicBlock> targetBlock = basicBlockList.at(targetBlockNum - ssaBuilder.getStartBlock());

    targetBlock->CFGForwardEdges.push_back(currentBlock);
    //targetBlock.DTForwardEdges.push_back(currentBlockNum.getBlockNum());
    operationToChange->operands.at(operandToFix).setBlock(currentBlock->getBlockNum());

}

void Parser :: FixLink(unsigned long loc)
{
    unsigned long next;
    while(loc != 0)
    {
        shared_ptr<IRFormat> operationToChange = IRCodes.at(loc);
        unsigned long operandToFix = 0; //IR_bra
        if(operationToChange->getIROP() != IR_bra)
            operandToFix = 1;
        next = (unsigned long)operationToChange->operands.at(operandToFix).getConst();
        Fixup(loc);
        loc = next;
    }
}

//Only for getting return address(should not be used)(return will be dealt with by code generation)
Result Parser:: getAddressInStack(int location)
{
    Result offset;offset.setConst(location);
    Result adjust;adjust.setConst(4);
    Result FP;FP.setReg(REG_FP);
    offset = emitIntermediate(IR_mul,{offset,adjust});
    offset = emitIntermediate(IR_adda,{FP,offset});
    //offset.setArrayInst();
    return offset;
}

void Parser::predefinedFunc()
{
    //predefined function
    SymType symType = sym_func;
    //InputNum
    addFuncSymbol(symType,"InputNum",0);
    //scopeStack.push("InputNum"); //Current Scope set
    //Result x = emitIntermediate(IR_read,{});
    //Result ret;ret.setVariable(REG_RET);
    //emitIntermediate(IR_move,{x,ret});

    //Result offset = getAddressInStack(RETURN_IN_STACK);
    //x = emitIntermediate(IR_load,{offset});
    //emitIntermediate(IR_bra,{x});
    //scopeStack.pop(); //Current Scope set

    //predefined procedure
    symType = sym_proc;

    //OutputNum
    addFuncSymbol(symType,"OutputNum",1);
    //scopeStack.push("OutputNum"); //Current Scope set
    //string param = "x";
    //Result paramVal;paramVal.setVariable(param);
    //addParamSymbol(param,1,0);
    //emitIntermediate(IR_write,{paramVal});
    //offset = getAddressInStack(RETURN_IN_STACK);
    //x = emitIntermediate(IR_load,{offset});
    //emitIntermediate(IR_bra,{x});

    //scopeStack.pop();

    //OutputNewLine
    addFuncSymbol(symType,"OutputNewLine",0);
    //scopeStack.push("OutputNewLine");
    //emitIntermediate(IR_writeNL,{});
    //offset = getAddressInStack(RETURN_IN_STACK);
    //x = emitIntermediate(IR_load,{offset});
    //emitIntermediate(IR_bra,{x});
    //scopeStack.pop();
}


void Parser::updateBlockForDT(int dominatingBlockNum)//dominatingBlock dominate current block
{
    string currentScope = scopeStack.top();
    vector<shared_ptr<BasicBlock>> basicBlockList = functionList.at(currentScope);
    shared_ptr<BasicBlock> targetBlock = basicBlockList.at(dominatingBlockNum - ssaBuilder.getStartBlock());

    targetBlock->DTForwardEdges.push_back(currentBlock);//after end of while -> dominated by dominating block(condition)


    stack<int> dominatedBy = dominatedByInfo.at(dominatingBlockNum);
    dominatedBy.push(currentBlock->getBlockNum());
    dominatedByInfo.insert({currentBlock->getBlockNum(),dominatedBy});
}


shared_ptr<BasicBlock> Parser::getBlockFromNum(int blockNum) {
    string currentScope = scopeStack.top();
    vector<shared_ptr<BasicBlock>> basicBlockList = functionList.at(currentScope);
    return basicBlockList.at(blockNum - ssaBuilder.getStartBlock());
}

void Parser::insertBasicBlock(shared_ptr<BasicBlock> block)
{
    string currentScope = scopeStack.top();
    vector<shared_ptr<BasicBlock>> *basicBlockList = &functionList.at(currentScope);
    int blockNum = block->getBlockNum();
    basicBlockList->push_back(currentBlock);
    numOfBlock++;
}

void Parser::updatePhiInBB(int modifiedBlockNum, vector<shared_ptr<IRFormat>> phiCodes)
{
    string currentScope = scopeStack.top();
    vector<shared_ptr<BasicBlock>> basicBlockList = functionList.at(currentScope);
    shared_ptr<BasicBlock> modifiedBlock = basicBlockList.at(modifiedBlockNum - ssaBuilder.getStartBlock());
    modifiedBlock->phiCodes = phiCodes;
}


bool Parser::finalizeAndStartNewBlock(BlockKind newBlockKind, bool isCurrentCond, bool directFlowExist, bool dominate)
{
    //if(!currentBlock.irCodes.empty() || !currentBlock.phiCodes.empty()) {

        //currentBlockNum.setBlockName(currentBlockName);
        //string currentScope = scopeStack.top();
        //unordered_map<int,BasicBlock> basicBlockList = functionList.at(currentScope);
        int currentBlockNum = currentBlock->getBlockNum();
        shared_ptr<BasicBlock> tempBlock = std::make_shared<BasicBlock> (currentBlockNum + 1,newBlockKind);

        //Current Block will be dominated by parent
        if(dominate) {
            currentBlock->DTForwardEdges.push_back(tempBlock);
            stack<int> dominatedBy = dominatedByInfo.at(currentBlockNum);
            dominatedBy.push(currentBlockNum + 1);
            dominatedByInfo.insert({currentBlockNum + 1,dominatedBy});
        }

        //int nextBlockNum = numOfBlock + 1;
        if(directFlowExist) { //Make forward edge to next block
            currentBlock->CFGForwardEdges.push_back(tempBlock);
            if(isCurrentCond)
                currentBlock->setTrueEdge(currentBlockNum + 1);
        }


        string currentScope = scopeStack.top();
        vector<shared_ptr<BasicBlock>> *basicBlockList = &functionList.at(currentScope);
        int blockNum = currentBlock->getBlockNum();
        basicBlockList->push_back(currentBlock);
        numOfBlock++;

        //insertBasicBlock(currentBlock);
        currentBlock = tempBlock;
        return true;
    //}
    //return false;
}


bool Parser:: isDominate(int dominatingBlockNum, int dominatedBlockNum)
{
    stack<int> dominatingBlocks = dominatedByInfo.at(dominatedBlockNum);
    while(!dominatingBlocks.empty())
    {
        if(dominatingBlocks.top() == dominatingBlockNum)
        {
            return true;
        }
        dominatingBlocks.pop();
    }
    return false;
}

void Parser:: emitOrUpdatePhi(string x, Result defined){                    //If x is variable and the current block is condition update phi function

    shared_ptr<Symbol> x_sym = symTableLookup(scopeStack.top(),x,sym_var);
    if (ssaBuilder.currentBlockKind.empty())//Currently not in the then, else block
        return;

    if (ssaBuilder.currentBlockKind.top() == blk_if_then)//left operand of phi should be modified
    {
        //Operand index 1 should be changed

        shared_ptr<IRFormat> irCode = ssaBuilder.updatePhiFunction(x,x_sym,defined, 1, IRpc);
        if (irCode != NULL) {
            IRCodes.push_back(irCode);
            IRpc++;
        }
    }
    else if (ssaBuilder.currentBlockKind.top() == blk_if_else)//right opernad of phi should be modified
    {
        //Operand index 2 should be changed
        shared_ptr<IRFormat> irCode = ssaBuilder.updatePhiFunction(x,x_sym,defined, 2, IRpc);
        if (irCode != NULL) {
            IRCodes.push_back(irCode);
            IRpc++;
        }
    }
    else if (ssaBuilder.currentBlockKind.top() == blk_while_body) {
        shared_ptr<IRFormat> phiCode = ssaBuilder.updatePhiFunction(x,x_sym,defined, 2, IRpc);
        if (phiCode != NULL) { //there is new phi
            //Between assignment variables in blk_while_body and blk_while_cond are now defined in phi
            int conditionBlockNum = ssaBuilder.getCurrentJoinBlockNum();
            string currentFunc = scopeStack.top();
            vector<shared_ptr<BasicBlock>> basicBlockList = functionList.at(currentFunc);
            for (int i = conditionBlockNum; i < currentBlock->getBlockNum(); i++) {
                //for block number i,
                shared_ptr<BasicBlock> targetBlock = basicBlockList.at(i - ssaBuilder.getStartBlock());
                defChangedToPhi(x,phiCode,targetBlock->irCodes);
                defChangedToPhi(x,phiCode,targetBlock->phiCodes);
            }
            //current Block
            defChangedToPhi(x,phiCode,currentBlock->irCodes);
            defChangedToPhi(x,phiCode,currentBlock->phiCodes);
            IRCodes.push_back(phiCode);
            IRpc++;
        }

    }
}

void Parser:: defChangedToPhi(string x,shared_ptr<IRFormat> phiCode, vector<shared_ptr<IRFormat>> irCodes)
{
    Result defJustBefore = ssaBuilder.getDefBeforeInserted();
    Kind defJustBeforeKind = defJustBefore.getKind();
    for (auto &irCode : irCodes) {

        //Update global variable definition for function call
        /*
        if(irCode->getIROP() == IR_bra)
        {
            Result locationOperand = irCode->operands.at(0);
            if(irCode->operands.at(0).isDiffFuncLoc())//For function call
            {
                for(auto &globalDefInfo : locationOperand.globalDefInfo)
                {
                    string defString = globalDefInfo.first;
                    GlobalDefInfo *GdefInfo = &globalDefInfo.second;
                    if(defJustBeforeKind == GdefInfo->defInfo.getKind())
                    {
                        switch (defJustBeforeKind) {
                            case constKind:
                                if (defJustBefore.getConst() == GdefInfo->defInfo.getConst() && x == defString)
                                    GdefInfo->defInfo.setInst(phiCode->getLineNo(), phiCode);
                                break;
                            case varKind:
                                if (defJustBefore.getVariableName() == GdefInfo->defInfo.getVar() &&
                                    defJustBefore.getDefInst() == GdefInfo->defInfo.getDefinedInstOfVar())
                                    GdefInfo->defInfo.setInst(phiCode->getLineNo(), phiCode);
                                break;
                            case instKind:
                                if (defJustBefore.getInst()->getLineNo() == GdefInfo->defInfo.getInstNum())
                                    GdefInfo->defInfo.setInst(phiCode->getLineNo(), phiCode);
                                break;
                        }
                    }
                }
            }
        }*/


        for (auto &operand : irCode->operands) {

            if (defJustBeforeKind == operand.getKind()) {
                switch (defJustBeforeKind) {
                    case constKind:
                        if (defJustBefore.getConst() == operand.getConst()&& x == operand.getConstPropVar()) {
                            int cost = phiCode->getCost();
                            phiCode->setCost(cost + pow(10,loopDepth));
                            operand.setInst(phiCode);
                        }

                        break;
                    case varKind:
                        if (defJustBefore.getVariableName() == operand.getVariableName() &&
                            defJustBefore.getDefInst() == operand.getDefInst()){
                            int cost = phiCode->getCost();
                            phiCode->setCost(cost + pow(10,loopDepth));
                            operand.setInst(phiCode);
                        }

                        break;
                    case instKind:
                        if (defJustBefore.getInst()->getLineNo() == operand.getInst()->getLineNo())
                        {
                            int cost = phiCode->getCost();
                            phiCode->setCost(cost + pow(10,loopDepth));
                            operand.setInst(phiCode);
                        }
                        break;
                }

            }
        }

    }
}



void Parser::cseForLoad(int dominatingBlockNum) {
/*
    vector<shared_ptr<IRFormat>> loadsCSEAvail;
    //For all loads
    for(auto loadInst : cseTracker.candidateLoadInstructions)
    {
        shared_ptr<IRFormat> storeInst = cseTracker.getCurrentInstPtr(IR_load);
        bool cseAvail = true;
        while(storeInst->getBlkNo() >= dominatingBlockNum)
        {
            if(storeInst->getIROP() == IR_store)
            {
                //Must be killed
                if(loadInst->operands.at(0).getVariableName() == storeInst->operands.at(1).getVariableName()) {
                    cseAvail = false;
                    break;
                }
            }
            storeInst = storeInst->getPreviousSameOpInst();
        }
        if(cseAvail)
            loadsCSEAvail.push_back(loadInst);
    }

    auto rit = loadsCSEAvail.rbegin();
    for(; rit!= loadsCSEAvail.rend(); ++rit) {
        shared_ptr<IRFormat> loadAvail = *rit;

        for (int i = dominatingBlockNum; i < currentBlock->getBlockNum(); i++) {
            string currentFunc = scopeStack.top();
            vector<shared_ptr<BasicBlock>> basicBlockList = functionList.at(currentFunc);
            shared_ptr<BasicBlock> targetBlock = basicBlockList.at(i - ssaBuilder.getStartBlock());

            if(loadAvail->getBlkNo() == i)
            {
                targetBlock->irCodes.erase(remove(targetBlock->irCodes.begin(),
                                                      targetBlock->irCodes.end(),loadAvail),
                                                                     targetBlock->irCodes.end());
            }

            //Replace instructions of load
            for (auto &irCode : targetBlock->irCodes) {
                for (auto &operand : irCode->operands) {
                     if(operand.getKind() == instKind && (operand.getInst()->getLineNo() == loadAvail->getLineNo()))
                            operand.setInst(loadAvail->getCommonSub());
                }
            }

            for (auto &irCode : targetBlock->phiCodes) {
                for (auto &operand : irCode->operands) {
                    if(operand.getKind() == instKind && (operand.getInst()->getLineNo() == loadAvail->getLineNo()))
                            operand.setInst(loadAvail->getCommonSub());
                }
            }
        }
    }

    /*
    for (int i = dominatingBlockNum; i < currentBlock->getBlockNum(); i++) {
        string currentFunc = scopeStack.top();
        vector<shared_ptr<BasicBlock>> basicBlockList = functionList.at(currentFunc);
        shared_ptr<BasicBlock> targetBlock = basicBlockList.at(i - ssaBuilder.getStartBlock());

        //Remove all loads
        for(auto loadAvail : loadsCSEAvail)
        {
            if(loadAvail->getBlkNo() == i)
            {
                targetBlock->irCodes.erase(remove(targetBlock->irCodes.begin(),
                                                  targetBlock->irCodes.end(),loadAvail),
                                           targetBlock->irCodes.end());
            }
        }

        //Replace instructions of load
        for (auto &irCode : targetBlock->irCodes) {
            for(auto loadAvail : loadsCSEAvail)
            {
                for (auto &operand : irCode->operands) {

                    if(operand.getKind() == instKind && (operand.getInst()->getLineNo() == loadAvail->getLineNo()))
                        operand.setInst(loadAvail->getCommonSubForLoad());
                }
            }
        }


        for (auto &irCode : targetBlock->phiCodes) {
            for (auto &operand : irCode->operands) {
                for(auto loadAvail : loadsCSEAvail)
                {
                    if(operand.getKind() == instKind && (operand.getInst()->getLineNo() == loadAvail->getLineNo()))
                        operand.setInst(loadAvail->getCommonSub());
                }
            }
        }
    }
    cseTracker.candidateLoadInstructions.clear();*/
}


void Parser::cseForWhileInst(int dominatingBlockNum) {

    vector<shared_ptr<IRFormat>> CSEAvail;
    for(auto cseCandidate : cseTracker.candidateCSEInstructions)
    {
        if(cseCandidate->getIROP() == IR_load && cseCandidate->operands.at(0).isArrayInst())
        {
            bool cseAvail = true;
            auto rit = cseTracker.killingStores.rbegin();
            for(; rit!= cseTracker.killingStores.rend(); ++rit)
            {
                shared_ptr<IRFormat> killStore = *rit;
                if(killStore->getBlkNo() >= dominatingBlockNum)
                {
                    if(cseCandidate->operands.at(0).getVariableName() == killStore->operands.at(1).getVariableName()) {
                        cseAvail = false;
                        break;
                    }
                }
            }
            /*
            shared_ptr<IRFormat> storeInst = cseTracker.getCurrentInstPtr(IR_load);

            while(storeInst->getBlkNo() >= dominatingBlockNum)
            {
                if(storeInst->getIROP() == IR_store)
                {
                    //Must be killed
                    if(cseCandidate->operands.at(0).getVariableName() == storeInst->operands.at(1).getVariableName()) {
                        cseAvail = false;
                        break;
                    }
                }
                storeInst = storeInst->getPreviousSameOpInst();
            }*/
            if(cseAvail)
                CSEAvail.push_back(cseCandidate);
        }
        else
        {
            CSEAvail.push_back(cseCandidate);
        }

    }


    auto rit = CSEAvail.begin();
    for(; rit!= CSEAvail.end(); ++rit) {
        shared_ptr<IRFormat> cseAvail = *rit;
        shared_ptr<IRFormat> commonSub = cseTracker.findExistingCommonSub(cseAvail);

        if(commonSub == NULL)//No commonsub until now
        {
            IROP irOp = cseAvail->getIROP();
            shared_ptr<IRFormat> previousSameOpInst = cseTracker.getCurrentInstPtr(irOp);
            if((previousSameOpInst == NULL) || (previousSameOpInst->getLineNo() < cseAvail->getLineNo()))
            {
                cseAvail->setPreviousSameOpInst(previousSameOpInst);
                cseTracker.setCurrentInst(irOp,cseAvail);
            }
            continue;
        }

        for (int i = dominatingBlockNum; i < currentBlock->getBlockNum(); i++) {
            string currentFunc = scopeStack.top();
            vector<shared_ptr<BasicBlock>> basicBlockList = functionList.at(currentFunc);
            shared_ptr<BasicBlock> targetBlock = basicBlockList.at(i - ssaBuilder.getStartBlock());

            if(cseAvail->getBlkNo() == i)
            {
                targetBlock->irCodes.erase(remove(targetBlock->irCodes.begin(),
                                                  targetBlock->irCodes.end(),cseAvail),
                                                             targetBlock->irCodes.end());
                //if load is deleted, according adda also deleted
                if(cseAvail->getIROP() == IR_load)
                {
                    targetBlock->irCodes.erase(remove(targetBlock->irCodes.begin(),
                                                      targetBlock->irCodes.end(),cseAvail->operands.at(0).getInst()),
                                               targetBlock->irCodes.end());
                }
            }

            for (auto &irCode : targetBlock->irCodes) {

                for (auto &operand : irCode->operands) {
                    if(operand.getKind() == instKind && (operand.getInst()->getLineNo() == cseAvail->getLineNo()))
                        operand.setInst(commonSub);
                }
            }

            for (auto &irCode : targetBlock->phiCodes) {
                for (auto &operand : irCode->operands) {
                    if(operand.getKind() == instKind && (operand.getInst()->getLineNo() == cseAvail->getLineNo()))
                        operand.setInst(commonSub);
                }
            }
        }
    }
    cseTracker.revertToOuter(dominatingBlockNum,true);
    cseTracker.candidateCSEInstructions.clear();
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
        PutF1(OP_ADDI, x.regNo,0,x.value);
        x.kind = regKind;
    }
    else if(x.kind == varKind)
    {
        x.regNo = AllocateReg();
        //Emit code for (value in Mem[globalBase + x.address] -> value in x.regNo)
        PutF1(OP_LDW,x.regNo,globalBase,x.address);
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
        op = OP_ADD;
    else if (computeToken == minusToken)
        op = OP_SUB;
    else if (computeToken == timesToken)
        op = OP_MUL;
    else if (computeToken == divToken)
        op = OP_DIV;
    else {
        std::cerr << "Only+,-,*,/ token can be allowed" << std::endl;
        return;
    }

    if (x.kind == constKind && y.kind == constKind)
    {
        if (op == OP_ADD)
            x.value += y.value;
        else if(op == OP_SUB)
            x.value -= y.value;
        else if(op == OP_MUL)
            x.value *= y.value;
        else if(op == OP_DIV)
            x.value /= y.value;
    }
    else if(y.kind == constKind)
    {
        Load(x); //value of x will be stored in a register
        PutF1(lmmOp((OpCode)op),x.regNo, x.regNo, y.value);
    }
        //
    else
    {
        Load(x);Load(y);
        PutF1(op,x.regNo,x.regNo,y.regNo);
        DeAllocateReg(y.regNo);
    }

}*/