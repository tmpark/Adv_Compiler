//
// Created by Taemin Park on 1/14/16.
//

#include "Parser.h"

Parser* Parser::_parser = 0;


Parser* Parser::instance() {
    if(!_parser)
        _parser = new Parser();
    return _parser;
}


RC Parser::openFile(const std::string &fileName) {
    RC rc;
    Scanner *scanner = Scanner::instance();
    rc = scanner->openFile(fileName);
    return rc;
}

RC Parser::closeFile() {
    RC rc;
    Scanner *scanner = Scanner::instance();
    rc = scanner->closeFile();
    return rc;
}

void Parser ::Next() {
    Scanner *scanner = Scanner::instance();
    scannerSym = scanner->GetSym();
}

void Parser :: startParse()
{
    Next();
    computation();
}
void Parser :: Error(std::string nonTerminal, std::initializer_list<std::string> missingTokens){
    std::cerr << "Parser error: in " << nonTerminal << ", " <<std::flush;

    for (auto i: missingTokens)
        std::cerr << "(" << i << ") "<<std::flush;

    std::cerr << "is missing" << std::endl<<std::flush;
}

/*
void Parser :: Error(std::string nonTerminal, std::string missingTerm){
        std::cerr << "Parser error: in " << nonTerminal << ", " << missingTerm << " is missing" << std::endl;
}

void Parser :: Error(std::string nonTerminal, std::string missingTerm, int numOfToken, ...){
    std::cerr << "Parser error: in " << nonTerminal << ", " << missingTerm;

    va_list tokenLists;
    va_start(tokenLists, numOfToken);
    for(unsigned i = 0 ; i < numOfToken ; i++)
    {
        int missingToken = va_arg(tokenLists, int);
        std::cerr << " or" << tokenStringMap.find(missingToken)->second;
    }
    va_end(tokenLists);

    std::cerr << " is missing" << std::endl;
}*/

//Push down automaton
void Parser::computation() {
    if(scannerSym == mainToken)
    {
        Next(); //Consume main
        while(isVarDecl(scannerSym))
            varDecl();
        while(isFuncDecl(scannerSym))
            funcDecl();
        if(scannerSym == beginToken)
        {
            Next(); //Consume begin Token
            if(isStatSequence(scannerSym))
            {
                statSequence();
                if(scannerSym == endToken)
                {
                    Next(); //Consume end Token
                    if(scannerSym == periodToken)
                        Next(); //Comsume period Token
                    else
                        Error("computation",{getTokenStr(periodToken)});
                }
                else
                    Error("computation",{"statSequence"});
            }
            else
                Error("computation",{"statSequence"});
        }
        else
            Error("computation",{getTokenStr(beginToken)});

    }
    else
        Error("computation",{getTokenStr(mainToken)});
}


void Parser:: funcBody(){
    while(isVarDecl(scannerSym))
    {
        varDecl();
    }
    if(scannerSym == beginToken)
    {
        Next();
        if(isStatSequence(scannerSym))
        {
            statSequence();
        }
        if(scannerSym == endToken)
        {
            Next();
        }
        else
            Error("funcBody",{getTokenStr(endToken)});
    }
    else
        Error("funcBody",{getTokenStr(beginToken)});

}


void Parser::formalParam(){
    if(scannerSym == openparenToken)
    {
        Next();
        if(scannerSym == identToken)
        {
            Next();
            while(scannerSym == commaToken)
            {
                Next();
                if(scannerSym == identToken)
                {
                    Next();
                }
                else
                    Error("formalParam",{getTokenStr(identToken)});
            }
        }

        if(scannerSym == closeparenToken)
        {
            Next();
        }
        else
            Error("formalParam",{getTokenStr(closeparenToken)});
    }
    else
        Error("formalParam",{getTokenStr(openparenToken)});
}

void Parser::funcDecl(){
    if(scannerSym == funcToken || scannerSym == procToken)
    {
        Next();
        if(scannerSym == identToken)
        {
            Next();
            if(isFormalParam(scannerSym))
                formalParam();
            if(scannerSym == semiToken)
            {
                Next();
                if(isFuncBody(scannerSym))
                {
                    funcBody();
                    if(scannerSym == semiToken)
                    {
                        Next();
                    }
                    else
                        Error("funcDecl",{getTokenStr(semiToken)});
                }
                else
                    Error("funcDecl",{"funcBody"});

            }
            else
                Error("funcDecl",{getTokenStr(semiToken)});

        }
        else
            Error("funcDecl",{getTokenStr(identToken)});
    }
    else
        Error("funcDecl",{getTokenStr(funcToken),getTokenStr(procToken)});

}

