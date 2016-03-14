//
// Created by tmpark on 2/26/16.
//

#include "RegAllocation.h"

RegAllocation :: RegAllocation(string functionName,vector<shared_ptr<BasicBlock>> blocks, const int numOfVars)
{

    instNodeStart = numOfVars;
    this->blocks = blocks;
    numOfVarNodes = 0;
    numOfVReg = 0;
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

    unordered_map<int,shared_ptr<Node>> liveSetIter = liveSet;

    for(auto liveEntry : liveSetIter)
    {
        int targetEntryNum = liveEntry.first;
        //cout << targetEntryNum << endl;
        shared_ptr<Node> targetEntry = liveEntry.second;

        liveSet.erase(targetEntryNum);

        for (auto otherEntry : liveSet)
            makeEdge(targetEntry, otherEntry.second);
    }



    Coloring();
    Parser *parser = Parser::instance();
    parser->setNumOfVirtualRegs(functionName,numOfVReg);

    RemovePhi(blocks.at(0));
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

    }

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
            Parser *parser = Parser::instance();
            if(index >= NUM_OF_DATA_REGS && numOfVReg < index-NUM_OF_DATA_REGS + 1)
            {
                numOfVReg = index-NUM_OF_DATA_REGS + 1;
                //index - NUM_OF_DATA_REGS + 1
            }
            targetNode->setReg(functionName,index);
            parser->insertRegUsed(functionName,index);

            return;
        }
        index++;
    }
}

