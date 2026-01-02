#ifndef OPCODES_H
#define OPCODES_H

// ==============================
// OPCODES
// ==============================

enum
{
    // =====
    // OTHER
    // =====

    OP_NOP = 0x00,
    OP_HLT = 0x01,
    OP_SYNC = 0x02,

    OP_FILL = 0x04,  // 3 operands: RA RP RL, fill RAM region starting at address in RA with 8-bit
                     // value in RP for RL bytes (4 byte-long instruction)
    OP_FILLw = 0x05, // 3 operands: RA RP RL, fill RAM region starting at address in RA with 16-bit
                     // value in RP for RL words (4 byte-long instruction)

    OP_SEED = 0x07, // 1 operand: RS, seed the random number generator with the 16-bit value in RS
                    // (2 byte-long instruction)
    OP_RAND = 0x08, // 1 operand: RD, generate a random 16-bit value and store it in RD (2 byte-long
                    // instruction)

    // ===
    // REG
    // ===

    OP_MOV = 0x0A, // 2 operands: RD RS, move value from RS to RD (3 byte-long instruction)

    OP_LDI = 0x0C, // 2 operands: RD IMM8, load immediate 8-bit value into RD, zero-filled (3
                   // byte-long instruction)
    OP_LDIw =
        0x0D, // 2 operands: RD IMM16, load immediate 16-bit value into RD (4 byte-long instruction)

    // =====
    // STACK
    // =====

    OP_PUSHI =
        0x10, // 1 operand: IMM8, push immediate 8-bit value onto stack (2 byte-long instruction)
    OP_PUSHIw =
        0x11, // 1 operand: IMM16, push immediate 16-bit value onto stack (3 byte-long instruction)

    OP_PUSH = 0x13,  // 1 operand: RS, push an 8-bit value from register onto stack (2 byte-long
                     // instruction)
    OP_PUSHw = 0x14, // 1 operand: RS, push a 16-bit value from register onto stack (2 byte-long
                     // instruction)

    OP_POP = 0x16,  // 1 operand: RD, pop an 8-bit value from stack into register, zero-filled (2
                    // byte-long instruction)
    OP_POPw = 0x17, // 1 operand: RD, pop a 16-bit value from stack into register (2 byte-long
                    // instruction)

    OP_SWP = 0x19,  // 0 operands, swap top two 8-bit values on stack (1 byte-long instruction)
    OP_SWPw = 0x1A, // 0 operands, swap top two 16-bit values on stack (1 byte-long instruction)

    OP_DUP = 0x1C,  // 0 operands, duplicate top 8-bit value on stack (1 byte-long instruction)
    OP_DUPw = 0x1D, // 0 operands, duplicate top 16-bit value on stack (1 byte-long instruction)

    // ===
    // RAM
    // ===

    OP_LD = 0x20,  // 2 operands: RD addr, load 8-bit value from RAM at address in addr into RD,
                   // zero-filled (4 byte-long instruction)
    OP_LDw = 0x21, // 2 operands: RD addr, load 16-bit value from RAM at address in addr into RD (4
                   // byte-long instruction)
    OP_ST = 0x22,  // 2 operands: RS addr, store 8-bit value from RS into RAM at address in addr (4
                   // byte-long instruction)
    OP_STw = 0x23, // 2 operands: RS addr, store 16-bit value from RS into RAM at address in addr (4
                   // byte-long instruction)

    OP_LDR = 0x25,  // 2 operands: RD RA, load 8-bit value from RAM at address in RA into RD,
                    // zero-filled (3 byte-long instruction)
    OP_LDRw = 0x26, // 2 operands: RD RA, load 16-bit value from RAM at address in RA into RD (3
                    // byte-long instruction)
    OP_STR = 0x27,  // 2 operands: RS RA, store 8-bit value from RS into RAM at address in RA (3
                    // byte-long instruction)
    OP_STRw = 0x28, // 2 operands: RS RA, store 16-bit value from RS into RAM at address in RA (3
                    // byte-long instruction)

    OP_LDS = 0x2A, // 1 operand: RD, load 8-bit value from RAM at address popped from stack into RD,
                   // zero-filled (2 byte-long instruction)
    OP_LDSw = 0x2B, // 1 operand: RD, load 16-bit value from RAM at address popped from stack into
                    // RD (2 byte-long instruction)
    OP_STS = 0x2C, // 1 operand: RS, store 8-bit value from RS into RAM at address popped from stack
                   // (2 byte-long instruction)
    OP_STSw = 0x2D, // 1 operand: RS, store 16-bit value from RS into RAM at address popped from
                    // stack (2 byte-long instruction)

    // ====
    // ROMA
    // ====

    OP_LDA = 0x30,  // 2 operands: RD addr, load 8-bit value from ROMA at address in addr into RD,
                    // zero-filled (4 byte-long instruction)
    OP_LDAw = 0x31, // 2 operands: RD addr, load 16-bit value from ROMA at address in addr into RD
                    // (4 byte-long instruction)

