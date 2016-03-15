//
// Created by tmpark on 3/4/16.
//

#include "CodeGeneration.h"
#include "Helper.h"
#include <bitset>

CodeGeneration :: CodeGeneration(std::unordered_map<std::string,vector<shared_ptr<BasicBlock>>> functionList){
    this->functionList = functionList;
    loc = 0;
    numOfGlobalVar = 0;
}


void CodeGeneration::doCodeGen()
{
    Parser *parser = Parser :: instance();
    //Extract main block
    vector<shared_ptr<BasicBlock>> blockList = functionList.at(GLOBAL_SCOPE_NAME);

    //Prolog for main
    //get NumOfGlobalVar;

    shared_ptr<BasicBlock> firstBlock = blockList.at(0);
    startLocOfBlock.insert({firstBlock->getBlockNum(),loc});

    SymTable globalSymTable = parser->getSymTable(GLOBAL_SCOPE_NAME);
    numOfGlobalVar = globalSymTable.getNumOfVar();
    int globalVarSize = globalSymTable.getLocalVarSize();
    VIRTUAL_IN_STACK = LOCAL_IN_STACK - globalVarSize;
    int numOfVRegs = parser->getNumOfVirtualRegs(GLOBAL_SCOPE_NAME);
    assembleAndPutCode(OP_ADD,REG_SP,0,REG_GP);//Initialize SP as GP
    assembleAndPutCode(OP_ADD,REG_FP,0,REG_GP);//Initialize FP as GP

    //Move Stack pointer to reserve slots for face return, global variable, and, virtual regs
    assembleAndPutCode(OP_ADDI,REG_SP,REG_SP,-4*(1 + globalVarSize + numOfVRegs));

    genCodeForBlock(GLOBAL_SCOPE_NAME,firstBlock);

    for(auto function : functionList)
    {
        if(function.first == GLOBAL_SCOPE_NAME)
            continue;
        string functionName = function.first;
        blockList = function.second;
        firstBlock = blockList.at(0);
        startLocOfBlock.insert({firstBlock->getBlockNum(),loc});
        //Fuction Prolog

        //push previous fp
        int R_a = REG_FP;
        int R_b = REG_SP;
        int R_c = -4;
        assembleAndPutCode(OP_PSH, R_a, R_b, R_c);
        //FP value become SP
        R_c = 0;//Register 0
        assembleAndPutCode(OP_ADD, R_a, R_b, R_c);
        //Push return address
        R_a = REG_RET;
        int valC = -4;
        assembleAndPutCode(OP_PSH, R_a, R_b, valC);
        //Move SP to the amount of the size of all variable;
        SymTable symTable = parser->getSymTable(functionName);
        int localVarSize = symTable.getLocalVarSize();
        VIRTUAL_IN_STACK = LOCAL_IN_STACK - localVarSize;

        if(localVarSize != 0)
            assembleAndPutCode(OP_ADDI,REG_SP,REG_SP,-4*(localVarSize));//Move Stack pointer to reserve slots for variable
        //Move SP to the amount of virtual register
        numOfVRegs = parser->getNumOfVirtualRegs(functionName);
        if(numOfVRegs != 0)
            assembleAndPutCode(OP_ADDI,REG_SP,REG_SP,-4*numOfVRegs);//Move Stack pointer to reserve slots for virtual register

        //Register that will be used is stored and initialize that as 0
        vector<int> regUsedList = symTable.regUsedList;
        for(auto regUsed : regUsedList)
        {
            R_a = regUsed;
            R_b = REG_SP;
            valC = -4;
            assembleAndPutCode(OP_PSH, R_a, R_b, valC);
            assembleAndPutCode(OP_ADD, R_a, REG_0, REG_0);
        }


        for(auto symIter : symTable.varSymbolList)
        {
            shared_ptr<Symbol> sym = symIter.second;
            SymType symType = sym->getSymType();
            if(symType == sym_param)
            {
                int loc = sym->getBaseAddr()*4;
                int paramIndex = sym->getParamIndex();
                int allocatedReg = sym->getRegNo(functionName);
                if(allocatedReg != -1)
                {
                    //regTable[allocatedReg] = true;
                    R_a = allocatedReg;
                    if(paramIndex < 3)
                    {
                        R_b = REG_PARAM + paramIndex;
                        R_c = 0;
                        assembleAndPutCode(OP_ADD,R_a,R_b,R_c);
                    }
                    else
                    {
                        R_b = REG_FP;
                        int cVal = loc;
                        assembleAndPutCode(OP_LDW,R_a,R_b,cVal);
                    }
                }
            }
        }


        //For global variable register allocation that will be used
        for(auto symIter : globalSymTable.varSymbolList)
        {
            shared_ptr<Symbol> sym = symIter.second;
            if(sym->getSymType() == sym_var)
            {
                int loc = sym->getBaseAddr()*4;
                int allocatedReg = sym->getRegNo(functionName);
                if(allocatedReg != -1)
                {
                    //regTable[allocatedReg] = true;
                    R_a = allocatedReg;
                    R_b = REG_GP;
                    int cVal = loc;
                    assembleAndPutCode(OP_LDW,R_a,R_b,cVal);
                }
            }
        }


        //Real function Body
        genCodeForBlock(functionName,firstBlock);
        endLocOfFunc.insert({functionName,loc});

        //Function Epilog
        //if there is global variable defined, store it so that main also can get the changed value
        for(auto globalVarIter : symTable.definedGlobalVal)
        {
            string globalVarName = globalVarIter.first;
            DefinedInfo defInfo = globalVarIter.second;
            Kind defKind = defInfo.getKind();
            shared_ptr<Symbol> globalVarSym = globalSymTable.varSymbolList.at(globalVarName);
            int loc = globalVarSym->getBaseAddr()*4;
            if(defKind == constKind)
            {
                int cVal = defInfo.getConst();
                assembleAndPutCode(OP_ADDI,REG_PROXY,0,cVal);
                R_a = REG_PROXY;
            }
            else if(defKind == varKind)
            {
                shared_ptr<Symbol> definedVarSym = defInfo.getVarSym();
                R_a = definedVarSym->getRegNo(functionName);
            }
            else if(defKind == instKind)
            {
                shared_ptr<IRFormat> definedInst = defInfo.getInst();
                R_a = definedInst->getRegNo();
            }
            R_b = REG_GP;
            int cVal = loc;
            assembleAndPutCode(OP_STW,R_a,R_b,cVal);
        }

        //Return back registers' for caller
        while(!regUsedList.empty())
        {
            R_a = regUsedList.back();
            R_b = REG_SP;
            R_c = 4;
            assembleAndPutCode(OP_POP,R_a,R_b,R_c);
            regUsedList.pop_back();
        }
        if(numOfVRegs != 0)
            assembleAndPutCode(OP_ADDI,REG_SP,REG_SP,4*numOfVRegs);//Pop the virtual registers
        if(localVarSize != 0)
            assembleAndPutCode(OP_ADDI,REG_SP,REG_SP,4*(localVarSize));//Pop the local variable


        R_a = REG_RET;
        R_b = REG_SP;
        valC = 4;
        //get the return address back
        assembleAndPutCode(OP_POP, R_a, R_b, valC);
        //get the previous FP back
        R_a = REG_FP;
        assembleAndPutCode(OP_POP, R_a, R_b, valC);
        //Remove Parameter
        R_a = REG_SP;
        R_b = REG_SP;
        valC = symTable.getNumOfParam()*4;
        if(valC !=0)
            assembleAndPutCode(OP_ADDI, R_a, R_b, valC);
        //Return!
        assembleAndPutCode(OP_RET,REG_RET);
    }

    //Fixup
    for(auto fixInfo : locationTobeFixed)
    {
        int fixLoc = fixInfo.first;
        Result operandToFix = fixInfo.second;
        int blockTobeFixed = operandToFix.getBlockNo();

        int newLoc;
        if(operandToFix.getJumpLoc() > 0)
            newLoc = jumpLocOfBlock.at(blockTobeFixed);
        else
            newLoc = startLocOfBlock.at(blockTobeFixed);

        buf[fixLoc] = buf[fixLoc] & 0xFFFF0000;
        buf[fixLoc] = buf[fixLoc] | newLoc-fixLoc & 0x0000FFFF;
    }

    for(auto fixInfo : returnTobeFixed)
    {
        int fixLoc = fixInfo.first;
        string functionTobeFixed = fixInfo.second;

        int newLoc = endLocOfFunc.at(functionTobeFixed);
        buf[fixLoc] = buf[fixLoc] & 0xFFFF0000;
        buf[fixLoc] = buf[fixLoc] | newLoc-fixLoc & 0x0000FFFF;
    }

    /*
    for(int i = 0 ; i < loc ; i++)
    {
        cout << bitset<32>(buf.at(i)) << endl;
    }*/
}
void CodeGeneration::genCodeForBlock(string functionName, shared_ptr<BasicBlock> currentBlock)
{
    startLocOfBlock.insert({currentBlock->getBlockNum(),loc});
    //Generate Each code
    for(auto code : currentBlock->edgeCodes)
    {
        insertCode(functionName,code);
    }

    if(!currentBlock->edgeCodes.empty())
        jumpLocOfBlock.insert({currentBlock->getBlockNum(),loc});

    for(auto code : currentBlock->irCodes)
    {
        insertCode(functionName,code);
    }

    //NextBlock
    for(auto childBlock : currentBlock->DTForwardEdges)
        genCodeForBlock(functionName,childBlock);
}

