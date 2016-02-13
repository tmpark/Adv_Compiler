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

IRFormat SSABuilder:: updatePhiFunction(Result x, int operandIndex, int IRpc)
{

    //Phi operand is phi x, x1, x2
    for(auto &irCode : currentPhiCodes)
    {
        //if(irCode.getIROP() != IR_phi)
            //continue;
        Result var = irCode.operands.at((unsigned long)operandIndex);
        if(var.getVariable() == x.getVariable())//There is existing phi function
        {
            var.setDefInst(x.getDefInst());
            irCode.operands.at((unsigned long)operandIndex) = var;
            IRFormat nullCode;
            return nullCode;
        }
    }
    //There is no Phi function

    //For while block variables under phi should be changed to indicate phi line
    if(currentJoinBlockKind == blk_while_cond)
    {
        for(auto& irCode : currentJoinBlockCodes)
        {
            for(auto& operand : irCode.operands)
            {
                if(operand.getKind() == varKind)
                {
                    if(operand.getVariable() == x.getVariable())
                        operand.setDefInst(IRpc);//IRpc would be future inst num of phi
                }
            }
        }
    }

    //make phi for the first time
    Result operand[3];
    operand[0] = x;
    operand[0].setDefInst(IRpc);
    operand[operandIndex] = x;
    int intactOperandIndex = (operandIndex == 1) ? 2 : 1;
    operand[intactOperandIndex] = previousDef;

    IRFormat irCode(IRpc,IR_phi,operand[0],operand[1],operand[2]);
    //currentJoinBlock.irCodes.insert(currentJoinBlock.irCodes.begin(),irCode);
    currentPhiCodes.push_back(irCode);
    return irCode;
}

void SSABuilder::startJoinBlock(BlockKind blockKind, vector<IRFormat> codes)
{
    previousJoinBlockCodes.push(currentJoinBlockCodes);
    previousPhiCodes.push(currentPhiCodes);
    previousJoinBlockKind.push(currentJoinBlockKind);
    currentJoinBlockCodes = codes;
    currentPhiCodes = vector<IRFormat>();
    currentJoinBlockKind = blockKind;
}
void SSABuilder::endJoinBlock()
{
    currentJoinBlockCodes = previousJoinBlockCodes.top();
    currentPhiCodes = previousPhiCodes.top();
    currentJoinBlockKind = previousJoinBlockKind.top();
    previousJoinBlockCodes.pop();
    previousPhiCodes.pop();
    previousJoinBlockKind.pop();
}

