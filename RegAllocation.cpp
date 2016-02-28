//
// Created by tmpark on 2/26/16.
//

#include "RegAllocation.h"


void RegAllocation::makeEdge(shared_ptr<IRFormat> node1, shared_ptr<IRFormat> node2)
{
    node1->neighbors.insert({node2->getLineNo(),node2});
    node2->neighbors.insert({node1->getLineNo(),node1});
}

void RegAllocation::buildIntefGraph()
{
    auto rit = blocks.rbegin();

    for(; rit!= blocks.rend(); ++rit)
    {

        BasicBlock targetBlock = *rit;
        unordered_map<int,shared_ptr<IRFormat>> liveSet;
        getLiveSet(liveSet,targetBlock.irCodes);
        //getLiveSet(liveSet,targetBlock.phiCodes);
    }

}

void RegAllocation::getLiveSet(unordered_map<int,shared_ptr<IRFormat>> &liveSet, vector<shared_ptr<IRFormat>> codes)
{
    auto rit = codes.rbegin();
    for(; rit != codes.rend(); ++rit)
    {
        shared_ptr<IRFormat> code = *rit;
        if(liveSet.erase(code->getLineNo()) != 0)
        {
            for(auto liveEntry : liveSet)
                makeEdge(code,liveEntry.second);
        }
        for(auto operand : code->operands)
        {
            if(operand.getKind() == instKind)
                liveSet.insert({operand.getInstNum(),operand.getInst()});
        }
    }
}