void CodeGeneration::insertCode(string functionName, shared_ptr<IRFormat> code)
{
    if(code->isElimiated())
        return;
    IROP irOp = code->getIROP();
    size_t numOfIROperand = code->operands.size();

    //end, read, writeNL case
    if(numOfIROperand == 0)
    {
        if(irOp == IR_end)
        {
            int opCode = OP_RET;
            assembleAndPutCode(opCode,0);
        }
        else if(irOp == IR_read)
        {
            int opCode = OP_RDD;
            int instRegNum = code->getRegNo();
            assembleAndPutCode(opCode,instRegNum);
        }
        else if(irOp == IR_writeNL)
        {
            int opCode = OP_WRL;
            assembleAndPutCode(opCode);
        }
        return;
    }
    bool isFirstOperandConst = false;
    Result firstOperand = code->operands.at(0);
    if(firstOperand.getKind() == constKind)
        isFirstOperandConst = true;
    if(numOfIROperand == 1)
    {

        if(irOp == IR_load)
        {
            bool arrayLoad = firstOperand.isArrayInst();
            string stringName = firstOperand.getVariableName();
            int R_a = code->getRegNo();

            shared_ptr<IRFormat> actualCode = firstOperand.getInst();
            firstOperand = actualCode->operands.at(0);
            int R_b = firstOperand.getReg(functionName);
            Result secondOperand = actualCode->operands.at(1);

            if(arrayLoad) {
                Parser *parser = Parser::instance();
                SymTable symTable = parser->getSymTable(functionName);
                auto symIter = symTable.varSymbolList.find(stringName);
                int baseReg;
                int maxMemSize;
                if (symIter == symTable.varSymbolList.end())//which means global
                {
                    baseReg = REG_GP;
                    symTable = parser->getSymTable(GLOBAL_SCOPE_NAME);
                    shared_ptr<Symbol> sym = symTable.varSymbolList.at(stringName);
                    maxMemSize = sym->getVarSize() * 4;
                    if(secondOperand.getKind() == constKind)
                    {
                        assembleAndPutCode(OP_ADDI, REG_TEMP, REG_0, secondOperand.getConst());
                        assembleAndPutCode(OP_CHKI, REG_TEMP, maxMemSize);
                    }
                    else
                        assembleAndPutCode(OP_CHKI, secondOperand.getReg(functionName), maxMemSize);
                }
                else {
                    shared_ptr<Symbol> sym = symIter->second;
                    baseReg = REG_FP;
                    maxMemSize = sym->getVarSize() * 4;
                    if(secondOperand.getKind() == constKind)
                    {
                        assembleAndPutCode(OP_ADDI, REG_TEMP, REG_0, secondOperand.getConst());
                        assembleAndPutCode(OP_CHKI, REG_TEMP, maxMemSize);
                    }
                    else
                        assembleAndPutCode(OP_CHKI, secondOperand.getReg(functionName), maxMemSize);
                }
            }


            if(secondOperand.getKind() == constKind)
                assembleAndPutCode(OP_LDW,R_a,R_b,secondOperand.getConst());
            else
                assembleAndPutCode(OP_LDX,R_a,R_b,secondOperand.getReg(functionName));

        }
        else if(irOp == IR_bra) //should distinguish whether it is Normal jump or func call
        {
            //Function call
            if(firstOperand.isDiffFuncLoc())
            {
                int firstOperandVal = REG_RET;
                int opCode = OP_BSR;
                locationTobeFixed.insert({loc,firstOperand});
                assembleAndPutCode(opCode,firstOperandVal);

            }
            else if(firstOperand.getKind() == instKind){//not be used

            }
            else if(firstOperand.getKind() == regKind && firstOperand.isReturnAddr()) //return
            {
                int firstOperandVal = firstOperand.getReg(functionName);
                returnTobeFixed.insert({loc,functionName});
                assembleAndPutCode(OP_BSR,firstOperandVal);
            }
            else//General unconditional branch
            {
                int firstOperandVal = firstOperand.getBlockNo();
                int opCode = OP_BEQ;
                locationTobeFixed.insert({loc,firstOperand});
                assembleAndPutCode(opCode,0,firstOperandVal);
            }

        }
        else if(irOp == IR_write)
        {
            int outputReg;
            if(isFirstOperandConst)
            {
                assembleAndPutCode(OP_ADDI,REG_TEMP,0,firstOperand.getConst());
                outputReg = REG_TEMP;
            }
            else
                outputReg = firstOperand.getReg(functionName);
            assembleAndPutCode(OP_WRD,outputReg);
        }
        return;
    }
    bool isSecondOperandConst = false;
    Result secondOperand = code->operands.at(1);
    if(secondOperand.getKind() == constKind)
        isSecondOperandConst = true;

    if(numOfIROperand > 1)
    {
        //kinds of adds
        if(irOp == IR_add || irOp == IR_sub || irOp == IR_mul || irOp == IR_div || irOp == IR_cmp)
        {
            int destReg = code->getRegNo();
            int firstOperandReg;
            int secondOperandVal;

            OPFORMAT opFormat;
            if(isFirstOperandConst) {
                assembleAndPutCode(OP_ADDI, REG_TEMP, 0, firstOperand.getConst());
                firstOperandReg = REG_TEMP;
            }
            else
                firstOperandReg = firstOperand.getReg(functionName);

            if(isSecondOperandConst) {
                opFormat = OP_F1;
                secondOperandVal = secondOperand.getConst();
            }
            else {
                opFormat = OP_F2;
                secondOperandVal = secondOperand.getReg(functionName);
            }
            int opCode = irToOp(irOp,opFormat);
            assembleAndPutCode(opCode,destReg,firstOperandReg,secondOperandVal);
        }
        else if(irOp == IR_store)
        {
            int R_a;
            if(isFirstOperandConst) {
                assembleAndPutCode(OP_ADDI, REG_TEMP, 0, firstOperand.getConst());
                R_a = REG_TEMP;
            }
            else
                R_a = firstOperand.getReg(functionName);

            bool arrayStore = secondOperand.isArrayInst();
            string stringName = secondOperand.getVariableName();

            shared_ptr<IRFormat> actualCode = secondOperand.getInst();
            firstOperand = actualCode->operands.at(0);
            secondOperand = actualCode->operands.at(1);

            if(arrayStore) {
                Parser *parser = Parser::instance();
                SymTable symTable = parser->getSymTable(functionName);
                auto symIter = symTable.varSymbolList.find(stringName);
                int baseReg;
                int maxMemSize;
                if (symIter == symTable.varSymbolList.end())//which means global
                {
                    baseReg = REG_GP;
                    symTable = parser->getSymTable(GLOBAL_SCOPE_NAME);
                    shared_ptr<Symbol> sym = symTable.varSymbolList.at(stringName);
                    maxMemSize = sym->getVarSize() * 4;
                    if(secondOperand.getKind() == constKind)
                    {
                        assembleAndPutCode(OP_ADDI, REG_TEMP, REG_0, secondOperand.getConst());
                        assembleAndPutCode(OP_CHKI, REG_TEMP, maxMemSize);
                    }
                    else
                        assembleAndPutCode(OP_CHKI, secondOperand.getReg(functionName), maxMemSize);
                }
                else {
                    shared_ptr<Symbol> sym = symIter->second;
                    baseReg = REG_FP;
                    maxMemSize = sym->getVarSize() * 4;
                    if(secondOperand.getKind() == constKind)
                    {
                        assembleAndPutCode(OP_ADDI, REG_TEMP, REG_0, secondOperand.getConst());
                        assembleAndPutCode(OP_CHKI, REG_TEMP, maxMemSize);
                    }
                    else
                        assembleAndPutCode(OP_CHKI, secondOperand.getReg(functionName), maxMemSize);
                }
            }


            int R_b = firstOperand.getReg(functionName);

            if(secondOperand.getKind() == constKind)
                assembleAndPutCode(OP_STW,R_a,R_b,secondOperand.getConst());
            else
                assembleAndPutCode(OP_STX,R_a,R_b,secondOperand.getReg(functionName));




        }
        else if(irOp == IR_move)
        {

            int firstOperandReg;
            int secondOperandReg = secondOperand.getReg(functionName);
            int destReg = secondOperandReg;
            if(isFirstOperandConst) {
                assembleAndPutCode(OP_ADDI, REG_TEMP, 0, firstOperand.getConst());
                firstOperandReg = REG_TEMP;
            }
            else
                firstOperandReg = firstOperand.getReg(functionName);

            int opCode = OP_ADD;
            assembleAndPutCode(opCode,destReg,0,firstOperandReg);
        }
        else if(irOp == IR_bne || irOp == IR_beq || irOp == IR_ble || irOp == IR_blt || irOp == IR_bge|| irOp == IR_bgt)
        {
            int firstOperandReg = firstOperand.getReg(functionName);
            int secondOperandVal = secondOperand.getBlockNo();
            OPFORMAT opFormat = OP_F1;
            int opCode = irToOp(irOp,opFormat);
            locationTobeFixed.insert({loc,secondOperand});
            assembleAndPutCode(opCode,firstOperandReg,secondOperandVal);
        }
        else if(irOp == IR_miu)
        {

            int secondOperandReg = secondOperand.getReg(functionName);
            if(secondOperandReg == REG_SP)//should push to the stack
            {
                int R_a;
                int R_b = secondOperandReg;
                if(isFirstOperandConst) {
                    int val_c = firstOperand.getConst();
                    assembleAndPutCode(OP_ADDI, REG_TEMP, 0, val_c);
                    R_a = REG_TEMP;
                }
                else
                    R_a = firstOperand.getReg(functionName);
                assembleAndPutCode(OP_PSH, R_a, R_b, -4);
            }
            else//go to parameter register or return val reg
            {
                int R_a = secondOperandReg;
                int R_b = 0;
                int R_c;
                if(isFirstOperandConst) {
                    int val_c = firstOperand.getConst();
                    assembleAndPutCode(OP_ADDI, REG_TEMP, R_b, val_c);
                    R_c = REG_TEMP;
                }
                else
                    R_c = firstOperand.getReg(functionName);
                assembleAndPutCode(OP_ADD, R_a, R_b, R_c);

                //Even though parameters are inserted in reg, reserve the space
                if(secondOperandReg < REG_PARAM + NUM_OF_PARAM_REGS && secondOperandReg >= REG_PARAM)
                    assembleAndPutCode(OP_ADDI, REG_SP,REG_SP,-4);
            }
        }
        return;
    }

}