void Parser::varDecl() {

    if(isTypeDecl(scannerSym))
    {
        typeDecl();
        if(scannerSym == identToken)
        {
            Next();
            while(scannerSym == commaToken)
            {
                Next(); // Consume comma
                if(scannerSym == identToken)
                {
                    Next();
                }
                else
                    Error("varDecl",{getTokenStr(identToken)});

            }
            if(scannerSym == semiToken)
            {
                Next();
            }
            else
                Error("varDecl",{getTokenStr(semiToken)});
        }
        else
            Error("varDecl",{getTokenStr(identToken)});
    }
    else
        Error("varDecl",{"typeDecl"});
}

void Parser::typeDecl() {

    if(scannerSym == varToken)
    {
        Next(); //Consume var Token
    }
    else if(scannerSym == arrToken)
    {
        Next(); //Consume var Token

        if(scannerSym == openbracketToken)
        {
            Next(); //Consume open bracket Token
            if(scannerSym == numberToken)
            {
                Next();
                if(scannerSym == closebracketToken)
                {
                    Next();
                    while(scannerSym == openbracketToken) {

                        Next(); //Consume open bracket Token

                        if (scannerSym == numberToken) {
                            Next();
                            if (scannerSym == closebracketToken) {
                                Next();
                            }
                            else
                                Error("typeDecl",{getTokenStr(closebracketToken)});
                        }
                        else
                            Error("typeDecl",{getTokenStr(numberToken)});
                    }
                }
                else
                    Error("typeDecl",{getTokenStr(closebracketToken)});
            }
            else
                Error("typeDecl",{getTokenStr(numberToken)});
        }
        else
            Error("typeDecl",{getTokenStr(openbracketToken)});
    }
    else
        Error("typeDecl",{getTokenStr(varToken),getTokenStr(arrToken)});
}

void Parser::statSequence() {
    if(isStatement(scannerSym))
    {
        statement();
        while(scannerSym == semiToken)
        {
            Next();
            if(isStatement(scannerSym))
            {
                statement();
            }
            else
                Error("statSequence",{"statement"});
        }

    }
    else
        Error("statSequence",{"statement"});
}

void  Parser::statement()
{
    if(isAssignment(scannerSym))
    {
        assignment();
    }
    else if(isFuncCall(scannerSym))
    {
        funcCall();
    }
    else if(isIfStatement(scannerSym))
    {
        ifStatement();
    }
    else if(isWhileStatement(scannerSym))
    {
        whileStatement();
    }
    else if(isReturnStatement(scannerSym))
    {
        returnStatement();
    }
    else
        Error("statement",{"assignment","funcCall","ifStatement","whileStatement","returnStatement"});
}

void Parser::returnStatement() {
    if(scannerSym == returnToken)
    {
        Next();
        if(isExpression(scannerSym))
        {
            expression();
        }

    }
    else
        Error("returnStatement",{getTokenStr(returnToken)});
}

void Parser::whileStatement() {
    if(scannerSym == whileToken)
    {
        Next();
        if(isRelation(scannerSym))
        {
            relation();
            if(scannerSym == doToken)
            {
                Next();
                if(isStatSequence(scannerSym))
                {
                    statSequence();
                    if(scannerSym == odToken)
                    {
                        Next();
                    }
                    else
                        Error("whileStatement",{getTokenStr(odToken)});
                }
                else
                    Error("whileStatement",{"statSequence"});
            }
            else
                Error("whileStatement",{getTokenStr(doToken)});
        }
        else
            Error("whileStatement",{"relation"});
    }
    else
        Error("whileStatement",{getTokenStr(whileToken)});
}