void RegAllocation::RemovePhi(shared_ptr<BasicBlock> currentBlock)
{
    Parser *parser = Parser::instance();
    for(auto childBlockIter : currentBlock->DTForwardEdges)
    {
        BlockKind blkKind = childBlockIter->getBlockKind();
        if(blkKind == blk_while_cond) {
            for (auto phiCode : childBlockIter->phiCodes) {
                if(phiCode->isElimiated())
                    continue;
                //first operand is comming from the parent
                Result operand = phiCode->operands.at(1);
                Kind operandKind = operand.getKind();
                int regNoOfPhi = phiCode->getRegNo();

                if (operandKind == instKind) {
                    shared_ptr<IRFormat> operandCode = operand.getInst();
                    int operandCodeReg = operandCode->getRegNo();
                    if (operandCodeReg != regNoOfPhi) {
                        int definedBlockNum = operandCode->getBlkNo();
                        shared_ptr<IRFormat> ir_line = getMoveCode(currentBlock->getBlockNum(), operandCodeReg,
                                                                   regNoOfPhi, 0);
                        currentBlock->irCodes.push_back(ir_line);
                    }
                }
                else if (operandKind == varKind) {
                    shared_ptr<Symbol> operandSymbol = operand.getVarSym();
                    int operandSymReg = operandSymbol->getRegNo(functionName);
                    if (operandSymReg != regNoOfPhi) {
                        shared_ptr<IRFormat> ir_line = getMoveCode(currentBlock->getBlockNum(), operandSymReg,
                                                                   regNoOfPhi, 0);
                        currentBlock->irCodes.push_back(ir_line);
                    }
                }
                else if (operandKind == constKind) {
                    shared_ptr<IRFormat> ir_line = getMoveCode(currentBlock->getBlockNum(), operand.getConst(),
                                                               regNoOfPhi, 1);
                    currentBlock->irCodes.push_back(ir_line);
                }

            }
        }
        else if(blkKind == blk_while_body)
        {
            vector<array<int,3>> bodyBlockMoves;

            for (auto phiCode : currentBlock->phiCodes) {
                if(phiCode->isElimiated())
                    continue;
                //first operand is comming from the parent
                Result operand = phiCode->operands.at(2);
                Kind operandKind = operand.getKind();
                int regNoOfPhi = phiCode->getRegNo();

                if (operandKind == instKind) {
                    shared_ptr<IRFormat> operandCode = operand.getInst();
                    int operandCodeReg = operandCode->getRegNo();
                    if (operandCodeReg != regNoOfPhi) {
                        array<int,3> moveTuple = {operandCodeReg,regNoOfPhi,0};
                        bodyBlockMoves.push_back(moveTuple);
                    }
                }
                else if (operandKind == varKind) {
                    shared_ptr<Symbol> operandSymbol = operand.getVarSym();
                    int operandSymReg = operandSymbol->getRegNo(functionName);
                    if (operandSymReg != regNoOfPhi) {
                        array<int,3> moveTuple = {operandSymReg,regNoOfPhi,0};
                        bodyBlockMoves.push_back(moveTuple);
                    }
                }
                else if (operandKind == constKind) {
                    array<int,3> moveTuple = {operand.getConst(),regNoOfPhi,1};
                    bodyBlockMoves.push_back(moveTuple);
                }
            }
            if(!bodyBlockMoves.empty())
                insertMove(currentBlock->DTForwardEdges.at(0),bodyBlockMoves);

        }
        else if(blkKind == blk_if_end)
        {
            vector<array<int,3>> thenBlockMoves;
            vector<array<int,3>> elseBlockMoves;
            //the last value is to check whether the first number is const or regNum
            vector<array<int,3>> splitBlockMoves;

            //For every phi code, check whether allocated register is the same
            for(auto phiCode : childBlockIter->phiCodes)
            {
                if(phiCode->isElimiated())
                    continue;
                int regNoOfPhi = phiCode->getRegNo();
                //for operand 1, 2
                for(int index=1 ; index < 3 ; index++)
                {
                    Result operand = phiCode->operands.at(index);
                    Kind operandKind = operand.getKind();
                    if(operandKind == instKind)
                    {
                        shared_ptr<IRFormat> operandCode = operand.getInst();
                        int operandCodeReg = operandCode->getRegNo();
                        if(operandCodeReg != regNoOfPhi)
                        {
                            //if there is no else block for dealing with second operand(children is just two(if.then and if.end)
                            if(currentBlock->DTForwardEdges.size() < 3 && index == 2)
                            {
                                array<int,3> moveTuple = {operandCodeReg,regNoOfPhi,0};
                                splitBlockMoves.push_back(moveTuple);
                            }
                            else
                            {
                                //Insert Move at the end of then or else block(First collect and do the job for all)
                                array<int,3> moveTuple = {operandCodeReg,regNoOfPhi,0};
                                if(index == 1)
                                    thenBlockMoves.push_back(moveTuple);
                                else if(index == 2)
                                    elseBlockMoves.push_back(moveTuple);
                            }
                        }
                    }
                    else if(operandKind == varKind)
                    {
                        shared_ptr<Symbol> operandSymbol = operand.getVarSym();
                        int operandSymReg = operandSymbol->getRegNo(functionName);
                        if(operandSymReg != regNoOfPhi)
                        {
                            //if there is no else block for dealing with second operand(children is just two(if.then and if.end)
                            if(currentBlock->DTForwardEdges.size() < 3 && index == 2)
                            {
                                array<int,3> moveTuple = {operandSymReg,regNoOfPhi,0};
                                splitBlockMoves.push_back(moveTuple);
                            }
                            else
                            {
                                //Insert Move at the end of then or else block(First collect and do the job for all)
                                array<int,3> moveTuple = {operandSymReg,regNoOfPhi,0};
                                if(index == 1)
                                    thenBlockMoves.push_back(moveTuple);
                                else if(index == 2)
                                    elseBlockMoves.push_back(moveTuple);
                            }
                        }
                    }
                    else if(operandKind == constKind)
                    {
                        //if there is no else block for dealing with second operand(children is just two(if.then and if.end)
                        if(currentBlock->DTForwardEdges.size() < 3 && index == 2)
                        {
                            array<int,3> moveTuple = {operand.getConst(),regNoOfPhi,1};
                            splitBlockMoves.push_back(moveTuple);
                        }
                        else
                        {
                            //Insert Move at the end of then or else block(First collect and do the job for all)
                            array<int,3> moveTuple = {operand.getConst(),regNoOfPhi,1};
                            if(index == 1)
                                thenBlockMoves.push_back(moveTuple);
                            else if(index == 2)
                                elseBlockMoves.push_back(moveTuple);
                        }
                    }
                }

            }

            if(!thenBlockMoves.empty())
                insertMove(currentBlock->DTForwardEdges.at(0),thenBlockMoves);
            if(!elseBlockMoves.empty())
                insertMove(currentBlock->DTForwardEdges.at(1),elseBlockMoves);
            if(!splitBlockMoves.empty())
            {
                shared_ptr<BasicBlock> splitBlock = currentBlock->DTForwardEdges.at(1);
                int splitBlockNum = splitBlock->getBlockNum();
                for(auto splitBlockMove : splitBlockMoves)
                {
                    //Insert Move at the end of currentBlock
                    shared_ptr<IRFormat> ir_line = getMoveCode(splitBlockNum,splitBlockMove.at(0),splitBlockMove.at(1),splitBlockMove.at(2));
                    splitBlock->edgeCodes.push_back(ir_line);
                }
                insertBranch(currentBlock->DTForwardEdges.at(0),splitBlockNum,splitBlock->edgeCodes.size());
                /*
                shared_ptr<BasicBlock> splitBlock(new BasicBlock(parser->newBasicBlock(),blk_move));
                int splitBlockNum = splitBlock->getBlockNum();
                //Branch Operand;
                Result splitBlockOperand;splitBlockOperand.setBlock(splitBlock->getBlockNum());
                Result existingBlockOperand = currentBlock->irCodes.back()->operands.at(1);
                currentBlock->irCodes.back()->operands.at(1) = splitBlockOperand;
                shared_ptr<IRFormat> ir_line = NULL;
                for(auto splitBlockMove : splitBlockMoves)
                {
                    //Insert Move at the end of currentBlock
                    ir_line = getMoveCode(splitBlockNum,splitBlockMove.at(0),splitBlockMove.at(1),splitBlockMove.at(2));
                    splitBlock->irCodes.push_back(ir_line);
                }
                ir_line = getUncondBranchCode(splitBlockNum,existingBlockOperand.getBlockNo());
                splitBlock->irCodes.push_back(ir_line);

                //Fixup CFG
                shared_ptr<BasicBlock> endBlock = currentBlock->CFGForwardEdges.back();
                currentBlock->CFGForwardEdges.pop_back();
                currentBlock->CFGForwardEdges.push_back(splitBlock);
                splitBlock->CFGForwardEdges.push_back(endBlock);

                //Fixup DT
                endBlock = currentBlock->DTForwardEdges.back();
                currentBlock->DTForwardEdges.pop_back();
                currentBlock->DTForwardEdges.push_back(splitBlock);
                splitBlock->DTForwardEdges.push_back(endBlock);
                parser->updateBlockInfo(functionName,splitBlock);
                */
            }
        }
        RemovePhi(childBlockIter);
    }

}