    OP_LDAR = 0x33,  // 2 operands: RD RA, load 8-bit value from ROMA at address in RA into RD,
                     // zero-filled (3 byte-long instruction)
    OP_LDARw = 0x34, // 2 operands: RD RA, load 16-bit value from ROMA at address in RA into RD (3
                     // byte-long instruction)

    OP_LDAS = 0x36,  // 1 operand: RD, load 8-bit value from ROMA at address popped from stack into
                     // RD, zero-filled (2 byte-long instruction)
    OP_LDASw = 0x37, // 1 operand: RD, load 16-bit value from ROMA at address popped from stack into
                     // RD (2 byte-long instruction)

    // ===
    // ALU
    // ===

    OP_ADD = 0x40, // 2 operands: RD imm8, add immediate 8-bit value to the low byte of RD,
                   // preserving the high byte in RD (3 byte-long instruction)
    OP_SUB = 0x41, // 2 operands: RD imm8, subtract immediate 8-bit value from RD, ignoring high
                   // byte in RD (3 byte-long instruction)
    OP_MUL = 0x42, // 2 operands: RD imm8, multiply RD by immediate 8-bit value, ignoring high byte
                   // in RD (3 byte-long instruction)
    OP_AND = 0x43, // 2 operands: RD imm8, bitwise AND immediate 8-bit value with RD, ignoring high
                   // byte in RD (3 byte-long instruction)
    OP_OR = 0x44,  // 2 operands: RD imm8, bitwise OR immediate 8-bit value with RD, ignoring high
                   // byte in RD (3 byte-long instruction)
    OP_XOR = 0x45, // 2 operands: RD imm8, bitwise XOR immediate 8-bit value with RD, ignoring high
                   // byte in RD (3 byte-long instruction)
    OP_CMP = 0x46, // 2 operands: RD imm8, compare RD with immediate 8-bit value, set flags
                   // accordingly, ignoring high byte in RD (3 byte-long instruction)

    OP_ADDw =
        0x48, // 2 operands: RD imm16, add immediate 16-bit value to RD (4 byte-long instruction)
    OP_SUBw = 0x49, // 2 operands: RD imm16, subtract immediate 16-bit value from RD (4 byte-long
                    // instruction)
    OP_MULw = 0x4A, // 2 operands: RD imm16, multiply RD by immediate 16-bit value (4 byte-long
                    // instruction)
    OP_ANDw = 0x4B, // 2 operands: RD imm16, bitwise AND immediate 16-bit value with RD (4 byte-long
                    // instruction)
    OP_ORw = 0x4C,  // 2 operands: RD imm16, bitwise OR immediate 16-bit value with RD (4 byte-long
                    // instruction)
    OP_XORw = 0x4D, // 2 operands: RD imm16, bitwise XOR immediate 16-bit value with RD (4 byte-long
                    // instruction)
    OP_CMPw = 0x4E, // 2 operands: RD imm16, compare RD with immediate 16-bit value, set flags
                    // accordingly (4 byte-long instruction)

    OP_ADDR = 0x50, // 2 operands: RD RS, add register RS to RD (3 byte-long instruction)
    OP_SUBR = 0x51, // 2 operands: RD RS, subtract register RS from RD (3 byte-long instruction)
    OP_MULR = 0x52, // 2 operands: RD RS, multiply RD by register RS (3 byte-long instruction)
    OP_ANDR = 0x53, // 2 operands: RD RS, bitwise AND register RS with RD (3 byte-long instruction)
    OP_ORR = 0x54,  // 2 operands: RD RS, bitwise OR register RS with RD (3 byte-long instruction)
    OP_XORR = 0x55, // 2 operands: RD RS, bitwise XOR register RS with RD (3 byte-long instruction)
    OP_CMPR = 0x56, // 2 operands: RD RS, compare RD with register RS, set flags accordingly (3
                    // byte-long instruction)

    OP_SHL = 0x58, // 2 operands: RD imm8, logical shift left RD by immediate 8-bit value (3
                   // byte-long instruction)
    OP_SHR = 0x59, // 2 operands: RD imm8, logical shift right RD by immediate 8-bit value (3
                   // byte-long instruction)
    OP_ASR = 0x5A, // 2 operands: RD imm8, arithmetic shift right RD by immediate 8-bit value (3
                   // byte-long instruction)

    OP_INC = 0x5C, // 1 operand: RD, increment RD (2 byte-long instruction)
    OP_DEC = 0x5D, // 1 operand: RD, decrement RD (2 byte-long instruction)

    OP_NOT =
        0x5F, // 1 operand: RD, bitwise NOT RD, ignoring high byte in RD (2 byte-long instruction)

    // =====
    // JUMPS
    // =====

    OP_JMP = 0x70,
};

#endif // OPCODES_H