void Parser::ifStatement() {
    if(scannerSym == ifToken)
    {
        Next();
        if(isRelation(scannerSym))
        {
            relation();
            if(scannerSym == thenToken)
            {
                Next();
                if(isStatSequence(scannerSym))
                {
                    statSequence();
                    if(scannerSym == elseToken)
                    {
                        Next();
                        if(isStatSequence(scannerSym))
                        {
                            statSequence();
                        }
                        else
                            Error("ifStatement",{"statSequence"});
                    }
                    if(scannerSym == fiToken)
                    {
                        Next();
                    }
                    else
                        Error("ifStatement",{getTokenStr(fiToken)});
                }
                else
                    Error("ifStatement",{"statSequence"});
            }
            else
                Error("ifStatement",{getTokenStr(thenToken)});
        }
        else
            Error("ifStatement",{"relation"});
    }
    else
        Error("ifStatement",{getTokenStr(ifToken)});
}

void Parser::funcCall() {
    if(scannerSym == callToken)
    {
        Next();
        if(scannerSym == identToken)
        {
            Next();
            if(scannerSym == openparenToken)
            {
                Next();
                if(isExpression((scannerSym)))
                {
                    expression();
                    while(scannerSym == commaToken)
                    {
                        Next();
                        if(isExpression(scannerSym))
                        {
                            expression();
                        }
                        else
                            Error("funcCall",{"expression"});
                    }
                }
                if(scannerSym == closeparenToken)
                {
                    Next();
                }
                else
                    Error("funcCall",{getTokenStr(closeparenToken)});
            }

        }
        else
            Error("funcCall",{getTokenStr(identToken)});
    }
    else
        Error("funcCall",{getTokenStr(callToken)});
}

void Parser::assignment() {
    if(scannerSym == letToken)
    {
        Next();
        if(isDesignator(scannerSym))
        {
            designator();
            if(scannerSym == becomesToken)
            {
                Next();
                if(isExpression(scannerSym))
                {
                    expression();
                }
                else
                    Error("assignment",{"expression"});
            }
            else
                Error("assignment",{getTokenStr(becomesToken)});
        }
        else
            Error("assignment",{"designator"});
    }
    else
        Error("assignment",{getTokenStr(letToken)});

}

void Parser::relation() {
    if(isExpression(scannerSym))
    {
        expression();
        if(isRelOp(scannerSym))
        {
            relOp();
            if(isExpression(scannerSym))
            {
                expression();
            }
            else
                Error("relation",{"expression"});
        }
        else
            Error("relation",{"relOp"});
    }
    else
        Error("relation",{"expression"});
}

void Parser::expression() {
    if(isTerm(scannerSym))
    {
        term();
        while(scannerSym == plusToken || scannerSym == minusToken)
        {
            Next();
            if(isTerm(scannerSym))
            {
                term();
            }
            else
                Error("expression",{"term"});

        }
    }
    else
        Error("expression",{"term"});
}

void Parser::term()
{
    if(isFactor(scannerSym))
    {
        factor();
        while(scannerSym == timesToken || scannerSym == divToken)
        {
            Next();
            if(isFactor(scannerSym))
            {
                factor();
            }
            else
                Error("term",{"factor"});
        }
    }
    else
        Error("term",{"factor"});

}

void Parser::factor()
{
    if(isDesignator(scannerSym))
    {
        designator();
    }
    else if(scannerSym == numberToken)
    {
        Next();
    }
    else if(scannerSym == openparenToken)
    {
        Next();
        if(isExpression(scannerSym))
        {
            expression();
            if(scannerSym == closeparenToken)
            {
                Next();
            }
            else
                Error("factor",{getTokenStr(closeparenToken)});
        }
        else
            Error("factor",{"expression"});
    }
    else if(isFuncCall(scannerSym))
    {
        funcCall();
    }
    else
        Error("factor",{"designator","funcCall",getTokenStr(numberToken), getTokenStr(openparenToken)});
}

void Parser::designator() {
    if(scannerSym == identToken)
    {
        Next();
        while(scannerSym == openbracketToken)
        {
            Next();
            if(isExpression(scannerSym))
            {
                expression();
                if(scannerSym == closebracketToken)
                {
                    Next();
                }
                else
                    Error("designator",{getTokenStr(closebracketToken)});
            }
            else
                Error("designator",{"expression"});

        }
    }
    else
        Error("designator",{getTokenStr(identToken)});

}

void Parser::relOp() {

    if(isRelOp(scannerSym))
    {
        Next();
    }
    else
        Error("relOp",{"relOp"});
}