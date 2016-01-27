//
// Created by Taemin Park on 1/11/16.
//

#ifndef ADV_COMPILER_HELPER_H
#define ADV_COMPILER_HELPER_H

#define SOURCE_CODE_PRINT false
#define TraceScan false
#define NO_PARSE false


#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

typedef enum {
    errToken = 0,
    timesToken = 1, divToken = 2,
    plusToken = 11, minusToken = 12,
    eqlToken = 20, neqToken = 21, lssToken = 22, geqToken = 23, leqToken = 24, gtrToken = 25,
    periodToken = 30, commaToken = 31, openbracketToken = 32, closebracketToken = 33, closeparenToken = 35,
    becomesToken = 40, thenToken = 41, doToken = 42,
    openparenToken = 50,
    numberToken = 60, identToken = 61,
    semiToken = 70,
    endToken = 80, odToken = 81, fiToken = 82,
    elseToken = 90,
    letToken = 100, callToken = 101, ifToken = 102, whileToken = 103, returnToken = 104,
    varToken = 110, arrToken = 111, funcToken = 112, procToken = 113,
    beginToken = 150, mainToken = 200, eofToken = 255, commentToken
}TokenType;


static std::unordered_map<int,std::string> tokenStringMap = {{errToken,       "error"}, {timesToken, "*"}, {divToken, "/"},
                                                             {plusToken,      "+"}, {minusToken, "-"}, {eqlToken, "=="}, {neqToken, "!="},
                                                             {lssToken,       "<"}, {geqToken, ">="}, {leqToken, "<="}, {gtrToken, ">"},
                                                             {periodToken,    "."}, {commaToken, ","}, {openbracketToken, "["}, {closebracketToken, "]"}, {closeparenToken, ")"},
                                                             {becomesToken,   "<-"}, {thenToken, "then"}, {doToken, "do"},
                                                             {openparenToken, "("}, {numberToken, "number"}, {identToken, "identifier"}, {semiToken, ";"},
                                                             {endToken,       "}"}, {odToken, "od"}, {fiToken, "fi"}, {elseToken,"else"},
                                                             {letToken,"let"}, {callToken, "call"}, {ifToken, "if"}, {whileToken,"while"}, {returnToken,"return"},
                                                             {varToken,"var"}, {arrToken,"array"}, {funcToken, "function"}, {procToken,"procedure"},
                                                             {beginToken,"{"}, {mainToken,"main"}, {eofToken,"end of file"}, {commentToken,"comment"}};


std::string getTokenStr(TokenType token);


static std::unordered_map<std::string,TokenType> reservedWord = {{"then",thenToken},{"do",doToken},{"od",odToken},
                                  {"fi",fiToken},{"else",elseToken},{"let",letToken},
                                  {"call",callToken},{"if",ifToken},{"while",whileToken},
                                  {"return",returnToken},{"var",varToken},{"array",arrToken},
                                  {"function",funcToken},{"procedure",procToken},{"main",mainToken}};

typedef enum{

    ADD_OP = 0, SUB_OP, MUL_OP, DIV_OP, MOD_OP, CMP_OP,
    OR_OP = 8, AND_OP, BIC_OP, XOR_OP, LSH_OP, ASH_OP, CHK_OP,
    ADDI_OP = 16, SUBI_OP, MULI_OP, DIVI_OP, MODI_OP, CMPI_OP,
    ORI_OP = 24, ANDI_OP, BICI_OP, XORI_OP, LSHI_OP, ASHI_OP, CHKI_OP,
    LDW_OP = 32, LDX_OP, POP_OP,
    STW_OP = 36, STX_OP, PSH_OP,
    BEQ_OP = 40, BNE_OP, BLT_OP, BGE_OP, BLE_OP, BGT_OP, BSR_OP,
    JSR_OP = 48, RET_OP, RDD_OP, WRD_OP, WRH_OP, WRL_OP
}Opcode;

typedef enum{
    IR_err, IR_neg, IR_add, IR_sub, IR_mul, IR_div, IR_cmp, IR_adda, IR_load, IR_store, IR_move, IR_phi, IR_end,
    IR_bra, IR_bne, IR_beq, IR_ble, IR_blt, IR_bge, IR_bgt, IR_read, IR_write, IR_writeNL
}IROP;


TokenType getTypeOfOneToken(char c);
void printToken(TokenType tokenTypeReturned,std::string idConstructed);

bool isRelOp(TokenType scannerSym);
bool isDesignator(TokenType scannerSym);
bool isFactor(TokenType scannerSym);
bool isTerm(TokenType scannerSym);
bool isExpression(TokenType scannerSym);
bool isRelation(TokenType scannerSym);
bool isAssignment(TokenType scannerSym);
bool isFuncCall(TokenType scannerSym);
bool isIfStatement(TokenType scannerSym);
bool isWhileStatement(TokenType scannerSym);
bool isReturnStatement(TokenType scannerSym);
bool isStatement(TokenType scannerSym);
bool isStatSequence(TokenType scannerSym);
bool isTypeDecl(TokenType scannerSym);
bool isVarDecl(TokenType scannerSym);
bool isFuncDecl(TokenType scannerSym);
bool isFormalParam(TokenType scannerSym);
bool isFuncBody(TokenType scannerSym);
Opcode lmmOp(Opcode op);
std::string getIROperatorString(IROP irOp);
IROP getIRopFromToken(TokenType scannerSym);
IROP negateCondition(IROP ir_op);
std::vector<std::string> splitString(std::string stringToSplit);




#endif //ADV_COMPILER_HELPER_H
