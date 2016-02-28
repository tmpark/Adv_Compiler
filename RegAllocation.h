//
// Created by tmpark on 2/26/16.
//

#ifndef ADV_COMPILER_REGALLOCATION_H
#define ADV_COMPILER_REGALLOCATION_H

#include "Parser.h"


class RegAllocation {

public:
    RegAllocation(vector<BasicBlock> blocks){this->blocks = blocks;};
    void buildIntefGraph();
private:

    vector<BasicBlock> blocks;
    void makeEdge(shared_ptr<IRFormat> node1, shared_ptr<IRFormat> node2);
    void getLiveSet(unordered_map<int,shared_ptr<IRFormat>> &liveSet, vector<shared_ptr<IRFormat>> codes);

};


#endif //ADV_COMPILER_REGALLOCATION_H
