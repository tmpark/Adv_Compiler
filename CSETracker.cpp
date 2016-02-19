//
// Created by tmpark on 2/15/16.
//

#include "CSETracker.h"


CSETracker:: CSETracker()
{
}

Result CSETracker::findExistingCommonSub(IROP irOp,vector<Result> operands)
{
    shared_ptr<IRFormat> currentInstPtr = getCurrentInstPtr(irOp);

    Result cseReturn;

    while(currentInstPtr != NULL)
    {
        //For all operands they should be same
        bool sameOperand = true;
        for(int i = 0 ; i < operands.size() ;i++)
        {
            vector<Result> temp = currentInstPtr->operands;
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
            cseReturn.setInst(currentInstPtr->getLineNo());
            return cseReturn;
        }

        currentInstPtr = currentInstPtr->getPreviousSameOpInst();
    }

    return cseReturn;
}

//Fixme: Negate case insert

shared_ptr<IRFormat> CSETracker::getCurrentInstPtr(IROP irOp)
{
    switch(irOp)
    {
        case IR_add:
            if(currentAddInst.empty())
                return NULL;
            return currentAddInst.top();
        case IR_adda:
            if(currentAddaInst.empty())
                return NULL;
            return currentAddaInst.top();
        case IR_sub:
            if(currentSubInst.empty())
                return NULL;
            return currentSubInst.top();
        case IR_mul:
            if(currentMulInst.empty())
                return NULL;
            return currentMulInst.top();
        case IR_div:
            if(currentDivInst.empty())
                return NULL;
            return currentDivInst.top();
        case IR_neg:
            if(currentNegInst.empty())
                return NULL;
            return currentNegInst.top();
        default:
            return NULL;
    }
}
void CSETracker::setCurrentInst(IROP irOp, shared_ptr<IRFormat> currentInst, bool sameBlock)
{
    switch(irOp)
    {
        case IR_add:
            if(sameBlock)
                currentAddInst.top() = currentInst;
            else
                currentAddInst.push(currentInst);
            break;
        case IR_adda:
            if(sameBlock)
                currentAddaInst.top() = currentInst;
            else
                currentAddaInst.push(currentInst);
            break;
        case IR_sub:
            if(sameBlock)
                currentSubInst.top() = currentInst;
            else
                currentSubInst.push(currentInst);
            break;
        case IR_mul:
            if(sameBlock)
                currentMulInst.top() = currentInst;
            else
                currentMulInst.push(currentInst);
            break;
        case IR_div:
            if(sameBlock)
                currentDivInst.top() = currentInst;
            else
                currentDivInst.push(currentInst);
            break;
        case IR_neg:
            if(sameBlock)
                currentNegInst.top() = currentInst;
            else
                currentNegInst.push(currentInst);
            break;
        default:
            break;
    }
}

void CSETracker:: revertToOuter(int blockNum)
{
    while(!currentAddInst.empty() && currentAddInst.top()->getBlkNo() == blockNum)
            currentAddInst.pop();
    while(!currentSubInst.empty() && currentSubInst.top()->getBlkNo() == blockNum)
        currentSubInst.pop();
    while(!currentMulInst.empty() && currentMulInst.top()->getBlkNo() == blockNum)
        currentMulInst.pop();
    while(!currentDivInst.empty() && currentDivInst.top()->getBlkNo() == blockNum)
        currentDivInst.pop();
    while(!currentAddaInst.empty() && currentAddaInst.top()->getBlkNo() == blockNum)
        currentAddaInst.pop();
    while(!currentNegInst.empty() && currentNegInst.top()->getBlkNo() == blockNum)
        currentAddInst.pop();
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