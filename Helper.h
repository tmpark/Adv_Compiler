//
// Created by Taemin Park on 1/11/16.
//

#ifndef ADV_COMPILER_HELPER_H
#define ADV_COMPILER_HELPER_H

#define SOURCE_CODE_PRINT false
#define TraceScan true
#define NO_PARSE true


#include <string>
#include <unordered_map>

typedef enum {
    errorToken,
    timesToken, divToken,
    plusToken, minusToken,
    eqlToken, neqToken, lssToken, geqToken, leqToken, gtrToken,
    periodToken, commaToken, openbracketToken, closebracketToken, closeparenToken,
    becomesToken, thenToken, doToken,
    openparenToken,
    numberToken, identToken,
    semiToken,
    endToken, odToken, fiToken,
    elseToken,
    letToken, callToken, ifToken, whileToken, returnToken,
    varToken, arrToken, funcToken, procToken,
    beginToken, mainToken, eofToken
}TokenType;



static std::unordered_map<std::string,TokenType> reservedWord = {{"then",thenToken},{"do",doToken},{"od",odToken},
                                  {"fi",fiToken},{"else",elseToken},{"let",letToken},
                                  {"call",callToken},{"if",ifToken},{"while",whileToken},
                                  {"return",returnToken},{"var",varToken},{"array",arrToken},
                                  {"function",funcToken},{"procedure",procToken},{"main",mainToken}};

TokenType getTypeOfOneToken(char c);
TokenType reservedWordCheck(std::string &idConstructed);
void printToken(TokenType tokenTypeReturned,std::string idConstructed);


#endif //ADV_COMPILER_HELPER_H
