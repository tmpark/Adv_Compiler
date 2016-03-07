//
// Created by tmpark on 3/4/16.
//

#include "CodeGeneration.h"
#include <bitset>

CodeGeneration :: CodeGeneration(std::unordered_map<std::string,vector<shared_ptr<BasicBlock>>> functionList){
    this->functionList = functionList;
    loc = 0;
}


void CodeGeneration::doCodeGen()
{
    //Extract main block
    vector<shared_ptr<BasicBlock>> blockList = functionList.at("main");
    shared_ptr<BasicBlock> firstBlock = blockList.at(0);
    genCodeForBlock(firstBlock);
    //Fixup
    for(auto fixInfo : locationTobeFixed)
    {
        int fixLoc = fixInfo.first;
        int blockTobeFixed = fixInfo.second;
        int newLoc = startLocOfBlock.at(blockTobeFixed);
        buf[fixLoc] = buf[fixLoc] & 0xFFFF0000;
        buf[fixLoc] = buf[fixLoc] | newLoc-fixLoc & 0x0000FFFF;
    }
    /*
    for(int i = 0 ; i < loc ; i++)
    {
        cout << bitset<32>(buf.at(i)) << endl;
    }*/
}
void CodeGeneration::genCodeForBlock(shared_ptr<BasicBlock> currentBlock)
{
    startLocOfBlock.insert({currentBlock->getBlockNum(),loc});

    //Generate Each code
    for(auto code : currentBlock->irCodes)
    {
        insertCode(code);
    }

    //NextBlock
    for(auto childBlock : currentBlock->DTForwardEdges)
        genCodeForBlock(childBlock);
}

void CodeGeneration::insertCode(shared_ptr<IRFormat> code)
{
    if(code->isElimiated())
        return;
    IROP irOp = code->getIROP();
    size_t numOfIROperand = code->operands.size();

    //end, read, writeNL case
    if(numOfIROperand == 0)
    {
        if(irOp == IR_end)
        {
            int opCode = OP_RET;
            assembleAndPutCode(opCode,0);
        }
        else if(irOp == IR_read)
        {
            int opCode = OP_RDD;
            int instRegNum = code->getRegNo();
            assembleAndPutCode(opCode,instRegNum);
        }
        else if(irOp == IR_writeNL)
        {
            int opCode = OP_WRL;
            assembleAndPutCode(opCode);
        }
        return;
    }
    bool isFirstOperandConst = false;
    Result firstOperand = code->operands.at(0);
    if(firstOperand.getKind() == constKind)
        isFirstOperandConst = true;
    if(numOfIROperand == 1)
    {
        //Fixme:difficult part
        if(irOp == IR_load)
        {

        }
        else if(irOp == IR_bra) //should distinguish whether it is Normal jump or func call
        {
            //Function call
            if(firstOperand.isDiffFuncLoc())
            {

            }
            else//General unconditional branch
            {
                int firstOperandVal = firstOperand.getBlockNo();
                int opCode = OP_BEQ;
                locationTobeFixed.insert({loc,firstOperandVal});
                assembleAndPutCode(opCode,0,firstOperandVal);
            }

        }
        else if(irOp == IR_write)
        {
            int outputReg;
            if(isFirstOperandConst)
            {
                assembleAndPutCode(OP_ADDI,REG_PROXY,0,firstOperand.getConst());
                outputReg = REG_PROXY;
            }
            else
                outputReg = firstOperand.getReg();
            assembleAndPutCode(OP_WRD,outputReg);
        }
        return;
    }
    bool isSecondOperandConst = false;
    Result secondOperand = code->operands.at(1);
    if(secondOperand.getKind() == constKind)
        isSecondOperandConst = true;

    if(numOfIROperand > 1)
    {
        //kinds of adds
        if(irOp == IR_add || irOp == IR_sub || irOp == IR_mul || irOp == IR_div || irOp == IR_cmp)
        {
            int destReg = code->getRegNo();
            int firstOperandReg;
            int secondOperandVal;

            OPFORMAT opFormat;
            if(isFirstOperandConst) {
                assembleAndPutCode(OP_ADDI, REG_PROXY, 0, firstOperand.getConst());
                firstOperandReg = REG_PROXY;
            }
            else
                firstOperandReg = firstOperand.getReg();

            if(isSecondOperandConst) {
                opFormat = OP_F1;
                secondOperandVal = secondOperand.getConst();
            }
            else {
                opFormat = OP_F2;
                secondOperandVal = secondOperand.getReg();
            }
            int opCode = irToOp(irOp,opFormat);
            assembleAndPutCode(opCode,destReg,firstOperandReg,secondOperandVal);
        }
        else if(irOp == IR_store)
        {

        }
        else if(irOp == IR_move)
        {

            int firstOperandReg;
            int secondOperandReg = secondOperand.getReg();
            int destReg = secondOperandReg;
            if(isFirstOperandConst) {
                assembleAndPutCode(OP_ADDI, REG_PROXY, 0, firstOperand.getConst());
                firstOperandReg = REG_PROXY;
            }
            else
                firstOperandReg = firstOperand.getReg();

            OPFORMAT opFormat = OP_F2;
            int opCode = OP_ADD;
            assembleAndPutCode(opCode,destReg,0,firstOperandReg);
        }
        else if(irOp == IR_bne || irOp == IR_beq || irOp == IR_ble || irOp == IR_blt || irOp == IR_bge|| irOp == IR_bgt)
        {
            int firstOperandReg = firstOperand.getReg();
            int secondOperandVal = secondOperand.getBlockNo();
            OPFORMAT opFormat = OP_F1;
            int opCode = irToOp(irOp,opFormat);
            locationTobeFixed.insert({loc,secondOperandVal});
            assembleAndPutCode(opCode,firstOperandReg,secondOperandVal);
        }
        return;
    }

}