void CodeGeneration::assembleAndPutCode(int op)
{
    OpCode opCode = (OpCode)op;
    if(opCode != OP_WRL)
        cerr << "There is only one opcode for no operand : WRL" << endl;
    PutF1(op,0,0,0);
}

void CodeGeneration::storeForVirtualReg(int virReg, int proxyIndex)
{
    int vRegIndex = virReg - REG_VIRTUAL;
    int R_a = REG_PROXY + proxyIndex;
    int R_b = REG_FP;
    int valC = (VIRTUAL_IN_STACK - vRegIndex-1)*4;

    int valC_1 = valC;
    int valC_2 = valC;
    if(valC > pow(2,15) || valC < -pow(2,15)) //+ overflow
    {
        int rest = valC & 1;
        valC_1 = valC >> 1;
        valC_2 = valC_1;
        valC_2 = valC_2 + rest;
        PutF1(OP_ADDI,REG_TEMP + 1,REG_0,valC_1);
        PutF1(OP_ADDI,REG_TEMP + 1,REG_TEMP + 1,valC_2);
        PutF1(OP_STX,R_a,R_b,REG_TEMP + 1);

    }//make half and half(if odd number arg3_2 is 1 more)
    else
        PutF1(OP_STW,R_a,R_b,valC);
}

