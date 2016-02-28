//
// Created by tmpark on 2/26/16.
//

#include "RegAllocation.h"


void RegAllocation::makeEdge(shared_ptr<IRFormat> node1, shared_ptr<IRFormat> node2)
{
    node1->neighbors.insert({node2->getLineNo(),node2});
    node2->neighbors.insert({node1->getLineNo(),node1});
}

void RegAllocation::doRegAllocation()
{
    unordered_map<int,shared_ptr<IRFormat>> liveSet;
    buildInterfGraph(blocks.at(0),liveSet);
}

void RegAllocation::buildInterfGraph(shared_ptr<BasicBlock> currentBlock, unordered_map<int,shared_ptr<IRFormat>> &liveSet)
{
    auto rit = currentBlock->DTForwardEdges.rbegin();
    vector<unordered_map<int,shared_ptr<IRFormat>>> tempLiveSetList;
    vector<shared_ptr<IRFormat>> phiCodes;
    for(; rit!= currentBlock->DTForwardEdges.rend(); ++rit)
    {
        shared_ptr<BasicBlock> targetBlock = *rit;
        unordered_map<int,shared_ptr<IRFormat>> tempLiveSet = liveSet;
        BlockKind blkKind = targetBlock->getBlockKind();

        if(blkKind == blk_if_end) {
            buildInterfGraph(targetBlock,liveSet);
            phiCodes = targetBlock->phiCodes;
        }
        else if(blkKind == blk_if_else)
        {
            getLiveSetForPhi(tempLiveSet,phiCodes,2);
            buildInterfGraph(targetBlock,tempLiveSet);
            tempLiveSetList.push_back(tempLiveSet);
        }
        else if(blkKind == blk_if_then)
        {
            getLiveSetForPhi(tempLiveSet,phiCodes,1);
            buildInterfGraph(targetBlock,tempLiveSet);
            tempLiveSetList.push_back(tempLiveSet);
        }
    }
    if(!tempLiveSetList.empty())
        liveSet.clear();
    for(auto eachSet :tempLiveSetList)
        liveSet.insert(eachSet.begin(),eachSet.end());
    getLiveSet(liveSet,currentBlock->irCodes);
    return;
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

void RegAllocation::getLiveSetForPhi(unordered_map<int,shared_ptr<IRFormat>> &liveSet, vector<shared_ptr<IRFormat>> codes, int index)
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
        Result operand = code->operands.at((unsigned long)index);
        if(operand.getKind() == instKind)
            liveSet.insert({operand.getInstNum(),operand.getInst()});
    }
}

void RegAllocation::createInterferenceGraph(const string &graphFolder,const string &sourceFileName, string functionName)
{
    Parser *parser = Parser::instance();
    string formatName = ".dot";
    RC rc = -1;

    string fileName = graphFolder + "IG_"+ sourceFileName+ "_" + functionName + formatName;

    GraphDrawer *graphDrawer = GraphDrawer::instance();
    rc = graphDrawer->createFile(fileName);
    if (rc == -1)
        return;
    rc = graphDrawer->openFile(fileName);
    if (rc == -1) {
        graphDrawer->destroyFile(fileName);
        return;
    }

    graphDrawer->writePreliminary(graph_IG,functionName);
    vector<shared_ptr<BasicBlock>> basicBlockList = blocks;
    for (auto blockPair : basicBlockList) {
        for(auto code : blockPair->irCodes)
        {

            graphDrawer->writeNodeStart(code->getLineNo(), "");
            string codeString = parser->getCodeString(code);
            graphDrawer->writeCode(codeString);
            graphDrawer->writeNodeEnd();
            for (auto dest : code->neighbors) {
                EDGETYPE edgeType = edge_normal;
                graphDrawer->writeEdge(code->getLineNo(), dest.first, edgeType);
            }
        }
        for(auto code : blockPair->phiCodes)
        {
            graphDrawer->writeNodeStart(code->getLineNo(), "");
            string codeString = parser->getCodeString(code);
            graphDrawer->writeCode(codeString);
            graphDrawer->writeNodeEnd();
            for (auto dest : code->neighbors) {
                EDGETYPE edgeType = edge_normal;
                graphDrawer->writeEdge(code->getLineNo(), dest.first, edgeType);
            }
        }
    }
    graphDrawer->writeEnd();
    graphDrawer->closeFile();
}