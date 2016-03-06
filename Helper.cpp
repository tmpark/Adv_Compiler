//
// Created by Taemin Park on 1/12/16.
//
#include "Helper.h"
#include <iostream>

using namespace std;



std::string getTokenStr(TokenType token)
{
    return tokenStringMap.find(token)->second;
}

TokenType getTypeOfOneToken(char c)
{
    switch(c) {
        case '*':
            return timesToken;
        case '+':
            return plusToken;
        case '-':
            return minusToken;
        case '.':
            return periodToken;
        case ',':
            return commaToken;
        case '[':
            return openbracketToken;
        case ']':
            return closebracketToken;
        case ')':
            return closeparenToken;
        case '(':
            return openparenToken;
        case ';':
            return semiToken;
        case '}':
            return endToken;
        case '{':
            return beginToken;
        case EOF:
            return eofToken;
        default:
            return errToken;
    }
}



void printToken(TokenType tokenTypeReturned,std::string idConstructed)
{
    switch(tokenTypeReturned)
    {
        case timesToken:
            std::cout << "*" << std::endl << std::flush;
            break;
        case divToken:
            std::cout << "/" << std::endl<< std::flush;
            break;
        case plusToken:
            std::cout << "+" << std::endl<< std::flush;
            break;
        case minusToken:
            std::cout << "-" << std::endl<< std::flush;
            break;
        case eqlToken:
            std::cout << "==" << std::endl<< std::flush;
            break;
        case neqToken:
            std::cout << "!=" << std::endl<< std::flush;
            break;
        case lssToken:
            std::cout << "<" << std::endl<< std::flush;
            break;
        case geqToken:
            std::cout << ">=" << std::endl<< std::flush;
            break;
        case leqToken:
            std::cout << "<=" << std::endl<< std::flush;
            break;
        case gtrToken:
            std::cout << ">" << std::endl<< std::flush;
            break;
        case periodToken:
            std::cout << "." << std::endl<< std::flush;
            break;
        case commaToken:
            std::cout << "," << std::endl<< std::flush;
            break;
        case openbracketToken:
            std::cout << "[" << std::endl<< std::flush;
            break;
        case closebracketToken:
            std::cout << "]" << std::endl<< std::flush;
            break;
        case closeparenToken:
            std::cout << ")" << std::endl<< std::flush;
            break;
        case becomesToken:
            std::cout << "<-" << std::endl<< std::flush;
            break;
        case openparenToken:
            std::cout << "(" << std::endl<< std::flush;
            break;
        case semiToken:
            std::cout << ";" << std::endl<< std::flush;
            break;
        case endToken:
            std::cout << "}" << std::endl<< std::flush;
            break;
        case beginToken:
            std::cout << "{" << std::endl<< std::flush;
            break;
        case eofToken:
            std::cout << "end of file"<< std::endl<< std::flush;
            break;
        case thenToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case doToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case odToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case fiToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case elseToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case letToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case callToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case ifToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case whileToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case returnToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case varToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case arrToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case funcToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case procToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case mainToken:
            std::cout << "reserved word: "<< idConstructed << std::endl<< std::flush;
            break;
        case errToken:
            std::cout << "ERROR: " << idConstructed << std::endl<< std::flush;
            break;
        case numberToken:
            std::cout << "NUM, val= " << idConstructed << std::endl<< std::flush;
            break;
        case identToken:
            std::cout << "ID, name= " << idConstructed << std::endl<< std::flush;
            break;
        default:
            std::cout << "Unknown token: " << tokenTypeReturned << std::endl<< std::flush;

    }

}

bool isRelOp(TokenType scannerSym)
{
    return scannerSym == eqlToken || scannerSym == neqToken || scannerSym ==lssToken || scannerSym ==leqToken || scannerSym ==gtrToken || scannerSym ==geqToken;
}

