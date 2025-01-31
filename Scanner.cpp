//
// Created by Taemin Park on 1/11/16.
//

#include "Scanner.h"

using namespace :: std;

Scanner* Scanner::_scanner = 0;

Scanner* Scanner::instance() {
    if(!_scanner)
        _scanner = new Scanner();
    return _scanner;
}


RC Scanner::openFile(const std::string &fileName) {
    RC rc;
    FileReader *fileReader = FileReader::instance();
    rc = fileReader->openFile(fileName);
    return rc;
}

RC Scanner::closeFile() {
    RC rc;
    FileReader *fileReader = FileReader::instance();
    rc = fileReader->closeFile();
    return rc;
}

void Scanner ::Next() {
    FileReader *fileReader = FileReader::instance();
    inputSym = fileReader->GetSym();
}

void Scanner ::Previous() {
    FileReader *fileReader = FileReader::instance();
    fileReader->unGetSym();
}

TokenType Scanner :: GetSym()
{
    FileReader *fileReader = FileReader::instance();
    StateType state = STAT_START;
    TokenType tokenTypeReturned = errToken;
    string stringConstructed;

    while(state != STAT_DONE)
    {
        Next();
        switch (state) {
            case STAT_START :
                if(isspace(inputSym)) //Skip blank character
                    state = STAT_START;
                else if (isdigit(inputSym)) {
                    state = STAT_NUM;
                    stringConstructed = stringConstructed + inputSym;
                }
                else if (isalpha(inputSym)) {
                    state = STAT_ID;
                    stringConstructed = stringConstructed + inputSym;
                }
                else if (inputSym == '=')
                    state = STAT_EQ;
                else if (inputSym == '!')
                    state = STAT_NE;
                else if (inputSym == '<')
                    state = STAT_L;
                else if (inputSym == '>')
                    state = STAT_R;
                else if (inputSym == '#')
                    state = STAT_COMMENT;
                else if(inputSym == '/')
                    state = STAT_SLASH;
                else {
                    tokenTypeReturned = getTypeOfOneToken(inputSym);
                    state = STAT_DONE;
                }
                break;

            case STAT_NUM :
                if (!isdigit(inputSym)) {
                    Previous();
                    state = STAT_DONE;
                    tokenTypeReturned = numberToken;
                }
                else
                    stringConstructed = stringConstructed + inputSym;
                break;

            case STAT_ID :
                if (!isalnum(inputSym)) {
                    Previous();
                    state = STAT_DONE;
                    tokenTypeReturned = identToken;
                }
                else
                    stringConstructed = stringConstructed + inputSym;
                break;

            case STAT_EQ:
                state = STAT_DONE;
                if (inputSym == '=')
                    tokenTypeReturned = eqlToken;
                else {
                    Previous();
                    tokenTypeReturned = errToken;
                    Error("STAT_EQ","=");
                }
                break;

            case STAT_NE:
                state =  STAT_DONE;
                if (inputSym == '=')
                    tokenTypeReturned = neqToken;

                else
                {
                    Previous();
                    tokenTypeReturned = errToken;
                    Error("STAT_NE","=");
                }
                break;

            case STAT_L:
                state = STAT_DONE;
                if (inputSym == '=')
                    tokenTypeReturned = leqToken;
                else if(inputSym == '-')
                    tokenTypeReturned = becomesToken;
                else {
                    Previous();
                    tokenTypeReturned = lssToken;
                }
                break;


            case STAT_R:
                state = STAT_DONE;
                if (inputSym == '=')
                    tokenTypeReturned = geqToken;
                else {
                    Previous();
                    tokenTypeReturned = gtrToken;
                }
                break;
            case STAT_COMMENT:
                if (inputSym == '\n')
                {
                    state = STAT_START;
                    tokenTypeReturned = commentToken;
                }

                break;

            case STAT_SLASH:
                if (inputSym == '/')
                    state = STAT_COMMENT;
                else {
                    Previous();
                    state = STAT_DONE;
                    tokenTypeReturned = divToken;
                }
                break;

            case STAT_DONE:
                break;

            default:
                std::cerr << "Scanner Bug: state= " << state << endl;
                state = STAT_DONE;
                tokenTypeReturned = errToken;
                break;
        }

    }

    if(tokenTypeReturned == identToken) {
        auto it = reservedWord.find(stringConstructed);
        if(it != reservedWord.end())
        {
            tokenTypeReturned = it->second;
        }
        else
            id = stringConstructed;
    }
    else if(tokenTypeReturned == numberToken){
        number = std::stoi(stringConstructed);
    }

    if(TraceScan)
    {
        cout << "\t" << fileReader->getCurrentLine() << " "<<flush;
        printToken(tokenTypeReturned, stringConstructed);
    }
    return tokenTypeReturned;
}

void Scanner :: Error(std::string state, std::string missingChar)
{
    cerr << "Scanner error: in " << state << ", " << missingChar << " is missing" << endl;
}


