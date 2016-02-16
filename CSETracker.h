//
// Created by tmpark on 2/15/16.
//

#ifndef ADV_COMPILER_CSETRACKER_H
#define ADV_COMPILER_CSETRACKER_H

#include "Helper.h"

class CSETracker {

public:
    IRFormat*getCurrentAddInst(){return currentAddInst;};
    void setCurrentAddInst(IRFormat *arg){ currentAddInst = arg;};
    IRFormat*getCurrentSubInst(){return currentSubInst;};
    void setCurrentSubInst(IRFormat *arg){ currentSubInst = arg;};
    IRFormat*getCurrentMulInst(){return currentMulInst;};
    void setCurrentMulInst(IRFormat *arg){ currentMulInst = arg;};
    IRFormat*getCurrentDivInst(){return currentDivInst;};
    void setCurrentDivInst(IRFormat *arg){ currentDivInst = arg;};
    Result findExistingCommonSub(IROP irOp, Result x,Result y);
    IRFormat*getCurrentInstPtr(IROP irOp);
    void setCurrentInst(IROP irOp, IRFormat* currentInst);

private:
    IRFormat *currentAddInst;
    IRFormat *currentSubInst;
    IRFormat *currentMulInst;
    IRFormat *currentDivInst;
};


#endif //ADV_COMPILER_CSETRACKER_H
