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
    numOfBlock = 0;
    depth = 0;
    currentBlock = BasicBlock(0,blk_entry);
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
    //emitIntermediate(IR_bra,{tempJumpLoc});
    //Default block for jump to the main
    //finalizeAndStartNewBlock(false);

    if(scannerSym == mainToken)
    {
        scopeStack.push("main"); //main function scope start
        unordered_map<int,BasicBlock> emptyBasicBlockList;
        functionList.insert({"main",emptyBasicBlockList});
        depth = 1; //main, function, procedure are considered level 1

        Next(); //Consume main
        while(isVarDecl(scannerSym))
            varDecl();
        predefinedFunc(); //Predefined functions
        while(isFuncDecl(scannerSym)) {
            //At the start of block in function, the start block dominates itself
            stack<int> dominatedBy;
            dominatedBy.push(currentBlock.getBlockNum());
            dominatedByInfo.insert({currentBlock.getBlockNum(),dominatedBy});
            funcDecl();
        }
        if(scannerSym == beginToken)
        {
            //At the start of block in main, the start block dominates itself
            stack<int> dominatedBy;
            dominatedBy.push(currentBlock.getBlockNum());
            dominatedByInfo.insert({currentBlock.getBlockNum(),dominatedBy});
            ssaBuilder = SSABuilder("main", currentBlock.getBlockNum(), IRpc);
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
            unordered_map<int,BasicBlock> emptyBasicBlockList;
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
                    ssaBuilder = SSABuilder(symName, currentBlock.getBlockNum(), IRpc);
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
                        Result offset = getAddressInStack(RETURN_IN_STACK);
                        emitIntermediate(IR_bra,{offset});
                    }

                    //At the end of function means end of the block
                    finalizeAndStartNewBlock(blk_entry, false,false,false);

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
        Symbol x = typeDecl();
        if(scannerSym == identToken)
        {
            addVarSymbol(scanner->id,x.getSymType(),x.arrayCapacity);
            Next();
            while(scannerSym == commaToken)
            {
                Next(); // Consume comma
                if(scannerSym == identToken) //Array do not allow multiple declaration. So it's just variable
                {
                    addVarSymbol(scanner->id,x.getSymType(),x.arrayCapacity);
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

Symbol Parser::typeDecl() {

    Scanner *scanner = Scanner::instance();
    Symbol result;
    if(scannerSym == varToken)
    {
        result.setSymType(sym_var); //Var type
        Next(); //Consume var Token
    }
    else if(scannerSym == arrToken)
    {
        result.setSymType(sym_array); // Array type
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
    Result x;
    if(scannerSym == returnToken)
    {
        Next();
        if(isExpression(scannerSym))
        {
            x = expression();
            Result ret;ret.setReg(RETURNADDRESS);
            emitIntermediate(IR_move,{x,ret});
            Result offset = getAddressInStack(RETURN_IN_STACK);
            Result x = emitIntermediate(IR_load,{offset});
            emitIntermediate(IR_bra,{x});
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
            //At the start of while, new Block starts
            if(!finalizeAndStartNewBlock(blk_while_cond, false, true,true)) //Already New block is made(just change the name of block)
            {
                currentBlock.setBlockKind(blk_while_cond);
            }

            int dominatingBlockNum = currentBlock.getBlockNum();
            int conditionBlockNum = dominatingBlockNum;

            x = relation(); //Result is instruction with relational operator
            CondJF(x); // x.fixloc indicate that the destination should be fixed

            ssaBuilder.startJoinBlock(currentBlock.getBlockKind(),currentBlock.getBlockNum());

            //After the condition, also new block starts
            finalizeAndStartNewBlock(blk_while_body, true, true,true); //while block automatically dominated by cond block

            ssaBuilder.currentBlockKind.push(blk_while_body);


            if(scannerSym == doToken)
            {
                depth++;//depth increases
                Next();
                if(isStatSequence(scannerSym))
                {
                    statSequence();
                    if(scannerSym == odToken)
                    {

                        Next();
                        //Next to od token
                        follow = x;
                        follow.setFixLoc(follow.getFixLoc() - 1);//jump one instruction more because while should check condition for every iteration
                        //cmp <- follow.fixloc
                        //bsh <- x.fixloc

                        currentBlock.CFGForwardEdges.push_back(instructionBlockPair.at(follow.getFixLoc())); //Connect forward edge to the point to the unconditional branch
                        UnCJF(follow); //unconditional branch follow.fixloc


                        //After inner block, ssa numbering should be revert to the previous state
                        ssaBuilder.revertToOuter(currentBlock.getBlockNum());
                        cseTracker.revertToOuter(currentBlock.getBlockNum());
                        if(finalizeAndStartNewBlock(blk_while_end, false, false,false))//After unconditional jump means going back without reservation
                            updateBlockForDT(dominatingBlockNum);

                        vector<shared_ptr<IRFormat>> phiCodes = ssaBuilder.getPhiCodes();
                        //vector<IRFormat> irCodes = phiCodes;
                        //irCodes.insert(irCodes.end(),std::make_move_iterator(joinBlockCodes.begin()),std::make_move_iterator(joinBlockCodes.end()));
                        updatePhiInBB(conditionBlockNum, phiCodes);
                        ssaBuilder.currentBlockKind.pop();

                        depth--;
                        Fixup((unsigned long)x.getFixLoc()); //fix so that while branch here
                        ssaBuilder.endJoinBlock();

                        for(auto code : phiCodes)
                        {
                            //Phi is also kind of definition(defined kind: inst)
                            string targetOperand = code->operands.at(0).getVariable();
                            Result definedOperand;
                            definedOperand.setInst(code->getLineNo());

                            DefinedInfo defInfo(currentBlock.getBlockNum(),targetOperand);
                            defInfo.setInst(code->getLineNo());

                            ssaBuilder.prepareForProcess(targetOperand, defInfo);
                            ssaBuilder.insertDefinedInstr();
                            //If there is outer join block propagate
                            emitOrUpdatePhi(targetOperand,definedOperand);
                        }
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
            int dominatingBlockNum = currentBlock.getBlockNum();

            //Join Block create
            ssaBuilder.startJoinBlock(currentBlock.getBlockKind(),currentBlock.getBlockNum());

            //In if statement, after the condition new block starts
            if(!finalizeAndStartNewBlock(blk_if_then, true, true,true)) //Automatically dominated by previous block
            {
                currentBlock.setBlockKind(blk_if_then);
            }
            ssaBuilder.currentBlockKind.push(blk_if_then);
            if(scannerSym == thenToken)
            {
                Next();
                follow.setKind(instKind);
                follow.setFixLoc(0);
                depth++; //in the then block depth increase
                if(isStatSequence(scannerSym))
                {
                    statSequence();
                    if(scannerSym == elseToken)
                    {
                        Next();
                        UnCJF(follow);
                        //Until this point, still in the then block

                        //After inner block, ssa numbering should be revert to the previous state
                        ssaBuilder.revertToOuter(currentBlock.getBlockNum());
                        cseTracker.revertToOuter(currentBlock.getBlockNum());
                        //The start of else is new block
                        finalizeAndStartNewBlock(blk_if_else, false, false,false);//if then is performed it should avoid else
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
                        ssaBuilder.revertToOuter(currentBlock.getBlockNum());
                        cseTracker.revertToOuter(currentBlock.getBlockNum());
                        if(finalizeAndStartNewBlock(blk_if_end, false, true,false)) //After all if related statements end, new block start
                            updateBlockForDT(dominatingBlockNum);//if.end block should be dominated by condition
                    }
                    else {
                        //After inner block, ssa numbering should be revert to the previous state
                        ssaBuilder.revertToOuter(currentBlock.getBlockNum());
                        cseTracker.revertToOuter(currentBlock.getBlockNum());
                        if(finalizeAndStartNewBlock(blk_if_end, false, true,false))//After all if related statements end, new block start
                            updateBlockForDT(dominatingBlockNum);//if.end block should be dominated by condition
                        Fixup((unsigned long) x.getFixLoc());
                    }
                    ssaBuilder.currentBlockKind.pop();
                    depth--; // end of the then and depth decreases
                    //updateBlockForDT(dominatingBlockNum);//if.end block should be dominated by condition

                    if(scannerSym == fiToken)
                    {

                        vector<shared_ptr<IRFormat>> phiCodes = ssaBuilder.getPhiCodes();
                        ssaBuilder.endJoinBlock(); //go back to outer joinBlock

                        for(auto code : phiCodes)
                        {
                            code->setBlkNo(currentBlock.getBlockNum());//Fix join block num which was not correct at first.
                            currentBlock.phiCodes.push_back(code);//contents of join block is copied

                            //Phi is also kind of definition(defined kind: inst)
                            string targetOperand = code->operands.at(0).getVariable();
                            Result definedOperand;
                            definedOperand.setInst(code->getLineNo());

                            DefinedInfo defInfo(currentBlock.getBlockNum(),targetOperand);
                            defInfo.setInst(code->getLineNo());

                            ssaBuilder.prepareForProcess(targetOperand, defInfo);
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
            Symbol functionSym = symTableLookup(functionName, sym_func);
            numOfParam = functionSym.getNumOfParam(); //number of function parameter
            locationOfFunc = functionSym.getBaseAddr();  //function location(instruction number)
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
                        Result SP;SP.setReg(STACKPOINTER);
                        emitIntermediate(IR_store,{x,SP}); //We assume that SP is automatically adjusted (So SP is adjusted and then store them)
                        i++;
                        while(scannerSym == commaToken)
                        {
                            Next();
                            if(isExpression(scannerSym))
                            {
                                x = expression();
                                emitIntermediate(IR_store,{x,SP});
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
                Result jumpLocation;jumpLocation.setConst(locationOfFunc);
                result = emitIntermediate(IR_bra,{jumpLocation});
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
                    result = emitIntermediate(IR_move,{y,x});//No move instruction more, but return y:which is x(variable)'s phi updated value
                    //code just inserted before
                    if(x.getKind() == varKind)
                        emitOrUpdatePhi(x.getVariable(),result);//For variable x
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

                //Find the same expression in the list

                result = emitIntermediate(irOp, {x, y});
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

Result Parser::factor()
{
    Scanner *scanner = Scanner::instance();
    Result result, x;
    if(isDesignator(scannerSym))
    {
        result = designator();
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
        Symbol symInfo = symTableLookup(scanner->id, sym_var);
        x.setVariable(scanner->id);

        Next();

        int i = 0;
        while(scannerSym == openbracketToken)
        {
            Next();

            if(isExpression(scannerSym))
            {
                if(i > 0) {
                    Result capacityOfArray;
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
        if(symInfo.getSymType() == sym_array)
        {
            //x is array. So we have to provide address of x and index
            Symbol xSymbol = symTableLookup(x.getVariable(), sym_array);
            x.setConst(xSymbol.getBaseAddr());
            Result indexAdjust;indexAdjust.setConst(4);
            updatedIndex = emitIntermediate(IR_mul,{updatedIndex,indexAdjust}); // index * 4(word size)
            y.setReg(FRAMEPOINTER);//Frame pointer
            y = emitIntermediate(IR_add,{y,x});//y: base address(FP + x's base)
            result = emitIntermediate(IR_adda,{updatedIndex,y});//base address + index
            result.setArrayInst();
            //load or store should be determined in assignment
            //result = emitIntermediate(IR_load,{y});//load value of the index
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
    for(auto &x_i : x)
    {

        if(x_i.isArrayInst() && index == 1 && irOp == IR_move)//move to something array[]
            irOp = IR_store;
        else if(x_i.isArrayInst())//before instruction, emit load
        {
            //Make load instruction for it
            shared_ptr<IRFormat> ir_line(new IRFormat);
            ir_line->setBlkNo(currentBlock.getBlockNum());
            ir_line->setLineNo(IRpc);
            ir_line->setIROP(IR_load);
            ir_line->operands.push_back(x_i);

            IRCodes.push_back(ir_line);
            currentBlock.irCodes.push_back(ir_line);
            IRpc++;
            x_i.setInst(ir_line->getLineNo());
        }
        index++;
    }

    if(irOp == IR_move) //Copy Propagation : every move should be replaced by first operand
    {
        if(x.at(1).getVariable()== "k")
        {
            int sibal = 0;
        }
        string defVar = x.at(1).getVariable();
        DefinedInfo defInfo(currentBlock.getBlockNum(),defVar);

        Kind definedKind = x.at(0).getKind();

        if(definedKind == instKind)
            defInfo.setInst(x.at(0).getInst());
        else if(definedKind == varKind) {
            DefinedInfo defInfoOfDef;
            ssaBuilder.prepareForProcess(x.at(0).getVariable(),defInfoOfDef);
            defInfoOfDef = ssaBuilder.getDefinedInfo();
            defInfo = defInfoOfDef;//setVar(x.at(0).getVariable(), defInfoOfDef.getDefinedInstOfVar());
        }
        else if(definedKind == constKind)
            defInfo.setConst(x.at(0).getConst());

        ssaBuilder.prepareForProcess(defVar, defInfo);
        ssaBuilder.insertDefinedInstr();

        return x.at(0);
    }

    //Check whether it is array operation or not

    //Normal IR generation
    Result result;
    result.setInst(IRpc);
    shared_ptr<IRFormat> ir_line(new IRFormat);
    ir_line->setBlkNo(currentBlock.getBlockNum());
    ir_line->setLineNo(IRpc);
    ir_line->setIROP(irOp);

    index = 0;
    instructionBlockPair.insert({IRpc, currentBlock.getBlockNum()});//it is emitted now in current block


    for(auto x_i : x) {

        if(x_i.getKind() == varKind)
        {
            string var = x_i.getVariable();
            //Symbol varSym = symTableLookup(var);

            DefinedInfo definedInfo;
            ssaBuilder.prepareForProcess(var, definedInfo);
            definedInfo = ssaBuilder.getDefinedInfo();
            if(definedInfo.getKind() == instKind)
                x_i.setInst(definedInfo.getInst());
            else if(definedInfo.getKind() == constKind) {
                x_i.setConst(definedInfo.getConst());
                x_i.setConstPropVar(var);
            }
            else if(definedInfo.getKind() == varKind)
            {
                x_i.setVariable(definedInfo.getVar());
                x_i.setDefInst(definedInfo.getDefinedInstOfVar());//previously defined instr
            }
        }

        ir_line->operands.push_back(x_i);
        index++;
    }


    //Check whether elimination is possible(only for arithmetics?)
    if(irOp == IR_add || irOp == IR_mul || irOp == IR_div || irOp == IR_sub || irOp == IR_adda || irOp == IR_neg)
    {
        Result existingCommonSub = cseTracker.findExistingCommonSub(irOp,ir_line->operands);
        if(existingCommonSub.getKind() != errKind)
            return existingCommonSub;
    }
    //For common subexpression tracking

    int dominatingBlock;
    int dominatedBlock = currentBlock.getBlockNum();
    shared_ptr<IRFormat> previousSameOpInst = cseTracker.getCurrentInstPtr(irOp);
    if(previousSameOpInst == NULL)//if there is no instruction yet, it's the first->dominated itself
        dominatingBlock = dominatedBlock;
    else
        dominatingBlock = previousSameOpInst->getBlkNo();//should dominate

    ir_line->setPreviousSameOpInst(previousSameOpInst);
    //if dominatingBlock == dominatedBlock is same:overwrite, otherwise push
    bool sameBlock = false;
    if(previousSameOpInst == NULL && dominatingBlock == dominatedBlock)// initial element
        sameBlock = false;
    else if(previousSameOpInst != NULL && dominatingBlock == dominatedBlock)
        sameBlock = true;
    else if(dominatingBlock != dominatedBlock)
        sameBlock = false;
    cseTracker.setCurrentInst(irOp,ir_line,sameBlock);

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

        //only care about jump to the inst more than 1(no jump to main) and direct jump(do not allow jump to location stored in address)
        if(operandToChange->getConst() >= 0 && operandToChange->getKind() == constKind)
        {
            int blockNum = instructionBlockPair.at(operandToChange->getConst());
            operandToChange->setBlock(blockNum);
            //ir_line.operands.at(operandIndex) = operandToChange;
        }
    }

    currentBlock.irCodes.push_back(ir_line);

    IRpc++;
    return result;
}

string Parser :: getCodeString(shared_ptr<IRFormat> code)
{
    string tab = "  ";
    string result = to_string(code->getLineNo()) + tab + getIROperatorString(code->getIROP());
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
            result = result + tab + operand.getVariable()+"_"+to_string(operand.getDefInst());// << "\t";
        }
        else if(operand.getKind() == instKind) //Result of that instruction
        {
            result = result + tab + "(" + to_string(operand.getInst()) + ")";// << "\t";
        }
        else if(operand.getKind() == blockKind) //Result of that instruction
        {
            result = result + tab + "[" + to_string(operand.getBlock()) + "]";// << "\t";
        }
        else if(operand.getKind() == regKind)
        {
            result = result + tab + "%" + to_string(operand.getReg());// << "\t";
        }
        else {
            std::cerr << std::endl<< "No valid operand x"<< index << " : "<< operand.getKind() << std::endl;
        }
        index++;
    }
    return result;
}


void Parser::createControlFlowGraph(const string &graphFolder,const string &sourceFileName)
{
    string formatName = ".dot";
    RC rc = -1;
    for(auto function : functionList) {
        string functionName = function.first;
        string fileName = graphFolder + "CFG_"+ sourceFileName+ "_" + functionName + formatName;

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
        unordered_map<int, BasicBlock> basicBlockList = function.second;
        for (auto blockPair : basicBlockList) {
            BasicBlock block = blockPair.second;
            graphDrawer->writeNodeStart(block.getBlockNum(), block.getBlockName());
            for (auto code : block.phiCodes) {
                string codeString = getCodeString(code);
                graphDrawer->writeCode(codeString);
            }
            for (auto code : block.irCodes) {
                string codeString = getCodeString(code);
                graphDrawer->writeCode(codeString);
            }
            if (block.isCondBlock()) {
                graphDrawer->writeCodeForCond();
            }
            graphDrawer->writeNodeEnd();

            for (auto dest : block.CFGForwardEdges) {
                EDGETYPE edgeType = edge_normal;
                if (block.isCondBlock()) {
                    if (block.isTrueEdge(dest)) {
                        edgeType = edge_true;
                    }
                    else {
                        edgeType = edge_false;
                    }
                }
                graphDrawer->writeEdge(block.getBlockNum(), dest, edgeType);
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
        unordered_map<int, BasicBlock> basicBlockList = function.second;
        for (auto blockPair : basicBlockList) {
            BasicBlock block = blockPair.second;
            graphDrawer->writeNodeStart(block.getBlockNum(), block.getBlockName());
            for (auto code : block.phiCodes) {
                string codeString = getCodeString(code);
                graphDrawer->writeCode(codeString);
            }
            for (auto code : block.irCodes) {
                string codeString = getCodeString(code);
                graphDrawer->writeCode(codeString);
            }
            graphDrawer->writeNodeEnd();

            for (auto dest : block.DTForwardEdges) {
                EDGETYPE edgeType = edge_normal;
                graphDrawer->writeEdge(block.getBlockNum(), dest, edgeType);
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
        unordered_map<int,BasicBlock> basicBlockList = function.second;
        for(auto blockPair : basicBlockList)
        {
            BasicBlock block = blockPair.second;
            cout<<"Block " << block.getBlockNum() << " -------------------------------------" << endl;
            printIRCodes(block.phiCodes);
            printIRCodes(block.irCodes);
            cout<<"Forward Edge to";
            for(auto edge : block.CFGForwardEdges)
                cout << " " << edge;
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


void Parser::printIRCodes(vector<shared_ptr<IRFormat>> codes)
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
                std::cout << operand.getVariable() << "_" << operand.getDefInst();// << "\t";
            }
            else if(operand.getKind() == instKind) //Result of that instruction
            {
                std::cout << "(" << operand.getInst() << ")";// << "\t";
            }
            else if(operand.getKind() == blockKind) //Result of that instruction
            {
                std::cout << "[" << operand.getBlock() << "]";// << "\t";
            }
            else if(operand.getKind() == regKind)
            {
                std::cout << "%" << operand.getReg();// << "\t";
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
            Symbol sym = symIter.second;
            SymType symType = sym.getSymType();

            if(symType == sym_var)
            {
                cout << "\t" << "var";
                cout << " " << symIter.first;
                cout << "\t" << "Base Address: " << sym.getBaseAddr();
            }
            else if(symType == sym_param)
            {
                cout << "\t" << "param";
                cout << " " << symIter.first;
                cout << "\t" << "Base Address: " << sym.getBaseAddr();
            }
            else if(symType == sym_array)
            {
                cout << "\t" << "array ";
                for(auto cap : sym.arrayCapacity)
                {
                    cout << "[" << cap << "]";
                }
                cout << " " << symIter.first;
                cout << "\t" << "Base Address: " << sym.getBaseAddr();
            }

            else if(symType == sym_func)
            {
                cout << "\t" << "function";
                cout << "\t" << symIter.first;
                cout << "\t" << "Base Address: " << sym.getBaseAddr();
                cout << "\t" << "Number of Parameters: " << sym.getNumOfParam();
            }
            else if(symType == sym_proc)
            {
                cout << "\t" << "procedure";
                cout << "\t" << symIter.first;
                cout << "\t" << "Base Address: " << sym.getBaseAddr();
                cout << "\t" << "Number of Parameters: " << sym.getNumOfParam();
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
    Symbol functionSymbol(symType,IRpc,(int)numOfParam);
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
    int localVarTop = currentSymTable->getLocalVarTop() - varSize;//move go downwards
    //Make symbol
    Symbol newSymbol(symType,localVarTop,arrayCapacity);

    //Update symtable
    currentSymTable->varSymbolList.insert({symbol, newSymbol});

    currentSymTable->setLocalVarTop(localVarTop);

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
    Symbol newSymbol(sym_param, PARAM_IN_STACK + ((int)numOfParam - 1) - index);

    //Update symtable
    currentSymTable->varSymbolList.insert({symbol, newSymbol});
}

void Parser:: symbolTableUpdate(string var,Symbol varSym)
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



Symbol Parser:: symTableLookup(std::string symbol,SymType symType)
{
    Symbol nullInfo;
    string currentScope = scopeStack.top();

    while(currentScope != "")
    {
        auto currentSymTableIter = symTableList.find(currentScope);
        if(currentSymTableIter == symTableList.end())
        {
            std::cerr << "Current scope is not valid" << std::endl;
            return nullInfo;
        }


        SymTable currentSymTable = currentSymTableIter->second;
        unordered_map<string,Symbol> symbolList;

        //Distinguish varsym and funcsym
        if(symType == sym_var || symType == sym_param || symType == sym_array)
            symbolList = currentSymTable.varSymbolList;
        else if (symType == sym_func || symType == sym_proc)
            symbolList = currentSymTable.funcSymbolList;

        auto symIter = symbolList.find(symbol);
        if(symIter != symbolList.end())
            return symIter->second;

        currentScope = currentSymTable.getParent();
    }


    std::cerr << "There is no symbol like " << symbol << std::endl;

    return nullInfo; //There is no symbol
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
    unsigned long targetBlockNum = instructionBlockPair.at(loc);

    string currentScope = scopeStack.top();
    unordered_map<int,BasicBlock> *basicBlockList = &functionList.at(currentScope);
    BasicBlock *targetBlock = &basicBlockList->at(targetBlockNum);

    targetBlock->CFGForwardEdges.push_back(currentBlock.getBlockNum());
    //targetBlock.DTForwardEdges.push_back(currentBlockNum.getBlockNum());
    operationToChange->operands.at(operandToFix).setBlock(currentBlock.getBlockNum());

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


Result Parser:: getAddressInStack(int location)
{
    Result offset;offset.setConst(location);
    Result adjust;adjust.setConst(4);
    Result FP;FP.setReg(FRAMEPOINTER);
    offset = emitIntermediate(IR_mul,{offset,adjust});
    offset = emitIntermediate(IR_adda,{offset,FP});
    offset.setArrayInst();
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
    //Result ret;ret.setVariable(RETURNADDRESS);
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
    unordered_map<int, BasicBlock> *basicBlockList = &functionList.at(currentScope);
    BasicBlock *targetBlock = &basicBlockList->at(dominatingBlockNum);

    targetBlock->DTForwardEdges.push_back(
            currentBlock.getBlockNum());//after end of while -> dominated by dominating block(condition)


    stack<int> dominatedBy = dominatedByInfo.at(dominatingBlockNum);
    dominatedBy.push(currentBlock.getBlockNum());
    dominatedByInfo.insert({currentBlock.getBlockNum(),dominatedBy});
}


BasicBlock Parser::getBlockFromNum(int blockNum) {
    string currentScope = scopeStack.top();
    unordered_map<int, BasicBlock> basicBlockList = functionList.at(currentScope);
    return basicBlockList.at(blockNum);
}

void Parser::insertBasicBlock(BasicBlock block)
{
    string currentScope = scopeStack.top();
    unordered_map<int,BasicBlock> *basicBlockList = &functionList.at(currentScope);
    int blockNum = block.getBlockNum();
    basicBlockList->insert({blockNum,currentBlock});
    numOfBlock++;
}

void Parser::updatePhiInBB(int modifiedBlockNum, vector<shared_ptr<IRFormat>> phiCodes)
{
    string currentScope = scopeStack.top();
    unordered_map<int,BasicBlock> *basicBlockList = &functionList.at(currentScope);
    BasicBlock *modifiedBlock = &basicBlockList->at(modifiedBlockNum);
    modifiedBlock->phiCodes = phiCodes;
}


bool Parser::finalizeAndStartNewBlock(BlockKind newBlockKind, bool isCurrentCond, bool directFlowExist, bool dominate)
{
    //if(!currentBlock.irCodes.empty() || !currentBlock.phiCodes.empty()) {

        //currentBlockNum.setBlockName(currentBlockName);
        //string currentScope = scopeStack.top();
        //unordered_map<int,BasicBlock> basicBlockList = functionList.at(currentScope);
        int currentBlockNum = currentBlock.getBlockNum();

        //Current Block will be dominated by parent
        if(dominate) {
            currentBlock.DTForwardEdges.push_back(currentBlockNum + 1);
            stack<int> dominatedBy = dominatedByInfo.at(currentBlockNum);
            dominatedBy.push(currentBlockNum + 1);
            dominatedByInfo.insert({currentBlockNum + 1,dominatedBy});
        }

        //int nextBlockNum = numOfBlock + 1;
        if(directFlowExist) { //Make forward edge to next block
            currentBlock.CFGForwardEdges.push_back(currentBlockNum + 1);
            if(isCurrentCond)
                currentBlock.setTrueEdge(currentBlockNum + 1);
        }


        string currentScope = scopeStack.top();
        unordered_map<int,BasicBlock> *basicBlockList = &functionList.at(currentScope);
        int blockNum = currentBlock.getBlockNum();
        basicBlockList->insert({blockNum,currentBlock});
        numOfBlock++;

        //insertBasicBlock(currentBlock);
        currentBlock = BasicBlock(currentBlockNum + 1,newBlockKind);
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

    if (ssaBuilder.currentBlockKind.empty())//Currently not in the then, else block
        return;

    if (ssaBuilder.currentBlockKind.top() == blk_if_then)//left operand of phi should be modified
    {
        //Operand index 1 should be changed
        shared_ptr<IRFormat> irCode = ssaBuilder.updatePhiFunction(x,defined, 1, IRpc);
        if (irCode != NULL) {
            IRCodes.push_back(irCode);
            IRpc++;
        }
    }
    else if (ssaBuilder.currentBlockKind.top() == blk_if_else)//right opernad of phi should be modified
    {
        //Operand index 2 should be changed
        shared_ptr<IRFormat> irCode = ssaBuilder.updatePhiFunction(x,defined, 2, IRpc);
        if (irCode != NULL) {
            IRCodes.push_back(irCode);
            IRpc++;
        }
    }
    else if (ssaBuilder.currentBlockKind.top() == blk_while_body) {
        shared_ptr<IRFormat> phiCode = ssaBuilder.updatePhiFunction(x,defined, 2, IRpc);
        if (phiCode != NULL) { //there is new phi
            //Between assignment variables in blk_while_body and blk_while_cond are now defined in phi
            int conditionBlockNum = ssaBuilder.getCurrentJoinBlockNum();
            for (int i = conditionBlockNum; i < currentBlock.getBlockNum(); i++) {
                //for block number i,
                string currentFunc = scopeStack.top();
                unordered_map<int, BasicBlock> *basicBlockList = &functionList.at(currentFunc);
                BasicBlock *targetBlock = &basicBlockList->at(i);
                for (auto &irCode : targetBlock->irCodes) {
                    for (auto &operand : irCode->operands) {
                        Result defJustBefore = ssaBuilder.getDefBeforeInserted();
                        Kind defJustBeforeKind = defJustBefore.getKind();
                        if (defJustBeforeKind == operand.getKind()) {
                            switch (defJustBeforeKind) {
                                case constKind:
                                    if (defJustBefore.getConst() == operand.getConst()&& x == operand.getConstPropVar())
                                        operand.setInst(phiCode->getLineNo());
                                    break;
                                case varKind:
                                    if (defJustBefore.getVariable() == operand.getVariable() &&
                                        defJustBefore.getDefInst() == operand.getDefInst())
                                        operand.setInst(phiCode->getLineNo());
                                    break;
                                case instKind:
                                    if (defJustBefore.getInst() == operand.getInst())
                                        operand.setInst(phiCode->getLineNo());
                                    break;
                            }

                        }
                    }
                }
            }


            //current Block
            for (auto &irCode : currentBlock.irCodes) {
                for (auto &operand : irCode->operands) {
                    Result defJustBefore = ssaBuilder.getDefBeforeInserted();
                    Kind defJustBeforeKind = defJustBefore.getKind();
                    if (defJustBeforeKind == operand.getKind()) {
                        switch (defJustBeforeKind) {
                            case constKind:
                                if (defJustBefore.getConst() == operand.getConst()&& x == operand.getConstPropVar())
                                    operand.setInst(phiCode->getLineNo());
                                break;
                            case varKind:
                                if (defJustBefore.getVariable() == operand.getVariable() &&
                                    defJustBefore.getDefInst() == operand.getDefInst())
                                    operand.setInst(phiCode->getLineNo());
                                break;
                            case instKind:
                                if (defJustBefore.getInst() == operand.getInst())
                                    operand.setInst(phiCode->getLineNo());
                                break;
                        }

                    }
                }
            }
            IRCodes.push_back(phiCode);
            IRpc++;
        }

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