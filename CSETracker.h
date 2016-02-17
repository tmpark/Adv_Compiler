//
// Created by tmpark on 2/15/16.
//

#ifndef ADV_COMPILER_CSETRACKER_H
#define ADV_COMPILER_CSETRACKER_H

#include "Helper.h"

class CSETracker {

public:
    CSETracker();
    Result findExistingCommonSub(IROP irOp, vector<Result> operands);
    shared_ptr<IRFormat> getCurrentInstPtr(IROP irOp);
    void setCurrentInst(IROP irOp, shared_ptr<IRFormat> currentInst, bool sameBlock);
    void revertToOuter(IROP irOp);
private:
    stack<shared_ptr<IRFormat>> currentAddInst;
    stack<shared_ptr<IRFormat>> currentAddaInst;
    stack<shared_ptr<IRFormat>> currentSubInst;
    stack<shared_ptr<IRFormat>> currentMulInst;
    stack<shared_ptr<IRFormat>> currentDivInst;
};


#endif //ADV_COMPILER_CSETRACKER_H
