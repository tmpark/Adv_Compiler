//
// Created by tmpark on 2/15/16.
//

#include "CSETracker.h"


CSETracker:: CSETracker()
{
}

shared_ptr<IRFormat> CSETracker::findExistingCommonSub(IROP irOp,vector<Result> operands)
{
    shared_ptr<IRFormat> currentInstPtr = getCurrentInstPtr(irOp);


    while(currentInstPtr != NULL)
    {
        if(irOp == IR_load)
        {
            if(currentInstPtr->getIROP() == IR_store) {
                //if the variable of array is the same kill(load var, store ~,var)
                if (operands.at(0).getVariable() == currentInstPtr->operands.at(1).getVariable())
                    return NULL;
                else {
                    currentInstPtr = currentInstPtr->getPreviousSameOpInst();
                    continue;
                }
            }
                //else let them go
        }
        //For all operands they should be same
        bool sameOperand = true;
        for(int i = 0 ; i < operands.size() ;i++)
        {
            if(!isSameOperand(currentInstPtr->operands.at(i),operands.at(i)))
                sameOperand = false;
        }

        //Special Case handling(for add and mul, reverse order also same)
        if(irOp == IR_mul || irOp == IR_add)
        {
            if(isSameOperand(currentInstPtr->operands.at(0),operands.at(1)) &&
                    isSameOperand(currentInstPtr->operands.at(1),operands.at(0)))
                sameOperand = true;
        }



        //Same operand
        if(sameOperand)
        {
            return currentInstPtr;
        }

        currentInstPtr = currentInstPtr->getPreviousSameOpInst();
    }

    return NULL;
}

//Fixme: Negate case insert

shared_ptr<IRFormat> CSETracker::getCurrentInstPtr(IROP irOp)
{
    switch(irOp)
    {
        case IR_add:
            return currentAddInst;
        case IR_adda:
            return currentAddaInst;
        case IR_sub:
            return currentSubInst;
        case IR_mul:
            return currentMulInst;
        case IR_div:
            return currentDivInst;
        case IR_neg:
            return currentNegInst;
        case IR_cmp:
            return currentCmpInst;
        case IR_load:
            return currentLoadInst;
        case IR_store:
            return currentLoadInst;
        default:
            return NULL;
    }
}
void CSETracker::setCurrentInst(IROP irOp, shared_ptr<IRFormat> currentInst)
{
    switch(irOp)
    {
        case IR_add:
                currentAddInst = currentInst;
            break;
        case IR_adda:
                currentAddaInst = currentInst;
            break;
        case IR_sub:
                currentSubInst = currentInst;
            break;
        case IR_mul:
                currentMulInst = currentInst;
            break;
        case IR_div:
                currentDivInst = currentInst;
            break;
        case IR_neg:
                currentNegInst = currentInst;
            break;
        case IR_cmp:
                currentCmpInst = currentInst;
            break;
        case IR_load:
                currentLoadInst = currentInst;
            break;
        case IR_store:
                currentLoadInst = currentInst;
            break;
        default:
            break;
    }
}

void CSETracker:: revertToOuter(int blockNum)
{
    vector<shared_ptr<IRFormat>> killingStores;
    int a;
    if(currentAddInst != NULL)
        a = currentAddInst->getBlkNo();
    while(!(currentAddInst == NULL || currentAddInst->getBlkNo() <= blockNum))
        currentAddInst = currentAddInst->getPreviousSameOpInst();
    while(!(currentSubInst == NULL || currentSubInst->getBlkNo() <= blockNum))
        currentSubInst = currentSubInst->getPreviousSameOpInst();
    while(!(currentMulInst == NULL || currentMulInst->getBlkNo() <= blockNum))
        currentMulInst = currentMulInst->getPreviousSameOpInst();
    while(!(currentDivInst == NULL || currentDivInst->getBlkNo() <= blockNum))
        currentDivInst = currentDivInst->getPreviousSameOpInst();
    while(!(currentAddaInst == NULL || currentAddaInst->getBlkNo() <= blockNum))
        currentAddaInst = currentAddaInst->getPreviousSameOpInst();
    while(!(currentNegInst == NULL || currentNegInst->getBlkNo() <= blockNum))
        currentNegInst = currentNegInst->getPreviousSameOpInst();
    while(!(currentCmpInst == NULL || currentCmpInst->getBlkNo() <= blockNum))
        currentCmpInst = currentCmpInst->getPreviousSameOpInst();
    while(!(currentLoadInst == NULL || currentLoadInst->getBlkNo() <= blockNum)) {
        if(currentLoadInst->getIROP() == IR_store)
            killingStores.push_back(currentLoadInst);
        currentLoadInst = currentLoadInst->getPreviousSameOpInst();
    }
    for(auto store : killingStores)
    {
        store->setPreviousSameOpInst(currentLoadInst);
        currentLoadInst = store;
    }
}

/*
void CSETracker::revertToOuter(IROP irOp)
{
    switch(irOp)
    {
        case IR_add:
            currentAddInst.pop();
            break;
        case IR_adda:
            currentAddaInst.pop();
            break;
        case IR_sub:
            currentSubInst.pop();
            break;
        case IR_mul:
            currentMulInst.pop();
            break;
        case IR_div:
            currentDivInst.pop();
            break;
        default:
            break;
    }
}
*/