bool isDesignator(TokenType scannerSym)
{

    return (scannerSym == identToken);
}
bool isFactor(TokenType scannerSym)
{
    return isDesignator(scannerSym) || scannerSym == numberToken || scannerSym == openparenToken || isFuncCall(scannerSym);
}
bool isTerm(TokenType scannerSym)
{
    return isFactor(scannerSym);
}
bool isExpression(TokenType scannerSym)
{
    return isTerm(scannerSym);
}
bool isRelation(TokenType scannerSym)
{
    return isExpression(scannerSym);
}
bool isAssignment(TokenType scannerSym)
{
    return scannerSym == letToken;
}
bool isFuncCall(TokenType scannerSym)
{
    return scannerSym == callToken;
}
bool isIfStatement(TokenType scannerSym)
{
    return scannerSym == ifToken;
}
bool isWhileStatement(TokenType scannerSym)
{
    return scannerSym == whileToken;
}
bool isReturnStatement(TokenType scannerSym)
{
    return scannerSym == returnToken;
}
bool isStatement(TokenType scannerSym)
{
    return isAssignment(scannerSym) || isFuncCall(scannerSym) || isIfStatement(scannerSym) || isWhileStatement(scannerSym) || isReturnStatement(scannerSym);
}
bool isStatSequence(TokenType scannerSym)
{
    return isStatement(scannerSym);

}
bool isTypeDecl(TokenType scannerSym)
{
    return scannerSym == varToken || scannerSym == arrToken;
}

bool isVarDecl(TokenType scannerSym)
{
    return isTypeDecl(scannerSym);
}
bool isFuncDecl(TokenType scannerSym)
{
    return scannerSym == funcToken || scannerSym == procToken;
}
bool isFormalParam(TokenType scannerSym)
{
    return scannerSym ==  openparenToken;
}

bool isFuncBody(TokenType scannerSym)
{
    return isVarDecl(scannerSym) || scannerSym == beginToken;
}


OpCode lmmOp(OpCode op)
{
    return (OpCode)(op + 16);
}

std::string getIROperatorString(IROP irOp)
{
    switch(irOp){
        case IR_neg:
            return "neg";
        case IR_add:
            return "add";
        case IR_sub:
            return "sub";
        case IR_mul:
            return "mul";
        case IR_div:
            return "div";
        case IR_cmp:
            return "cmp";
        case IR_adda:
            return "adda";
        case IR_load:
            return "load";
        case IR_store:
            return "store";
        case IR_move:
            return "move";
        case IR_phi:
            return "phi";
        case IR_miu:
            return "miu";
        case IR_end:
            return "end";
        case IR_bra:
            return "bra";
        case IR_bne:
            return "bne";
        case IR_beq:
            return "beq";
        case IR_ble:
            return "ble";
        case IR_blt:
            return "blt";
        case IR_bge:
            return "bge";
        case IR_bgt:
            return "bgt";
        case IR_read:
            return "read";
        case IR_write:
            return "write";
        case IR_writeNL:
            return "writeNL";
        default:
            return "err_IR";
    }

}

IROP getIRopFromToken(TokenType scannerSym)
{
    switch(scannerSym)
    {
        case timesToken:
            return IR_mul;
        case divToken:
            return IR_div;
        case plusToken:
            return IR_add;
        case minusToken:
            return IR_sub;
        case eqlToken:
            return IR_beq;
        case neqToken:
            return IR_bne;
        case lssToken:
            return IR_blt;
        case geqToken:
            return IR_bge;
        case leqToken:
            return IR_ble;
        case gtrToken:
            return IR_bgt;
        default:
            std::cerr << "Unknown token for IR: " << scannerSym << std::endl<< std::flush;
            return IR_err;
    }

}

IROP negateCondition(IROP ir_op)
{
    switch(ir_op)
    {
        case IR_beq:
            return IR_bne;
        case IR_bne:
            return IR_beq;
        case IR_blt:
            return IR_bge;
        case IR_bge:
            return IR_blt;
        case IR_ble:
            return IR_bgt;
        case IR_bgt:
            return IR_ble;
        default:
            std::cerr << "No comparison IR: " << ir_op<< std::endl<< std::flush;
            return IR_err;
    }
}


std::vector<std::string> splitString(std::string stringToSplit)
{
    std::istringstream ss(stringToSplit);
    std::string token;
    vector<string> playerInfoVector;
    while(std::getline(ss, token, ';')) {
        playerInfoVector.push_back(token);
    }
    return playerInfoVector;
}

bool isBranchCond(IROP op)
{
    return (op == IR_bra) ||
            (op == IR_bne) ||
            (op == IR_beq) ||
            (op == IR_ble) ||
            (op == IR_blt) ||
            (op == IR_bge) ||
            (op == IR_bgt);
}

