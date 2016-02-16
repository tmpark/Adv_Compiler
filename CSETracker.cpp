//
// Created by tmpark on 2/15/16.
//

#include "CSETracker.h"


Result CSETracker::findExistingCommonSub(IROP irOp,Result x,Result y)
{
    IRFormat *currentInstPtr = NULL;
    currentInstPtr = getCurrentInstPtr(irOp);

    Result cseReturn;

    while(currentInstPtr != NULL)
    {
        IRFormat currentInst = *currentInstPtr;
        Result operand0 = currentInst.operands.at(0);
        Result operand1 = currentInst.operands.at(1);
        if(isSameOperand(operand0,x) && isSameOperand(operand1,y) ||
           isSameOperand(operand0,y) && isSameOperand(operand1,x) && irOp != IR_div)//vice versa except div
        {
            //Same operand
            cseReturn.setInst(currentInst.getLineNo());
            return cseReturn;
        }
        currentInstPtr = currentInst.getPreviousSameOpInst();
    }

    return cseReturn;
}


IRFormat* CSETracker::getCurrentInstPtr(IROP irOp)
{
    switch(irOp)
    {
        case IR_add:
            return currentAddInst;
        case IR_sub:
            return currentSubInst;
        case IR_mul:
            return currentMulInst;
        case IR_div:
            return currentDivInst;
        default:
            return NULL;
    }
}
void CSETracker::setCurrentInst(IROP irOp, IRFormat* currentInst)
{
    switch(irOp)
    {
        case IR_add:
            currentAddInst = currentInst;
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
        default:
            break;
    }
}