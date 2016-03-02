//
// Created by tmpark on 2/26/16.
//

#include "RegAllocation.h"

RegAllocation :: RegAllocation(string functionName,vector<shared_ptr<BasicBlock>> blocks, const int numOfVars)
{
    instNodeStart = numOfVars;
    this->blocks = blocks;
    numOfVarNodes = 0;
    this->functionName = functionName;
    for(int i = 0 ; i < MAX_NUMS_OF_REGS ; i++)
        regMapTemplate.push_back(false);
}

void RegAllocation::makeEdge(shared_ptr<Node> node1, shared_ptr<Node> node2)
{
    node1->neighbors.insert({node2->getNodeNum(),node2});
    node2->neighbors.insert({node1->getNodeNum(),node1});
}


void RegAllocation::doRegAllocation()
{
    unordered_map<int,shared_ptr<Node>> liveSet;
    buildInterfGraph(blocks.at(0),liveSet);
    Coloring();
}

void RegAllocation::Coloring()
{
    if(nodeList.empty())
        return;

    int lowestCost = 999999999;
    shared_ptr<Node> lowestCostNode = NULL;
    int IXOFLowestCostNode = -1;
    shared_ptr<Node> targetNode = NULL;
    int IXOFTargetNode = -1;
    bool NodeFound = false;
    for(auto nodeIter: nodeList)
    {
        shared_ptr<Node> node = nodeIter.second;
        int costOfCode = node->getCost();
        if(lowestCost > costOfCode) {
            lowestCost = costOfCode;
            lowestCostNode = node;
        }
        size_t numOfNeighbors = node->neighbors.size();
        if(numOfNeighbors < NUM_OF_DATA_REGS) {
            targetNode = node;
            NodeFound = true;
            break;
        }
    }
    //No inst fewer than num of regs -> node with lowest cost
    if(!NodeFound)
        targetNode = lowestCostNode;

    int targetNodeNum = targetNode->getNodeNum();

    //del edge from neighbor
    for(auto neighborIter :targetNode->neighbors) {
        shared_ptr<Node> neighbor = neighborIter.second;
        neighbor->neighbors.erase(targetNodeNum);
    }
    //del node itself
    nodeList.erase(targetNodeNum);

    Coloring();
    vector<bool> registerMap = regMapTemplate;
    //Insert Edge again
    for(auto neighborIter : targetNode->neighbors)
    {
        shared_ptr<Node> neighbor = neighborIter.second;
        neighbor->neighbors.insert({targetNodeNum,targetNode});
        int coloredReg = neighbor->getReg();
        if(coloredReg != -1)
            registerMap.at((unsigned long)coloredReg) = true;
    }
    nodeList.insert({targetNodeNum,targetNode});
    //Chose color that is not in neighbor
    int index = 0;
    for(auto occupied : registerMap)
    {
        if(!occupied)
        {
            targetNode->setReg(index);
            return;
        }
        index++;
    }
}



