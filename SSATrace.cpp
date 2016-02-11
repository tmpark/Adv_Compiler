//
// Created by tmpark on 2/10/16.
//

#include "SSATrace.h"


SSATrace :: SSATrace(string functionName, int startBlock, int startInst)
{
    this->functionName = functionName;
    this->startBlock = startBlock;
    this->startInst = startInst;
}

void SSATrace :: prepareForProcess(string var,int blockNum, int instrNum)
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


void SSATrace :: insertDefinedInstr()
{
    if(!definitionExist)
    {
        DefinedLoc symDefined = {currentBlockNum, currentInstrNum};
        definedLocList.push(symDefined);
        definedLocTable.insert({varName, definedLocList});
        return;
    }

    DefinedLoc symDefined = definedLocList.top();
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


int SSATrace ::getDefinedInstr() {

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

void SSATrace :: traceBack()
{
    definedLocList.pop();
    definedLocTable.at(varName) = definedLocList;
    if(definedLocList.empty())
        definitionExist = false;
    return;
}