void RegAllocation::insertBranch(shared_ptr<BasicBlock> currentBlock,int blockNum, int numOfEdgeCodes)
{
    //Go down to the end of the block;
    while(!currentBlock->DTForwardEdges.empty())
        currentBlock = currentBlock->DTForwardEdges.back();
    if(!currentBlock->irCodes.empty())
    {
        shared_ptr<IRFormat> lastCode = currentBlock->irCodes.back();
        if(lastCode->getIROP() == IR_bra && lastCode->operands.at(0).getKind() == blockKind)
        {
            lastCode->operands.at(0).setBlock(blockNum);
            lastCode->operands.at(0).setJumpLoc(numOfEdgeCodes);
        }
        else
        {
            Parser *parser = Parser::instance();
            shared_ptr<IRFormat> ir_line(new IRFormat);
            ir_line->setBlkNo(currentBlock->getBlockNum());
            ir_line->setLineNo(parser->newInstruction());
            ir_line->setIROP(IR_bra);
            Result jumpBra;jumpBra.setBlock(blockNum);jumpBra.setJumpLoc(numOfEdgeCodes);
            ir_line->operands.push_back(jumpBra);
            currentBlock->irCodes.push_back(ir_line);
        }
    }

}

void RegAllocation::insertMove(shared_ptr<BasicBlock> currentBlock, vector<array<int,3>> blockMoves)
{
    //Go down to the end of the block;
    while(!currentBlock->DTForwardEdges.empty())
        currentBlock = currentBlock->DTForwardEdges.back();

    //Insert move before unconditional branch
    shared_ptr<IRFormat> branchInstr = NULL;
    if(!currentBlock->irCodes.empty())
    {
        branchInstr = currentBlock->irCodes.back();
        if(branchInstr->getIROP() == IR_bra)
            currentBlock->irCodes.pop_back();
    }

    for(auto blockMove : blockMoves)
    {
        shared_ptr<IRFormat> ir_line = getMoveCode(currentBlock->getBlockNum(),blockMove.at(0),blockMove.at(1),blockMove.at(2));
        currentBlock->irCodes.push_back(ir_line);
    }
    if(branchInstr != NULL && branchInstr->getIROP() == IR_bra)
        currentBlock->irCodes.push_back(branchInstr);
}