bool isSameOperand(Result x, Result y){
    Kind xKind = x.getKind();
    Kind yKind = y.getKind();
    Kind kind = xKind;
    if(xKind == yKind)
    {
        switch(kind){
            case constKind :
                return x.getConst() == y.getConst();
            case varKind :
                return (x.getVariableName() == y.getVariableName()) && (x.getDefInst() == y.getDefInst());
            case instKind :
                return x.getInst()->getLineNo() == y.getInst()->getLineNo();
            case regKind :
                return x.getReg() == y.getReg();
            case blockKind :
                return x.getBlockNo() == y.getBlockNo();
            default:
                cerr << "No distinguisable kind of operand" << endl;
        }
    }
    else
        return false;

}

bool isInnerBlock(BlockKind blkKind)
{
     return (blkKind == blk_while_body || blkKind == blk_if_then || blkKind == blk_if_else);
}

bool isDefInstr(IROP op)
{
    return op == IR_add || op == IR_sub || op == IR_mul || op == IR_div || op == IR_cmp || op == IR_adda ||
            op == IR_phi || op == IR_read;
}

OpCode irToOp(IROP irOp, OPFORMAT opFormat)
{
    switch(irOp){
        case IR_add:
            if(opFormat == OP_F1)
                return OP_ADDI;
            else
                return OP_ADD;
        case IR_sub:
            if(opFormat == OP_F1)
                return OP_SUBI;
            else
                return OP_SUB;
        case IR_mul:
            if(opFormat == OP_F1)
                return OP_MULI;
            else
                return OP_MUL;
        case IR_div:
            if(opFormat == OP_F1)
                return OP_DIVI;
            else
                return OP_DIV;
        case IR_cmp:
            if(opFormat == OP_F1)
                return OP_CMPI;
            else
                return OP_CMP;
        case IR_load:
            if(opFormat == OP_F1)
                return OP_LDW;
            else
                return OP_LDX;
        case IR_store:
            if(opFormat == OP_F1)
                return OP_STW;
            else
                return OP_STX;
        case IR_end:
            return OP_RET;
        case IR_bra:
            return OP_BEQ;
        case IR_bne:
            return OP_BNE;
        case IR_beq:
            return OP_BEQ;
        case IR_ble:
            return OP_BLE;
        case IR_blt:
            return OP_BLT;
        case IR_bge:
            return OP_BGE;
        case IR_bgt:
            return OP_BGT;
        case IR_read:
            return OP_RDD;
        case IR_write:
            return OP_WRD;
        case IR_writeNL:
            return OP_WRL;
        default:
            return OP_ERR;
    }
}

void getInfoForCodeGen(OpCode opCode,int &numOfArgs, OPFORMAT &opFormat)
{

    switch (opCode) {
        case OP_WRL:
            numOfArgs = 0;
            opFormat = OP_F1;
            break;
        // F1 Format
        case OP_BSR:
            numOfArgs = 1;
            opFormat = OP_F1;
            break;
        case OP_RDD:
            numOfArgs = 1;
            opFormat = OP_F1;
            break;
        case OP_WRD:
        case OP_WRH:
            numOfArgs = 1;
            opFormat = OP_F1;
            break;
            // F2 Format
        case OP_RET:
            numOfArgs = 1;
            opFormat = OP_F2;
            break;
            // F3 Format
        case OP_JSR:
            numOfArgs = 1;
            opFormat = OP_F3;
            break;
            // F1 Format
        case OP_CHKI:
        case OP_BEQ:
        case OP_BNE:
        case OP_BLT:
        case OP_BGE:
        case OP_BLE:
        case OP_BGT:
            numOfArgs = 2;
            opFormat = OP_F1;
            break;
            // F2 Format
        case OP_CHK:
            numOfArgs = 2;
            opFormat = OP_F2;
            break;
            // F1 Format
        case OP_ADDI:
        case OP_SUBI:
        case OP_MULI:
        case OP_DIVI:
        case OP_MODI:
        case OP_CMPI:
        case OP_ORI:
        case OP_ANDI:
        case OP_BICI:
        case OP_XORI:
        case OP_LSHI:
        case OP_ASHI:
        case OP_LDW:
        case OP_POP:
        case OP_STW:
        case OP_PSH:
            numOfArgs = 3;
            opFormat = OP_F1;
            break;
            // F2 Format
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
        case OP_CMP:
        case OP_OR:
        case OP_AND:
        case OP_BIC:
        case OP_XOR:
        case OP_LSH:
        case OP_ASH:
        case OP_LDX:
        case OP_STX:
            numOfArgs = 3;
            opFormat = OP_F2;
            break;
        default:
            cerr << "there is no opcode like this" << endl;
    }
}