void RegAllocation:: buildInterfGraph(shared_ptr<BasicBlock> currentBlock, unordered_map<int,shared_ptr<Node>> &liveSet)
{
    auto rit = currentBlock->DTForwardEdges.rbegin();
    vector<unordered_map<int,shared_ptr<Node>>> tempLiveSetList;
    vector<shared_ptr<IRFormat>> phiCodes;

    //deal with codes with child blocks
    for(; rit!= currentBlock->DTForwardEdges.rend(); ++rit)
    {
        shared_ptr<BasicBlock> targetBlock = *rit;
        unordered_map<int,shared_ptr<Node>> tempLiveSet = liveSet;
        BlockKind blkKind = targetBlock->getBlockKind();

        if(blkKind == blk_while_cond)
        {
            buildInterfGraph(targetBlock,liveSet);
            getLiveSet(liveSet,currentBlock->irCodes);
        }
        else if(blkKind == blk_while_end)
        {
            buildInterfGraph(targetBlock,tempLiveSet);
            tempLiveSetList.push_back(tempLiveSet);//Live1
            getLiveSet(tempLiveSet,currentBlock->irCodes);//Live2
            getLiveSetForPhi(tempLiveSet,currentBlock->phiCodes,2);
            liveSet = tempLiveSet;
        }
        else if(blkKind == blk_while_body)
        {
            buildInterfGraph(targetBlock,tempLiveSet);//Live3
            for(auto eachSet :tempLiveSetList)
                tempLiveSet.insert(eachSet.begin(),eachSet.end());//Live3 + Live1
            getLiveSet(tempLiveSet,currentBlock->irCodes);//Live2'
            getLiveSetForPhi(tempLiveSet,currentBlock->phiCodes,1);
            liveSet = tempLiveSet;
        }
        else if(blkKind == blk_if_end){
            buildInterfGraph(targetBlock,liveSet);//live1
            phiCodes = targetBlock->phiCodes;
        }
        else if(blkKind == blk_if_else)
        {
            getLiveSetForPhi(tempLiveSet,phiCodes,2);
            buildInterfGraph(targetBlock,tempLiveSet);//live3
            tempLiveSetList.push_back(tempLiveSet);
        }
        else if(blkKind == blk_if_then)
        {
            getLiveSetForPhi(tempLiveSet,phiCodes,1);
            buildInterfGraph(targetBlock,tempLiveSet);//live2
            tempLiveSetList.push_back(tempLiveSet);
            if(!tempLiveSetList.empty())
                liveSet.clear();
            for(auto eachSet :tempLiveSetList)
                liveSet.insert(eachSet.begin(),eachSet.end());
            getLiveSet(liveSet,currentBlock->irCodes);
        }
    }

    //Just deal with own codes
    if(currentBlock->DTForwardEdges.empty())
        getLiveSet(liveSet, currentBlock->irCodes);

    return;
}

void RegAllocation::getLiveSet(unordered_map<int,shared_ptr<Node>> &liveSet, vector<shared_ptr<IRFormat>> codes)
{
    Parser *parser = Parser::instance();
    auto rit = codes.rbegin();
    for(; rit != codes.rend(); ++rit)
    {
        shared_ptr<IRFormat> code = *rit;
        int nodeNum = instNodeStart+code->getLineNo();
        auto nodeIter = nodeList.find(nodeNum);
        shared_ptr<Node> node;

        if(isDefInstr(code->getIROP())) {
            if(nodeIter == nodeList.end())
            {
                node = std::make_shared<Node>(nodeNum, code);
                nodeList.insert({nodeNum, node});
            }
            else
                node = nodeIter->second;

            liveSet.erase(nodeNum);

            for (auto liveEntry : liveSet)
                makeEdge(node, liveEntry.second);
        }
        for(auto operand : code->operands)
        {
            Kind operandKind = operand.getKind();
            if(operandKind == instKind) {
                shared_ptr<IRFormat> inst = operand.getInst();
                nodeNum = instNodeStart + inst->getLineNo();
                nodeIter = nodeList.find(nodeNum);
                if(nodeIter == nodeList.end())
                {
                    node = std::make_shared<Node>(nodeNum,  inst);
                    nodeList.insert({nodeNum, node});
                }
                else
                    node = nodeIter->second;
                liveSet.insert({nodeNum, node});
            }
            else if(operandKind == varKind)
            {
                string varName = operand.getVariableName();
                //If there is no node for that symbol
                shared_ptr<Node> candidateNode = NULL;
                for(int i = 0 ; i < numOfVarNodes ; i++)
                {
                    auto varNodeIter = nodeList.find(i);
                    if(nodeList.end() != varNodeIter)
                    {
                        candidateNode = varNodeIter->second;
                        if(varName == candidateNode->getVarNodeName())
                            break;
                        candidateNode = NULL;
                    }
                }
                if(candidateNode == NULL)
                {
                    shared_ptr<Symbol> sym = parser->symTableLookup(functionName,varName,sym_var);
                    node = std::make_shared<Node>(numOfVarNodes, varName,sym);
                    nodeList.insert({numOfVarNodes, node});
                    numOfVarNodes++;
                }
                else
                    node = candidateNode;
                liveSet.insert({node->getNodeNum(),node});
            }
        }
    }
}

