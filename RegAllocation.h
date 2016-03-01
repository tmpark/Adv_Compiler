//
// Created by tmpark on 2/26/16.
//

#ifndef ADV_COMPILER_REGALLOCATION_H
#define ADV_COMPILER_REGALLOCATION_H

#include "Parser.h"

class Node{
public:
    Node(int arg0, Kind arg1, shared_ptr<IRFormat> arg2){nodeNum=arg0;nodeKind = instKind; inst = arg2;varCost = 0;coloredReg = -1;};
    Node(int arg0, Kind arg1, string arg2){nodeNum=arg0;nodeKind = varKind; var = arg2;varCost = 0;coloredReg = -1;};
    unordered_map<int,shared_ptr<Node>> neighbors;
    Kind getKind(){return nodeKind;};
    int getNodeNum(){return nodeNum;};
    int getCost(){return (nodeKind == instKind) ? inst->getCost() : varCost;};
    void setColor(int arg){coloredReg = arg;
        if(arg < NUM_OF_DATA_REGS)
            inst->setRegNo(arg + REG_DATA);
        else
            inst->setRegNo(arg - NUM_OF_DATA_REGS + REG_VIRTUAL);
    };
    int getColor(){return coloredReg;};


private:
    int nodeNum;
    Kind nodeKind;//Only inst and var kind
    shared_ptr<IRFormat> inst;
    string var;
    int varCost;
    int coloredReg;
};

class RegAllocation {

public:
    RegAllocation(vector<shared_ptr<BasicBlock>> blocks){this->blocks = blocks;numOfNodes = 0;nextVirReg = REG_VIRTUAL;};
    void doRegAllocation();
    void createInterferenceGraph(const string &graphFolder,const string &sourceFileName, string functionName);

private:
    void buildInterfGraph(shared_ptr<BasicBlock> currentBlock, unordered_map<int,shared_ptr<Node>> &liveSet);
    void Coloring();
    vector<shared_ptr<BasicBlock>> blocks;
    void makeEdge(shared_ptr<Node> node1, shared_ptr<Node> node2);
    void getLiveSet(unordered_map<int,shared_ptr<Node>> &liveSet,vector<shared_ptr<IRFormat>> codes);
    void getLiveSetForPhi(unordered_map<int,shared_ptr<Node>> &liveSet, vector<shared_ptr<IRFormat>> codes, int index);

    int numOfNodes;
    vector<bool> regMapTemplate;
    unordered_map<int,shared_ptr<Node>> nodeList;

};


#endif //ADV_COMPILER_REGALLOCATION_H
