//
// Created by Taemin Park on 1/12/16.
//

#ifndef ADV_COMPILER_FILEREADER_H
#define ADV_COMPILER_FILEREADER_H

#include <fstream>
#include <iostream>
#include "Helper.h"
#define LINEBUFSIZE 4096

using namespace std;



class FileReader {

public:

    FileReader();
    static FileReader* instance();
    RC openFile(const std::string &fileName);
    RC closeFile();

    char GetSym();
    void unGetSym();
    void Error(std::string errorMsg);

    bool isEndOfLine(){return currentLinePosition >= currentLineSize;}
    unsigned getCurrentLine(){return currentLine;};

private:
    static FileReader *_fileReader;
    std::fstream fileStream;
    char lineBuffer[LINEBUFSIZE];

    //Position information
    unsigned currentLinePosition;
    size_t currentLineSize;
    unsigned currentLine;

};


#endif //ADV_COMPILER_FILEREADER_H
