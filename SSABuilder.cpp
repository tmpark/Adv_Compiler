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

void SSABuilder:: prepareForProcess(string var, int blockNum, int instrNum)
{
    auto definedLocListIter = definedLocTable.find(var);
    varName = var;
    if(definedLocTable.end() == definedLocListIter) //No information about var definition
    {
        //Initialize For First definition
        currentBlockNum = blockNum;
        currentInstrNum = instrNum;
        definedLocList = stack<DefinedLoc>();
        definitionExist = false;
    }
    else{
        currentBlockNum = blockNum;
        currentInstrNum = instrNum;
        definedLocList = definedLocListIter->second;
        if(definedLocList.empty()) //There was def but no longer exist because that def is inner scope and popped
            definitionExist = false;
        else
            definitionExist = true;
    }
    return;
}


void SSABuilder:: insertDefinedInstr()
{
    previousDef.setVariable(varName);//remember previously defined loc(for later use of ssa phi)

    if(!definitionExist)
    {
        DefinedLoc symDefined = {currentBlockNum, currentInstrNum};
        definedLocList.push(symDefined);
        definedLocTable.insert({varName, definedLocList});

        previousDef.setDefInst(startInst);//Previously defined instr
        return;
    }

    DefinedLoc symDefined = definedLocList.top();

    previousDef.setDefInst(symDefined.instNum);//previously defined instr

    if(symDefined.blockNum == currentBlockNum) //local numbering
    {
        symDefined.instNum = currentInstrNum;
        definedLocList.top() = symDefined;
    }
    else//global numbering
    {
        symDefined.blockNum = currentBlockNum;
        symDefined.instNum = currentInstrNum;
        definedLocList.push(symDefined);
    }

    definedLocTable.at(varName) = definedLocList; //update
    return;
}


int SSABuilder::getDefinedInstr() {

    if(!definitionExist) //No definition (global, parameter, or illegal usage)
    {
        //Make virtual Definition(Defined at the start of a function)
        DefinedLoc symDefined = {startBlock, startInst};
        definedLocList.push(symDefined);
        definedLocTable.insert({varName, definedLocList});
        return startInst;
    }
    //There is instruction
    return definedLocList.top().instNum;
}


void SSABuilder:: revertToOuter(int blockNum)
{
    for (auto &definedLocIter : definedLocTable)
    {
        stack<DefinedLoc> definedLocs = definedLocIter.second;
        while(!definedLocs.empty() && definedLocs.top().blockNum == blockNum)
        {
            definedLocs.pop();
        }
        definedLocIter.second = definedLocs;
    }
}

shared_ptr<IRFormat> SSABuilder:: updatePhiFunction(Result x, int operandIndex, int IRpc)
{
    //Phi operand is phi x, x1, x2
    for(auto &irCode : currentPhiCodes)
    {
        //if(irCode.getIROP() != IR_phi)
            //continue;
        Result var = irCode->operands.at((unsigned long)operandIndex);
        if(var.getVariable() == x.getVariable())//There is existing phi function
        {
            var.setDefInst(x.getDefInst());
            irCode->operands.at((unsigned long)operandIndex) = var;
            shared_ptr<IRFormat> nullCode = NULL;
            return nullCode;
        }
    }
    //There is no Phi function

    //make phi for the first time
    Result operand[3];
    operand[0] = x;
    operand[0].setDefInst(IRpc);
    operand[operandIndex] = x;
    int intactOperandIndex = (operandIndex == 1) ? 2 : 1;
    operand[intactOperandIndex] = previousDef;

    shared_ptr<IRFormat> irCode(new IRFormat(IRpc,IR_phi,operand[0],operand[1],operand[2]));
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