void CodeGeneration::loadForVirtualReg(int virReg, int proxyIndex)
{
    int vRegIndex = virReg - REG_VIRTUAL;
    int R_a = REG_PROXY + proxyIndex;
    int R_b = REG_FP;
    int valC = (VIRTUAL_IN_STACK - vRegIndex -1)*4;

    int valC_1 = valC;
    int valC_2 = valC;
    if(valC > pow(2,15) || valC < -pow(2,15)) //+ overflow
    {
        int rest = valC & 1;
        valC_1 = valC >> 1;
        valC_2 = valC_1;
        valC_2 = valC_2 + rest;
        PutF1(OP_ADDI,REG_TEMP + 1,REG_0,valC_1);
        PutF1(OP_ADDI,REG_TEMP + 1,REG_TEMP + 1,valC_2);
        PutF1(OP_LDX,R_a,R_b,REG_TEMP + 1);

    }//make half and half(if odd number arg3_2 is 1 more)
    else
        PutF1(OP_LDW,R_a,R_b,valC);
}

void CodeGeneration::assembleAndPutCode(int op, int arg1)
{
    OpCode opCode = (OpCode)op;
    switch (opCode) {
        // F1 Format
        case OP_BSR:
            PutF1(op,0,0,arg1);
            break;
        case OP_RDD:
            if(arg1 > 31) //Virtual Register
            {
                PutF1(op,REG_PROXY,0,0);
                storeForVirtualReg(arg1,0);
            }
            else
                PutF1(op,arg1,0,0);
            break;
        case OP_WRD:
        case OP_WRH:
            if(arg1 > 31)
            {
                loadForVirtualReg(arg1,0);
                arg1 = REG_PROXY;

            }
            PutF1(op,0,arg1,0);
            break;
            // F2 Format
        case OP_RET:
            PutF2(op,0,0,arg1);
            break;
            // F3 Format
        case OP_JSR:
            PutF3(op,arg1);
            break;
        default:
            cerr << "This opcode is not for one argument" << endl;
    }
}

