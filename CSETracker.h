//
// Created by tmpark on 2/15/16.
//

#ifndef ADV_COMPILER_CSETRACKER_H
#define ADV_COMPILER_CSETRACKER_H

#include "Helper.h"

class CSETracker {

public:
    CSETracker();
    shared_ptr<IRFormat> findExistingCommonSub(shared_ptr<IRFormat> targetInst);
    shared_ptr<IRFormat> getCurrentInstPtr(IROP irOp);
    void setCurrentInst(IROP irOp, shared_ptr<IRFormat> currentInst);
    void revertToOuter(int blockNum, bool endOfInnerBlock);

    //vector<shared_ptr<IRFormat>> candidateLoadInstructions;
    vector<shared_ptr<IRFormat>> candidateCSEInstructions;
    vector<shared_ptr<IRFormat>> killingStores;
private:
    shared_ptr<IRFormat> currentAddInst;
    shared_ptr<IRFormat> currentAddaInst;
    shared_ptr<IRFormat> currentSubInst;
    shared_ptr<IRFormat> currentMulInst;
    shared_ptr<IRFormat> currentDivInst;
    shared_ptr<IRFormat> currentNegInst;
    shared_ptr<IRFormat> currentCmpInst;
    shared_ptr<IRFormat> currentLoadInst;


};


#endif //ADV_COMPILER_CSETRACKER_H