void RegAllocation::getLiveSetForPhi(unordered_map<int,shared_ptr<Node>> &liveSet, vector<shared_ptr<IRFormat>> codes, int index)
{
    Parser *parser = Parser::instance();
    auto rit = codes.rbegin();
    for(; rit != codes.rend(); ++rit)
    {
        shared_ptr<IRFormat> code = *rit;

        int nodeNum = instNodeStart+code->getLineNo();
        auto nodeIter = nodeList.find(nodeNum);
        shared_ptr<Node> node;
        if(nodeIter == nodeList.end())
        {
            node = std::make_shared<Node>(nodeNum, code);
            nodeList.insert({nodeNum, node});
        }
        else
            node = nodeIter->second;

        liveSet.erase(nodeNum);
        if(isDefInstr(code->getIROP())){
            for(auto liveEntry : liveSet)
                makeEdge(node,liveEntry.second);
        }
        Result operand = code->operands.at((unsigned long)index);
        Kind operandKind = operand.getKind();
        if(operandKind == instKind) {
            shared_ptr<IRFormat> inst = operand.getInst();
            nodeNum = instNodeStart+ inst->getLineNo();
            nodeIter = nodeList.find(nodeNum);
            if(nodeIter == nodeList.end())
            {
                node = std::make_shared<Node>(nodeNum,  inst);
                nodeList.insert({nodeNum, node});
            }
            else
                node = nodeIter->second;
            liveSet.insert({nodeNum, node});
        }
        else if(operandKind == varKind)
        {
            string varName = operand.getVariableName();
            //If there is no node for that symbol
            shared_ptr<Node> candidateNode = NULL;
            for(int i = 0 ; i < numOfVarNodes ; i++)
            {
                auto varNodeIter = nodeList.find(i);
                if(nodeList.end() != varNodeIter)
                {
                    candidateNode = varNodeIter->second;
                    if(varName == candidateNode->getVarNodeName())
                        break;
                    candidateNode = NULL;
                }
            }
            if(candidateNode == NULL)
            {
                shared_ptr<Symbol> sym = parser->symTableLookup(functionName,varName,sym_var);
                node = std::make_shared<Node>(numOfVarNodes, varName,sym);
                nodeList.insert({numOfVarNodes, node});
                numOfVarNodes++;
            }
            else
                node = candidateNode;
            liveSet.insert({node->getNodeNum(),node});
        }
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
    for(auto nodeIter : nodeList)
    {
        shared_ptr<Node> node = nodeIter.second;
        int cost = node->getCost();
        int nodeNum = node->getNodeNum();
        string costString = "Cost of " + to_string(cost);
        graphDrawer->writeNodeStart(nodeNum,costString);
        //string codeString = parser->getCodeString(node);
        Kind nodeKind = node->getKind();
        if(nodeKind == instKind)
        {
            shared_ptr<IRFormat> inst = node->getInst();
            string codeString = "R" + to_string(node->getAssignedReg()) + "::" +parser->getCodeString(inst);
            graphDrawer->writeCode(codeString);
        }
        else if(nodeKind == varKind)
        {
            string var = node->getVarNodeName();
            string codeString = "R" + to_string(node->getAssignedReg()) + "::" +var;
            graphDrawer->writeCode(codeString);
        }

        graphDrawer->writeNodeEnd();
        for (auto dest : node->neighbors) {
            EDGETYPE edgeType = edge_normal;
            if(nodeNum > dest.first)//Just print out one of them
                graphDrawer->writeEdge(nodeNum, dest.first, edgeType,graph_IG);
        }
    }
    graphDrawer->writeEnd();
    graphDrawer->closeFile();
}