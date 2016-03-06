//
// Created by tmpark on 2/26/16.
//

#ifndef ADV_COMPILER_REGALLOCATION_H
#define ADV_COMPILER_REGALLOCATION_H

#include "Parser.h"

class Node{
public:
    Node(int arg0,  shared_ptr<IRFormat> arg2){nodeNum=arg0;nodeKind = instKind; inst = arg2;coloredReg = -1;};
    Node(int arg0,  string arg2, shared_ptr<Symbol> sym){nodeNum=arg0;nodeKind = varKind; var = arg2;coloredReg = -1;varSym = sym;};
    unordered_map<int,shared_ptr<Node>> neighbors;
    Kind getKind(){return nodeKind;};
    int getNodeNum(){return nodeNum;};
    int getCost(){return (nodeKind == instKind) ? inst->getCost() : varSym->getCost();};
    void setReg(int arg){ coloredReg = arg;
        if(arg < NUM_OF_DATA_REGS) {
            if (nodeKind == instKind)
                inst->setRegNo(arg + REG_DATA);
            else if (nodeKind == varKind)
                varSym->setReg(arg + REG_DATA);
        }
        else {
            if(nodeKind == instKind)
                inst->setRegNo(arg - NUM_OF_DATA_REGS + REG_VIRTUAL);
            else if(nodeKind == varKind)
                varSym->setReg(arg - NUM_OF_DATA_REGS + REG_VIRTUAL);
        }
    };
    int getReg(){return coloredReg;};
    int getAssignedReg() {
        if (coloredReg < NUM_OF_DATA_REGS)
            return coloredReg + REG_DATA;
        else
            return coloredReg - NUM_OF_DATA_REGS + REG_VIRTUAL;
    }
    string getVarNodeName(){return var;};
    shared_ptr<IRFormat> getInst(){return inst;};


private:
    int nodeNum;
    Kind nodeKind;//Only inst and var kind
    shared_ptr<IRFormat> inst;

    string var;
    shared_ptr<Symbol> varSym;
    int coloredReg;
};

class RegAllocation {

public:
    RegAllocation(string functionName,vector<shared_ptr<BasicBlock>> blocks, const int numOfVars);
    void doRegAllocation();
    void createInterferenceGraph(const string &graphFolder,const string &sourceFileName, string functionName);

private:
    void buildInterfGraph(shared_ptr<BasicBlock> currentBlock, unordered_map<int,shared_ptr<Node>> &liveSet);
    void Coloring();
    void RemovePhi(shared_ptr<BasicBlock> currentBlock);
    vector<shared_ptr<BasicBlock>> blocks;
    void makeEdge(shared_ptr<Node> node1, shared_ptr<Node> node2);
    void getLiveSet(unordered_map<int,shared_ptr<Node>> &liveSet,vector<shared_ptr<IRFormat>> codes);
    void getLiveSetForPhi(unordered_map<int,shared_ptr<Node>> &liveSet, vector<shared_ptr<IRFormat>> codes, int index);
    shared_ptr<IRFormat> getMoveCode(int blkNo, int firstReg, int secondReg, int firstConst);
    void insertMove(shared_ptr<BasicBlock> currentBlock, vector<array<int,3>> blockMoves);
    shared_ptr<IRFormat> getUncondBranchCode(int blkNo, int destBlkNo);

    int numOfVarNodes;
    vector<bool> regMapTemplate;
    unordered_map<int,shared_ptr<Node>> nodeList;
    int instNodeStart;
    string functionName;

};


#endif //ADV_COMPILER_REGALLOCATION_H