void CodeGeneration::assembleAndPutCode(int op)
{
    OpCode opCode = (OpCode)op;
    if(opCode != OP_WRL)
        cerr << "There is only one opcode for no operand : WRL" << endl;
    PutF1(op,0,0,0);
}

void CodeGeneration::assembleAndPutCode(int op, int arg1)
{
    OpCode opCode = (OpCode)op;
    switch (opCode) {
        // F1 Format
        case OP_BSR:
            PutF1(op,0,0,arg1);
            break;
        case OP_RDD:
            PutF1(op,arg1,0,0);
            break;
        case OP_WRD:
        case OP_WRH:
            PutF1(op,0,arg1,0);
            break;
            // F2 Format
        case OP_RET:
            PutF2(op,0,0,arg1);
            break;
            // F3 Format
        case OP_JSR:
            PutF3(op,arg1);
            break;
        default:
            cerr << "This opcode is not for one argument" << endl;
    }
}

void CodeGeneration::assembleAndPutCode(int op, int arg1, int arg2)
{
    OpCode opCode = (OpCode)op;
    switch (opCode) {
        // F1 Format
        case OP_CHKI:
        case OP_BEQ:
        case OP_BNE:
        case OP_BLT:
        case OP_BGE:
        case OP_BLE:
        case OP_BGT:
            PutF1(op,arg1,0,arg2);
            break;
            // F2 Format
        case OP_CHK:
            PutF2(op,arg1,0,arg2);
            break;
        default:
            cerr << "This opcode is not for two arguments" << endl;
    }
}

void CodeGeneration::assembleAndPutCode(int op, int arg1, int arg2, int arg3)
{
    OpCode opCode = (OpCode)op;
    switch (opCode) {
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
            PutF1(op,arg1,arg2,arg3);
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
            PutF2(op,arg1,arg2,arg3);
            break;
        default:
            cerr << "This opcode is not for two arguments" << endl;
    }
}

void CodeGeneration::PutF1(int op, int a, int b, int c) {
    buf.at(loc) = op << 26 |
                 a << 21 |
                 b << 16 |
                 c & 0x0000FFFF;
    loc++;
}

void CodeGeneration::PutF2(int op, int a, int b, int c) {
    buf.at(loc) = op << 26 |
                  a << 21 |
                  b << 16 |
                  c & 0x0000001F;
    loc++;
}

void CodeGeneration::PutF3(int op, int c) {
    buf.at(loc) = op << 26 |
                  c & 0x03FFFFFF;
    loc++;
}


void CodeGeneration::writeOutCode(const string &binaryFolder,const string &sourceFileName)
{
    string formatName = ".out";
    RC rc = -1;
    string fileName = binaryFolder +  sourceFileName + formatName;

    destroyFile(fileName);
    rc = createFile(fileName);
    if (rc == -1)
        return;
    rc = openFile(fileName);
    if (rc == -1) {
        destroyFile(fileName);
        return;
    }

    fileStream.write((char*)buf.data(),loc*sizeof(int32_t));
    fileStream.close();
    return;
}



RC CodeGeneration::createFile(const string &fileName)
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

RC CodeGeneration::openFile(const std::string &fileName) {
    fileStream.open(fileName.c_str(), std::fstream::in | std::fstream::out | fstream::binary);
    if(!fileStream.is_open())
    {
        return -1;
    }
    return 1;
}

RC CodeGeneration::destroyFile(const string &fileName)
{
    const char* fileName_char = fileName.c_str();
    RC success = remove(fileName_char); //delete file
    if(success == 0)
    {
        return 0; //successful
    }

    return -1; //A file still exists
}

RC CodeGeneration::closeFile() {
    fileStream.close();
    return 1;
}