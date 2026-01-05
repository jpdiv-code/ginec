#ifndef OPCODES_H
#define OPCODES_H

enum
{
    // OTHER
    OP_NOP = 0x00,

    OP_HLT = 0x02,
    OP_SYNC = 0x03,
    OP_FILL = 0x04,
    OP_FILLw = 0x05,
    OP_SEED = 0x06,
    OP_RAND = 0x07,
    OP_RANDw = 0x08,

    // REG
    OP_MOV = 0x0A,
    OP_MOVw = 0x0B,
    OP_LDI = 0x0C,
    OP_LDIw = 0x0D,
    OP_SRB = 0x0E,

    // STACK
    OP_PUSHI = 0x10,
    OP_PUSHIw = 0x11,
    OP_PUSH = 0x12,
    OP_PUSHw = 0x13,

    OP_POP = 0x15,
    OP_POPw = 0x16,

    OP_SWP = 0x18,
    OP_SWPw = 0x19,

    OP_DUP = 0x1B,
    OP_DUPw = 0x1C,

    OP_ADJSP = 0x1E,

    // RAM
    OP_LD = 0x20,
    OP_LDw = 0x21,
    OP_ST = 0x22,
    OP_STw = 0x23,

    OP_LDR = 0x25,
    OP_LDRw = 0x26,
    OP_STR = 0x27,
    OP_STRw = 0x28,

    OP_LDRI = 0x2A,
    OP_LDRIw = 0x2B,
    OP_STRI = 0x2C,
    OP_STRIw = 0x2D,

    // ROMA
    OP_LDA = 0x30,
    OP_LDAw = 0x31,

    OP_LDAR = 0x33,
    OP_LDARw = 0x34,

    // ALU
    OP_ADD = 0x40,
    OP_SUB = 0x41,
    OP_MUL = 0x42,
    OP_AND = 0x43,
    OP_OR = 0x44,
    OP_XOR = 0x45,
    OP_CMP = 0x46,

    OP_ADDw = 0x48,
    OP_SUBw = 0x49,
    OP_MULw = 0x4A,
    OP_ANDw = 0x4B,
    OP_ORw = 0x4C,
    OP_XORw = 0x4D,
    OP_CMPw = 0x4E,

    OP_ADDR = 0x50,
    OP_SUBR = 0x51,
    OP_MULR = 0x52,
    OP_ANDR = 0x53,
    OP_ORR = 0x54,
    OP_XORR = 0x55,
    OP_CMPR = 0x56,

    OP_ADDRw = 0x58,
    OP_SUBRw = 0x59,
    OP_MULRw = 0x5A,
    OP_ANDRw = 0x5B,
    OP_ORRw = 0x5C,
    OP_XORRw = 0x5D,
    OP_CMPRw = 0x5E,

    OP_INC = 0x60,
    OP_INCw = 0x61,
    OP_DEC = 0x62,
    OP_DECw = 0x63,

    OP_NOT = 0x65,
    OP_NOTw = 0x66,
    OP_SHL = 0x67,
    OP_SHLw = 0x68,
    OP_SHR = 0x69,
    OP_SHRw = 0x6A,
    OP_ASR = 0x6B,
    OP_ASRw = 0x6C,

    OP_SEXT8w = 0x6E,

    // JUMPS
    OP_JMP = 0x70,
    OP_JZ = 0x71,
    OP_JNZ = 0x72,
    OP_JC = 0x73,
    OP_JNC = 0x74,
    OP_JLT = 0x75,
    OP_JGE = 0x76,

    OP_JMPR = 0x78,
    OP_JZR = 0x79,
    OP_JNZR = 0x7A,
    OP_JCR = 0x7B,
    OP_JNCR = 0x7C,
    OP_JLTR = 0x7D,
    OP_JGER = 0x7E,

    // CALLS
    OP_CALL = 0x90,
    OP_CALLR = 0x91,
    OP_RET = 0x92,
};

#endif // OPCODES_H

