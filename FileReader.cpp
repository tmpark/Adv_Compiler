//
// Created by Taemin Park on 1/12/16.
//

#include "FileReader.h"
#include "Helper.h"

using namespace std;

FileReader* FileReader::_fileReader = 0;

FileReader::FileReader() {
    currentLinePosition = 0;
    currentLineSize = 0;
    currentLine = 0;
}


FileReader* FileReader::instance() {
    if(!_fileReader)
        _fileReader = new FileReader();
    return _fileReader;
}

RC FileReader::openFile(const std::string &fileName) {
    fileStream.open(fileName.c_str(), std::fstream::in);
    if(!fileStream.is_open())
    {
        std::cerr << "File not exists";
        return -1;
    }
    return 1;
}


RC FileReader::closeFile() {
    fileStream.close();
    return 1;
}

void FileReader::Error(std::string errorMsg) {
    cout << errorMsg << endl;
}

char FileReader::GetSym() {

    if (isEndOfLine()) {

        if (fileStream.eof())
            return EOF;

        fileStream.getline(lineBuffer, LINEBUFSIZE);
        currentLineSize = std::strlen(lineBuffer);
        currentLine++;
        currentLinePosition = 0;

        if(currentLineSize == 0)
        {
            return '\n';
        }
        //for debug
        if(SOURCE_CODE_PRINT)
            std::cout << lineBuffer << std::endl;
    }

    return lineBuffer[currentLinePosition++];
}

void FileReader::unGetSym()
{
    currentLinePosition--;
}