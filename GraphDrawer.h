//
// Created by tmpark on 2/2/16.
//

#ifndef ADV_COMPILER_GRAPHDRAWER_H
#define ADV_COMPILER_GRAPHDRAWER_H

#include <fstream>
#include <iostream>
#include "Helper.h"


using namespace std;

class GraphDrawer {
public:
    GraphDrawer(){};
    ~GraphDrawer(){
        _graphDrawer = 0;
    }
    static GraphDrawer* instance();
    RC createFile(const string &fileName);
    RC openFile(const std::string &fileName);
    RC destroyFile(const string &fileName);
    RC closeFile();

    void writePreliminary(GRAPHTYPE graphType, string functionName);
    void writeNodeStart(int blockNum,string blockName);
    void writeCode(string codeString);
    void writeCodeForCond();
    void writeNodeEnd();
    void writeEdge(int sourceNum, int targetNum, EDGETYPE edgeType, GRAPHTYPE graphType);
    void writeEnd();

private:
    static GraphDrawer *_graphDrawer;
    std::fstream fileStream;

};


#endif //ADV_COMPILER_GRAPHDRAWER_H
