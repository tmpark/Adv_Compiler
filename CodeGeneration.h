//
// Created by tmpark on 3/4/16.
//

#ifndef ADV_COMPILER_CODEGENERATION_H
#define ADV_COMPILER_CODEGENERATION_H

#define CODEBUFSIZE 4096

#include "RegAllocation.h"

class CodeGeneration {
public:
    CodeGeneration(std::unordered_map<std::string,vector<shared_ptr<BasicBlock>>> functionList);
    void doCodeGen();
    void writeOutCode(const string &binaryFolder,const string &sourceFileName);

private:
    std::unordered_map<std::string,vector<shared_ptr<BasicBlock>>> functionList;
    void genCodeForBlock(string functionName, shared_ptr<BasicBlock> currentBlock);
    void insertCode(string functionName, shared_ptr<IRFormat> code);
    void PutF1(int op, int a, int b, int c);
    void PutF2(int op, int a, int b, int c);
    void PutF3(int op, int c);

    void assembleAndPutCode(int op);
    void assembleAndPutCode(int op, int arg1);
    void assembleAndPutCode(int op, int arg1, int arg2);
    void assembleAndPutCode(int op, int arg1, int arg2, int arg3);

    RC createFile(const string &fileName);
    RC openFile(const std::string &fileName);
    RC destroyFile(const string &fileName);
    RC closeFile();

    std::array<int32_t, CODEBUFSIZE> buf;
    int loc;
    int numOfGlobalVar;
    std::fstream fileStream;

    unordered_map<int,int> startLocOfBlock;
    unordered_map<int,int> locationTobeFixed;
};


#endif //ADV_COMPILER_CODEGENERATION_H