void CodeGeneration::assembleAndPutCode(int op, int arg1, int arg2)
{
    OpCode opCode = (OpCode)op;
    switch (opCode) {
        // F1 Format
        case OP_CHKI:
            if(arg1 > 31)
            {
                loadForVirtualReg(arg1, 0);
                arg1 = REG_PROXY;
            }
            PutF1(op,arg1,0,arg2);
            break;
        case OP_BEQ:
        case OP_BNE:
        case OP_BLT:
        case OP_BGE:
        case OP_BLE:
        case OP_BGT:
            if(arg1 > 31)
            {
                PutF1(op,REG_PROXY,0,arg2);
                storeForVirtualReg(arg1,0);
            }
            else
                PutF1(op,arg1,0,arg2);
            break;
            // F2 Format
        case OP_CHK:
            if(arg1 > 31) {
                loadForVirtualReg(arg1, 0);
                arg1 = REG_PROXY;
            }
            if(arg2 > 31)
            {
                loadForVirtualReg(arg2, 1);
                arg2 = REG_PROXY + 1;
            }

            PutF2(op,arg1,0,arg2);
            break;
        default:
            cerr << "This opcode is not for two arguments" << endl;
    }
}

void CodeGeneration::assembleAndPutCode(int op, int arg1, int arg2, int arg3)
{
    int arg3_1 = arg3;
    int arg3_2 = arg3;
    if(arg3 > pow(2,15) || arg3 < -pow(2,15)) //+ overflow
    {
        int rest = arg3 & 1;
        arg3_1 = arg3 >> 1;
        arg3_2 = arg3_1;
        arg3_2 = arg3_2 + rest;
    }//make half and half(if odd number arg3_2 is 1 more)

    OpCode opCode = (OpCode)op;
    switch (opCode) {
        // F1 Format
        case OP_ADDI:
        case OP_SUBI:
        case OP_MULI:
        case OP_DIVI:
        case OP_MODI:
        case OP_CMPI:
        case OP_ORI:
        case OP_ANDI:
        case OP_BICI:
        case OP_XORI:
        case OP_LSHI:
        case OP_ASHI:
        case OP_LDW:
        case OP_POP:
            if(arg2 > 31)
            {
                loadForVirtualReg(arg2, 0);
                arg2 = REG_PROXY;
            }
            if(arg1 > 31)
            {
                if(arg3 > pow(2,15) || arg3 < -pow(2,15)) //+ overflow
                {
                    PutF1(OP_ADDI,REG_PROXY+1,REG_0,arg3_1);
                    PutF1(OP_ADDI,REG_PROXY+1,REG_PROXY+1,arg3_2);
                    PutF1(op-16,REG_PROXY,arg2,REG_PROXY+1);
                }
                else
                    PutF1(op,REG_PROXY,arg2,arg3);

                storeForVirtualReg(arg1,0);
            }
            else
            {
                if(arg3 > pow(2,15) || arg3 < -pow(2,15)) //+ overflow
                {
                    PutF1(OP_ADDI,REG_PROXY+1,REG_0,arg3_1);
                    PutF1(OP_ADDI,REG_PROXY+1,REG_PROXY+1,arg3_2);
                    PutF1(op-16,arg1,arg2,REG_PROXY+1);
                }
                else
                    PutF1(op,arg1,arg2,arg3);
            }
            break;


        case OP_STW:
        case OP_PSH:
            if(arg1 > 31)
            {
                loadForVirtualReg(arg1, 0);
                arg1 = REG_PROXY;
            }

            if(arg2 > 31)
            {
                loadForVirtualReg(arg2, 1);
                arg2 = REG_PROXY + 1;
            }

            if(arg3 > pow(2,15) || arg3 < -pow(2,15)) //+ overflow
            {
               //No cases until now
            }
            PutF1(op,arg1,arg2,arg3);

            break;

            // F2 Format
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
        case OP_CMP:
        case OP_OR:
        case OP_AND:
        case OP_BIC:
        case OP_XOR:
        case OP_LSH:
        case OP_ASH:
        case OP_LDX:
            if(arg2 > 31)
            {
                loadForVirtualReg(arg2, 0);
                arg2 = REG_PROXY;
            }
            if(arg3 > 31)
            {
                loadForVirtualReg(arg3, 1);
                arg3 = REG_PROXY+1;
            }
            if(arg1 > 31)
            {
                PutF1(op,REG_PROXY,arg2,arg3);
                storeForVirtualReg(arg1,0);
            }
            else
                PutF2(op,arg1,arg2,arg3);
            break;
        case OP_STX:
            if(arg1 > 31)
            {
                loadForVirtualReg(arg1, 0);
                arg1 = REG_PROXY;
            }

            if(arg2 > 31)
            {
                loadForVirtualReg(arg2, 1);
                arg2 = REG_PROXY + 1;
            }

            PutF1(op,arg1,arg2,arg3);
            break;
        default:
            cerr << "This opcode is not for three arguments" << endl;
    }
}

