//
// Created by tmpark on 2/2/16.
//

#include "GraphDrawer.h"


GraphDrawer *GraphDrawer::_graphDrawer = 0;



GraphDrawer* GraphDrawer::instance() {
    if(!_graphDrawer)
        _graphDrawer = new GraphDrawer();
    return _graphDrawer;
}

RC GraphDrawer::createFile(const string &fileName)
{
    destroyFile(fileName);
    const char* fileName_char = fileName.c_str();
    fstream file_to_create;

    file_to_create.open(fileName_char, fstream::out | fstream:: binary); //Create a file (do not use in when creating a file)
    if(file_to_create.is_open())
    {
        file_to_create.close();
        return 1;
    }
    return -1;
}

RC GraphDrawer::openFile(const std::string &fileName) {
    fileStream.open(fileName.c_str(), std::fstream::in | std::fstream::out);
    if(!fileStream.is_open())
    {
        return -1;
    }
    return 1;
}

RC GraphDrawer::destroyFile(const string &fileName)
{
    const char* fileName_char = fileName.c_str();
    RC success = remove(fileName_char); //delete file
    if(success == 0)
    {
        return 0; //successful
    }

    return -1; //A file still exists
}

RC GraphDrawer::closeFile() {
    fileStream.close();
    return 1;
}

void GraphDrawer :: writePreliminary(string functionName){
    if(!fileStream.is_open())
        return;

    string prelimi = string("digraph \"CFG for \'" + functionName + "\" {\n")
    + string("label=\"CFG for \'") + functionName + "\' function\";\n\n";
    fileStream.write(prelimi.c_str(),prelimi.size());
    return;
}

void GraphDrawer :: writeNodeStart(int blockNum,string blockName){
    if(!fileStream.is_open())
        return;
    string nodeStart = string("Node") + to_string(blockNum) + string(" [shape=record, label=\"{") + blockName
                       + string(":\\l");
    fileStream.write(nodeStart.c_str(),nodeStart.size());
}

void GraphDrawer :: writeCode(string codeString)
{
    if(!fileStream.is_open())
        return;
    codeString = " " + codeString + "\\l";
    fileStream.write(codeString.c_str(),codeString.size());
}

void GraphDrawer :: writeCodeForCond()
{
    if(!fileStream.is_open())
        return;
    string codeString = "|{<s0>T|<s1>F}";
    fileStream.write(codeString.c_str(),codeString.size());
}


void GraphDrawer :: writeNodeEnd(){
    if(!fileStream.is_open())
        return;
    string endString = "}\"];\n";
    fileStream.write(endString.c_str(),endString.size());
}


void GraphDrawer :: writeEdge(int sourceNum, int targetNum, EDGETYPE edgeType)
{
    if(!fileStream.is_open())
        return;

    string edge = string("\t") + string("Node") + to_string(sourceNum);
    if(edgeType == edge_true)
        edge = edge + string(":s0");
    else if(edgeType == edge_false)
    {
        edge = edge + string(":s1");
    }

    edge = edge + string(" -> ") + string("Node") + to_string(targetNum) + string(";\n");

    fileStream.write(edge.c_str(),edge.size());
}

void GraphDrawer :: writeEnd(){
    if(!fileStream.is_open())
        return;
    string endString = "}\n";
    fileStream.write(endString.c_str(),endString.size());
}
