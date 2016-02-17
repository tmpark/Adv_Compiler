//
// Created by tmpark on 2/11/16.
//

#ifndef ADV_COMPILER_BASICBLOCK_H
#define ADV_COMPILER_BASICBLOCK_H

#include "Helper.h"

typedef enum{
    blk_normalAttr, blk_condAttr
} BlockAttribute;

typedef enum{
    blk_entry, blk_while_cond,blk_while_body,blk_while_end, blk_if_then, blk_if_else, blk_if_end
}BlockKind;


using namespace std;

class BasicBlock
{
public:
    BasicBlock(){blockNum = 0;
        blkAttr = blk_normalAttr;trueEdge = -1;blockKind = blk_entry;};
    BasicBlock(int blockNum){this->blockNum = blockNum;
        blkAttr = blk_normalAttr;trueEdge = -1;blockKind = blk_entry;};
    BasicBlock(int blockNum,BlockKind blockKind){this->blockNum = blockNum;this->blockKind = blockKind;
        blkAttr = blk_normalAttr;trueEdge = -1;};
    int getBlockNum(){return blockNum;};
    bool isTrueEdge(int edge){return edge == trueEdge;};
    bool isCondBlock(){return blkAttr == blk_condAttr;};
    void setTrueEdge(int edge){ blkAttr = blk_condAttr; trueEdge = edge;};
    string getBlockName(){if(blockKind == blk_entry)return "entry";
        else if(blockKind == blk_if_then)return "if.then";
        else if(blockKind == blk_if_else)return "if.else";
        else if(blockKind == blk_if_end)return "if.end";
        else if(blockKind == blk_while_cond)return "while.cond";
        else if(blockKind == blk_while_body)return "while.body";
        else if(blockKind == blk_while_end)return "while.end"; };
    void setBlockKind(BlockKind arg){blockKind = arg;};
    BlockKind getBlockKind(){return blockKind;};

    vector<shared_ptr<IRFormat>> irCodes;
    vector<shared_ptr<IRFormat>> phiCodes;
    vector<int> CFGForwardEdges;
    vector<int> DTForwardEdges;
private:
    int blockNum;
    BlockAttribute blkAttr;
    //string blockName;
    BlockKind blockKind;
    int trueEdge;
};


#endif //ADV_COMPILER_BASICBLOCK_H
