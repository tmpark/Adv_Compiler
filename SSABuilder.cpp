//
// Created by tmpark on 2/10/16.
//

#include "SSABuilder.h"


SSABuilder::SSABuilder(string functionName, int startBlock, int startInst)
{
    this->functionName = functionName;
    this->startBlock = startBlock;
    this->startInst = startInst;
}

void SSABuilder:: prepareForProcess(string var,shared_ptr<Symbol> sym, DefinedInfo defInfo)
{
    auto definedInfoListIter = definedInfoTable.find(var);
    varName = var;
    varSym = sym;
    if(definedInfoTable.end() == definedInfoListIter) //No information about var definition
    {
        //Initialize For First definition
        currentDefInfo = defInfo;
        definedInfoList = stack<DefinedInfo>();
        definitionExist = false;
    }
    else{
        currentDefInfo = defInfo;
        definedInfoList = definedInfoListIter->second;
        if(definedInfoList.empty()) //There was def but no longer exist because that def is inner scope and popped
        {
            definedInfoTable.erase(var);
            definitionExist = false;
        }
        else
            definitionExist = true;
    }
    return;
}


void SSABuilder:: insertDefinedInstr()
{
    if(!definitionExist)
    {
        definedInfoList.push(currentDefInfo);
        definedInfoTable.insert({varName, definedInfoList});

        //If there is no def before, become variable with start instruction
        defBeforeInserted.setVariable(varName,varSym);//remember previously defined loc(for later use of ssa phi)
        defBeforeInserted.setDefInst(startInst);//Previously defined instr
        return;
    }

    DefinedInfo symDefined = definedInfoList.top();
    if(symDefined.getKind() == instKind)
        defBeforeInserted.setInst(symDefined.getInst());
    else if(symDefined.getKind() == constKind)
        defBeforeInserted.setConst(symDefined.getConst());
    else if(symDefined.getKind() == varKind)
    {
        defBeforeInserted.setVariable(symDefined.getVar(),symDefined.getVarSym());
        defBeforeInserted.setDefInst(symDefined.getDefinedInstOfVar());//previously defined instr
    }

    if(symDefined.ispreserved()) //local numbering
        definedInfoList.push(currentDefInfo);
    else//global numbering
        definedInfoList.top() = currentDefInfo;

    definedInfoTable.at(varName) = definedInfoList; //update
    return;
}


DefinedInfo SSABuilder::getDefinedInfo() {

    if(!definitionExist) //No definition (global, parameter, or illegal usage)
    {
        //Make virtual Definition(Defined at the start of a function)
        DefinedInfo symDefined(startBlock,varName);
        symDefined.setVar(varName,varSym,startInst);
        definedInfoList.push(symDefined);
        definedInfoTable.insert({varName, definedInfoList});
        return symDefined;
    }
    //There is instruction
    return definedInfoList.top();
}


void SSABuilder:: revertToOuter(int blockNum)
{
    //until getBlkNum dominate blockNum
    for (auto &definedLocIter : definedInfoTable)
    {
        stack<DefinedInfo> definedInfos = definedLocIter.second;
        while(!definedInfos.empty() && definedInfos.top().getBlkNum() > blockNum)
        {
            definedInfos.pop();
        }
        if(!definedInfos.empty())
            definedInfos.top().setPreserved(false);
        definedLocIter.second = definedInfos;
    }
}

void SSABuilder:: preserveOuter()
{
    for (auto &definedLocIter : definedInfoTable)
    {
        stack<DefinedInfo> definedInfos = definedLocIter.second;
        if(!definedInfos.empty()) {
            definedInfos.top().setPreserved(true);
            definedLocIter.second = definedInfos;
        }
    }
}

shared_ptr<IRFormat> SSABuilder:: updatePhiFunction(string x, shared_ptr<Symbol> x_sym,Result defined, int operandIndex, int IRpc)
{
    //Phi operand is phi x, x1, x2
    for(auto &irCode : currentPhiCodes)
    {
        //if(irCode.getIROP() != IR_phi)
            //continue;
        Result var = irCode->operands.at(0);
        if(var.getVariableName() == x)//There is existing phi function
        {
            irCode->operands.at((unsigned long)operandIndex) = defined;
            shared_ptr<IRFormat> nullCode = NULL;
            return nullCode;
        }
    }
    //There is no Phi function

    //make phi for the first time
    Result operand[3];
    operand[0].setVariable(x,x_sym);
    operand[0].setDefInst(IRpc);
    operand[operandIndex] = defined;//x;
    int intactOperandIndex = (operandIndex == 1) ? 2 : 1;
    operand[intactOperandIndex] = defBeforeInserted;

    //Fixme:for if, join block should be fixed
    shared_ptr<IRFormat> irCode(new IRFormat(currentJoinBlockNum,IRpc,IR_phi,operand[0],operand[1],operand[2]));
    //currentJoinBlock.irCodes.insert(currentJoinBlock.irCodes.begin(),irCode);
    currentPhiCodes.push_back(irCode);
    return irCode;
}

void SSABuilder::startJoinBlock(BlockKind blockKind,int joinBlockNum)
{
    previousPhiCodes.push(currentPhiCodes);
    previousJoinBlockKind.push(currentJoinBlockKind);
    previousJoinBlockNum.push(currentJoinBlockNum);
    currentPhiCodes = vector<shared_ptr<IRFormat>>();
    currentJoinBlockKind = blockKind;
    currentJoinBlockNum = joinBlockNum;
}
void SSABuilder::endJoinBlock()
{
    currentPhiCodes = previousPhiCodes.top();
    currentJoinBlockKind = previousJoinBlockKind.top();
    currentJoinBlockNum = previousJoinBlockNum.top();
    previousPhiCodes.pop();
    previousJoinBlockKind.pop();
    previousJoinBlockNum.pop();
}