shared_ptr<IRFormat> RegAllocation:: getMoveCode(int blkNo, int firstReg, int secondReg, int firstConst)
{
    Parser *parser = Parser::instance();
    shared_ptr<IRFormat> ir_line(new IRFormat);
    ir_line->setBlkNo(blkNo);
    ir_line->setLineNo(parser->newInstruction());
    ir_line->setIROP(IR_move);

    Result regOp1;
    (firstConst == 1) ? regOp1.setConst(firstReg) : regOp1.setReg(firstReg);

    ir_line->operands.push_back(regOp1);
    Result regOp2;regOp2.setReg(secondReg);
    ir_line->operands.push_back(regOp2);
    return ir_line;
}

shared_ptr<IRFormat> RegAllocation::getUncondBranchCode(int blkNo, int destBlkNo)
{
    Parser *parser = Parser::instance();
    shared_ptr<IRFormat> ir_line(new IRFormat);
    ir_line->setBlkNo(blkNo);
    ir_line->setLineNo(parser->newInstruction());
    ir_line->setIROP(IR_bra);

    Result regOp;regOp.setBlock(destBlkNo);
    ir_line->operands.push_back(regOp);
    return ir_line;
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
            //getLiveSet(tempLiveSet,currentBlock->irCodes);//Live2
            //getLiveSetForPhi(tempLiveSet,currentBlock->phiCodes,2,false);
            liveSet = tempLiveSet;
        }
        else if(blkKind == blk_while_body)
        {
            //Do it two times
            for(int i = 0 ; i < 2 ; i++)
            {
                getLiveSet(tempLiveSet,currentBlock->irCodes);//Live2
                getLiveSetForPhi(tempLiveSet,currentBlock->phiCodes,2,false);//Live2 + x1
                buildInterfGraph(targetBlock,tempLiveSet);//Live3
                for(auto eachSet :tempLiveSetList)
                    tempLiveSet.insert(eachSet.begin(),eachSet.end());//Live3 + Live1
                getLiveSet(tempLiveSet,currentBlock->irCodes);//Live2'
            }


            getLiveSetForPhi(tempLiveSet,currentBlock->phiCodes,1,true); //Live2' - i + x0
            liveSet = tempLiveSet;
        }
        else if(blkKind == blk_if_end){
            buildInterfGraph(targetBlock,liveSet);//live1
            phiCodes = targetBlock->phiCodes;
        }
        else if(blkKind == blk_if_else)
        {
            getLiveSetForPhi(tempLiveSet,phiCodes,2,true);
            buildInterfGraph(targetBlock,tempLiveSet);//live3
            tempLiveSetList.push_back(tempLiveSet);
        }
        else if(blkKind == blk_if_then)
        {
            //If there is no else: just take phi
            if(currentBlock->DTForwardEdges.size() ==2)
            {
                getLiveSetForPhi(tempLiveSet,phiCodes,2,true);
                tempLiveSetList.push_back(tempLiveSet);
            }

            getLiveSetForPhi(tempLiveSet,phiCodes,1,true);
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

        if(isDefInstr(code)) {
            if(nodeIter == nodeList.end())
            {
                //If there were usage of that code, there should be node -> dead Code elimination
                //code->eliminate();
                //continue;
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
                    shared_ptr<Symbol> sym = operand.getVarSym();//parser->symTableLookup(functionName,varName,sym_var);
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

void RegAllocation::getLiveSetForPhi(unordered_map<int,shared_ptr<Node>> &liveSet, vector<shared_ptr<IRFormat>> codes, int index, bool phiDefRemove)
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
            //If there were usage of that code, there should be node -> dead Code elimination
            //code->eliminate();
            //continue;
            node = std::make_shared<Node>(nodeNum, code);
            nodeList.insert({nodeNum, node});
        }
        else
            node = nodeIter->second;

        if(phiDefRemove)
           liveSet.erase(nodeNum);

        if(isDefInstr(code)){
            for(auto liveEntryIter : liveSet) {

                int liveEntryNum = liveEntryIter.first;
                shared_ptr<Node> liveEntry = liveEntryIter.second;
                int foundNodeNum = node->getNodeNum();
                if(foundNodeNum != liveEntryNum)
                    makeEdge(node, liveEntry);
            }
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
                shared_ptr<Symbol> sym = operand.getVarSym();//parser->symTableLookup(functionName,varName,sym_var);
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
            string codeString = "R" + to_string(node->getAssignedReg()) + "::" +parser->getCodeString(functionName, inst);
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