void CodeGeneration::PutF1(int op, int a, int b, int c) {
    buf.at(loc) = op << 26 |
                 a << 21 |
                 b << 16 |
                 c & 0x0000FFFF;
    loc++;
}

void CodeGeneration::PutF2(int op, int a, int b, int c) {
    buf.at(loc) = op << 26 |
                  a << 21 |
                  b << 16 |
                  c & 0x0000001F;
    loc++;
}

void CodeGeneration::PutF3(int op, int c) {
    buf.at(loc) = op << 26 |
                  c & 0x03FFFFFF;
    loc++;
}


void CodeGeneration::writeOutCode(const string &binaryFolder,const string &sourceFileName)
{
    string formatName = ".out";
    RC rc = -1;
    string fileName = binaryFolder +  sourceFileName + formatName;

    destroyFile(fileName);
    rc = createFile(fileName);
    if (rc == -1)
        return;
    rc = openFile(fileName);
    if (rc == -1) {
        destroyFile(fileName);
        return;
    }

    fileStream.write((char*)buf.data(),loc*sizeof(int32_t));
    fileStream.close();
    return;
}



RC CodeGeneration::createFile(const string &fileName)
{
    destroyFile(fileName);
    const char* fileName_char = fileName.c_str();
    fstream file_to_create;

    file_to_create.open(fileName_char, fstream::out | fstream:: binary); //Create a file (do not use in when creating a file)
    if(file_to_create.is_open())
    {
        file_to_create.close();
        return 1;
    }
    return -1;
}

RC CodeGeneration::openFile(const std::string &fileName) {
    fileStream.open(fileName.c_str(), std::fstream::in | std::fstream::out | fstream::binary);
    if(!fileStream.is_open())
    {
        return -1;
    }
    return 1;
}

RC CodeGeneration::destroyFile(const string &fileName)
{
    const char* fileName_char = fileName.c_str();
    RC success = remove(fileName_char); //delete file
    if(success == 0)
    {
        return 0; //successful
    }

    return -1; //A file still exists
}

RC CodeGeneration::closeFile() {
    fileStream.close();
    return 1;
}