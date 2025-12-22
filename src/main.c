#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <string.h>
#include <time.h>

#include "roma.h"   // Include test ROMA data
#include "vga256.h" // Include Color type and VGA256 palette

// ==============================
// VM CONSTANTS
// ==============================

#define REG_COUNT 4 // Number of general purpose registers

#define ROMA_SIZE 65536 // Size of assets ROM
#define ROMB_SIZE 65536 // Size of bytecode/binary ROM
#define RAM_SIZE 65536  // Size of RAM

#define MIMO_BASE 0x0000 // Base address for MIMO memory-mapped I/O
#define MIMO_SIZE 0x0100 // Size of MIMO region

#define MIMO_INPUT_DOWN 0x0000     // MIMO address for input down state
#define MIMO_INPUT_PRESSED 0x0002  // MIMO address for input pressed state
#define MIMO_INPUT_RELEASED 0x0004 // MIMO address for input released state

#define RAM_FB_BASE 0x0100    // Base address for framebuffer in RAM
#define FB_W 180              // Framebuffer width
#define FB_H 136              // Framebuffer height
#define FB_SIZE (FB_W * FB_H) // Framebuffer size

#define RENDER_SCALE 4 // Scale factor for rendering

#define VM_FRAME_DT (1.0f / 24.0f) // Fixed timestep for VM frame updates (24 FPS)

// ==============================
// INPUT BIT LAYOUT (12 buttons)
// ==============================

typedef enum
{
    BTN_UP = 0,
    BTN_DOWN = 1,
    BTN_LEFT = 2,
    BTN_RIGHT = 3,
    BTN_A = 4,
    BTN_B = 5,
    BTN_X = 6,
    BTN_Y = 7,
    BTN_L = 8,
    BTN_R = 9,
    BTN_START = 10,
    BTN_SELECT = 11,
} VmButtonBit;

// ==============================
// VM STRUCT
// ==============================

typedef struct VM
{
    uint16_t ip;             // Instruction pointer, targets romb
    uint16_t sp;             // Stack pointer, targets ram
    uint8_t flags;           // CPU flags: 0000VCNZ (Z=zero, N=negative, C=carry, V=overflow)
    uint16_t reg[REG_COUNT]; // General purpose registers
    uint8_t roma[ROMA_SIZE]; // Assets ROM (contains resources like images, sounds, etc)
    uint8_t romb[ROMB_SIZE]; // Bytecode/binary ROM (contains program code)
    uint8_t ram[RAM_SIZE];   // RAM (data memory)
} VM;

typedef enum
{
    STEP_OK = 0,
    STEP_SYNC = 1,
    STEP_HALT = 2,
    STEP_ERROR = 3,
} StepResult;

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

    // =====
    // JUMPS
    // =====

    OP_JMP = 0x70,
};

// ==============================
// TIME UTILITIES
// ==============================

static double now_seconds(void)
{
    static uint64_t freq = 0;
    if (freq == 0)
    {
        freq = (uint64_t)SDL_GetPerformanceFrequency();
    }
    return (double)SDL_GetPerformanceCounter() / (double)freq;
}

static void sleep_until(double target_time)
{
    for (;;)
    {
        double t = now_seconds();
        double dt = target_time - t;
        if (dt <= 0.0)
        {
            return;
        }

        if (dt > 0.002)
        {
            SDL_Delay((uint32_t)((dt - 0.001) * 1000.0));
        }
        else
        {
            // Busy wait
        }
    }
}

// ==============================
// VM
// ==============================

StepResult vm_step(VM* vm)
{
    uint8_t opcode = vm->romb[vm->ip++];
    switch (opcode)
    {
        // =====
        // OTHER
        // =====

    case OP_NOP:
        break;
    case OP_HLT:
        return STEP_HALT;
    case OP_SYNC:
        return STEP_SYNC;

    case OP_FILL:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rp = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rl = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t addr = vm->reg[ra];
        uint8_t value = (uint8_t)(vm->reg[rp] & 0x00FF);
        uint16_t length = vm->reg[rl];
        for (uint32_t i = 0; i < length; i++)
        {
            uint32_t write_addr = addr + i;
            vm->ram[write_addr] = value;
        }
        break;
    }
    case OP_FILLw:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rp = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rl = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t addr = vm->reg[ra];
        uint16_t value = vm->reg[rp];
        uint16_t length = vm->reg[rl];
        uint8_t low = (uint8_t)(value & 0x00FF);
        uint8_t high = (uint8_t)((value >> 8) & 0x00FF);
        for (uint32_t i = 0; i < length; i++)
        {
            uint32_t write_addr = addr + i * 2;
            vm->ram[write_addr] = low;
            vm->ram[write_addr + 1] = high;
        }
        break;
    }

    case OP_SEED:
    {
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t seed = vm->reg[rs];
        if (seed == 0)
        {
            seed = (uint16_t)(time(NULL) & 0xFFFF);
        }
        srand((uint32_t)seed);
        break;
    }
    case OP_RAND:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t rand_val = (uint16_t)(rand() & 0xFFFF);
        vm->reg[rd] = rand_val;
        break;
    }

        // ===
        // REG
        // ===

    case OP_MOV:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[rd] = vm->reg[rs];
        break;
    }

    case OP_LDI:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[rd] = (uint16_t)imm8;
        break;
    }
    case OP_LDIw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low;
        imm16 |= (uint16_t)(high << 8);
        vm->reg[rd] = imm16;
        break;
    }

        // =====
        // STACK
        // =====

    case OP_PUSHI:
    {
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->ram[vm->sp] = imm8;
        vm->sp--;
        break;
    }
    case OP_PUSHIw:
    {
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        vm->ram[vm->sp] = high;
        vm->sp--;
        vm->ram[vm->sp] = low;
        vm->sp--;
        break;
    }

    case OP_PUSH:
    {
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t val8 = (uint8_t)(vm->reg[rs] & 0x00FF);
        vm->ram[vm->sp] = val8;
        vm->sp--;
        break;
    }
    case OP_PUSHw:
    {
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t val16 = vm->reg[rs];
        uint8_t low = (uint8_t)(val16 & 0x00FF);
        uint8_t high = (uint8_t)((val16 >> 8) & 0x00FF);
        vm->ram[vm->sp] = high;
        vm->sp--;
        vm->ram[vm->sp] = low;
        vm->sp--;
        break;
    }

    case OP_POP:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->sp++;
        uint8_t val8 = vm->ram[vm->sp];
        vm->reg[rd] = (uint16_t)val8;
        break;
    }
    case OP_POPw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->sp++;
        uint8_t low = vm->ram[vm->sp];
        vm->sp++;
        uint8_t high = vm->ram[vm->sp];
        uint16_t val16 = 0x0000;
        val16 |= (uint16_t)low;
        val16 |= (uint16_t)(high << 8);
        vm->reg[rd] = val16;
        break;
    }

    case OP_SWP:
    {
        vm->sp++;
        uint8_t val1 = vm->ram[vm->sp];
        vm->sp++;
        uint8_t val2 = vm->ram[vm->sp];
        vm->ram[vm->sp] = val1;
        vm->sp--;
        vm->ram[vm->sp] = val2;
        vm->sp--;
        break;
    }
    case OP_SWPw:
    {
        vm->sp++;
        uint8_t low1 = vm->ram[vm->sp];
        vm->sp++;
        uint8_t high1 = vm->ram[vm->sp];
        vm->sp++;
        uint8_t low2 = vm->ram[vm->sp];
        vm->sp++;
        uint8_t high2 = vm->ram[vm->sp];
        vm->ram[vm->sp] = high1;
        vm->sp--;
        vm->ram[vm->sp] = low1;
        vm->sp--;
        vm->ram[vm->sp] = high2;
        vm->sp--;
        vm->ram[vm->sp] = low2;
        vm->sp--;
        break;
    }

    case OP_DUP:
    {
        vm->sp++;
        uint8_t val = vm->ram[vm->sp];
        vm->sp--;
        vm->ram[vm->sp] = val;
        vm->sp--;
        break;
    }
    case OP_DUPw:
    {
        vm->sp++;
        uint8_t low = vm->ram[vm->sp];
        vm->sp++;
        uint8_t high = vm->ram[vm->sp];
        vm->sp--;
        vm->sp--;
        vm->ram[vm->sp] = high;
        vm->sp--;
        vm->ram[vm->sp] = low;
        vm->sp--;
        break;
    }

        // ===
        // RAM
        // ===

    case OP_LD:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        uint8_t val8 = vm->ram[addr];
        vm->reg[rd] = (uint16_t)val8;
        break;
    }
    case OP_LDw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        uint8_t low_val = vm->ram[addr];
        uint8_t high_val = vm->ram[addr + 1];
        uint16_t val16 = 0x0000;
        val16 |= (uint16_t)low_val;
        val16 |= (uint16_t)(high_val << 8);
        vm->reg[rd] = val16;
        break;
    }
    case OP_ST:
    {
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        uint8_t val8 = (uint8_t)(vm->reg[rs] & 0x00FF);
        vm->ram[addr] = val8;
        break;
    }
    case OP_STw:
    {
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        uint16_t val16 = vm->reg[rs];
        uint8_t low_val = (uint8_t)(val16 & 0x00FF);
        uint8_t high_val = (uint8_t)((val16 >> 8) & 0x00FF);
        vm->ram[addr] = low_val;
        vm->ram[addr + 1] = high_val;
        break;
    }

    case OP_LDR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t addr = vm->reg[ra];
        uint8_t val8 = vm->ram[addr];
        vm->reg[rd] = (uint16_t)val8;
        break;
    }
    case OP_LDRw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t addr = vm->reg[ra];
        uint8_t low_val = vm->ram[addr];
        uint8_t high_val = vm->ram[addr + 1];
        uint16_t val16 = 0x0000;
        val16 |= (uint16_t)low_val;
        val16 |= (uint16_t)(high_val << 8);
        vm->reg[rd] = val16;
        break;
    }
    case OP_STR:
    {
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t addr = vm->reg[ra];
        uint8_t val8 = (uint8_t)(vm->reg[rs] & 0x00FF);
        vm->ram[addr] = val8;
        break;
    }
    case OP_STRw:
    {
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t addr = vm->reg[ra];
        uint16_t val16 = vm->reg[rs];
        uint8_t low_val = (uint8_t)(val16 & 0x00FF);
        uint8_t high_val = (uint8_t)((val16 >> 8) & 0x00FF);
        vm->ram[addr] = low_val;
        vm->ram[addr + 1] = high_val;
        break;
    }

    case OP_LDS:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->sp++;
        uint8_t low = vm->ram[vm->sp];
        vm->sp++;
        uint8_t high = vm->ram[vm->sp];
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        uint8_t val8 = vm->ram[addr];
        vm->reg[rd] = (uint16_t)val8;
        break;
    }
    case OP_LDSw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->sp++;
        uint8_t low = vm->ram[vm->sp];
        vm->sp++;
        uint8_t high = vm->ram[vm->sp];
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        uint8_t low_val = vm->ram[addr];
        uint8_t high_val = vm->ram[addr + 1];
        uint16_t val16 = 0x0000;
        val16 |= (uint16_t)low_val;
        val16 |= (uint16_t)(high_val << 8);
        vm->reg[rd] = val16;
        break;
    }
    case OP_STS:
    {
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->sp++;
        uint8_t low = vm->ram[vm->sp];
        vm->sp++;
        uint8_t high = vm->ram[vm->sp];
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        uint8_t val8 = (uint8_t)(vm->reg[rs] & 0x00FF);
        vm->ram[addr] = val8;
        break;
    }
    case OP_STSw:
    {
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->sp++;
        uint8_t low = vm->ram[vm->sp];
        vm->sp++;
        uint8_t high = vm->ram[vm->sp];
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        uint16_t val16 = vm->reg[rs];
        uint8_t low_val = (uint8_t)(val16 & 0x00FF);
        uint8_t high_val = (uint8_t)((val16 >> 8) & 0x00FF);
        vm->ram[addr] = low_val;
        vm->ram[addr + 1] = high_val;
        break;
    }

        // ====
        // ROMA
        // ====

    case OP_LDA:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        uint8_t val8 = vm->roma[addr];
        vm->reg[rd] = (uint16_t)val8;
        break;
    }
    case OP_LDAw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        uint8_t low_val = vm->roma[addr];
        uint8_t high_val = vm->roma[addr + 1];
        uint16_t val16 = 0x0000;
        val16 |= (uint16_t)low_val;
        val16 |= (uint16_t)(high_val << 8);
        vm->reg[rd] = val16;
        break;
    }

    case OP_LDAR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t addr = vm->reg[ra];
        uint8_t val8 = vm->roma[addr];
        vm->reg[rd] = (uint16_t)val8;
        break;
    }
    case OP_LDARw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t addr = vm->reg[ra];
        uint8_t low_val = vm->roma[addr];
        uint8_t high_val = vm->roma[addr + 1];
        uint16_t val16 = 0x0000;
        val16 |= (uint16_t)low_val;
        val16 |= (uint16_t)(high_val << 8);
        vm->reg[rd] = val16;
        break;
    }

    case OP_LDAS:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->sp++;
        uint8_t low = vm->ram[vm->sp];
        vm->sp++;
        uint8_t high = vm->ram[vm->sp];
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        uint8_t val8 = vm->roma[addr];
        vm->reg[rd] = (uint16_t)val8;
        break;
    }
    case OP_LDASw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->sp++;
        uint8_t low = vm->ram[vm->sp];
        vm->sp++;
        uint8_t high = vm->ram[vm->sp];
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        uint8_t low_val = vm->roma[addr];
        uint8_t high_val = vm->roma[addr + 1];
        uint16_t val16 = 0x0000;
        val16 |= (uint16_t)low_val;
        val16 |= (uint16_t)(high_val << 8);
        vm->reg[rd] = val16;
        break;
    }

        // =====
        // JUMPS
        // =====

    case OP_JMP:
    {
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        vm->ip = addr;
        break;
    }

        // =====
        // JUMPS
        // =====

        // =====
        // ARITHMETIC
        // =====

    case ADD:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] + imm8;
        break;
    }
    case ADD.w:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low;
        imm16 |= (uint16_t)(high << 8);
        vm->reg[reg_dest] = vm->reg[reg_a] + imm16;
        break;
    }
    case ADDR:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_b = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] + vm->reg[reg_b];
        break;
    }
    case SUB:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] - imm8;
        break;
    }
    case SUB.w:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low;
        imm16 |= (uint16_t)(high << 8);
        vm->reg[reg_dest] = vm->reg[reg_a] - imm16;
        break;
    }
    case SUBR:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_b = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] - vm->reg[reg_b];
        break;
    }
    case MUL:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] * imm8;
        break;
    }
    case MUL.w:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low;
        imm16 |= (uint16_t)(high << 8);
        vm->reg[reg_dest] = vm->reg[reg_a] * imm16;
        break;
    }
    case MULR:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_b = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] * vm->reg[reg_b];
        break;
    }
    // case DIV:
    // {
    //     uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
    //     vm->ip++;
    //     uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
    //     vm->ip++;
    //     uint8_t imm8 = vm->romb[vm->ip];
    //     vm->ip++;
    //     if (imm8 == 0)
    //     {
    //         return STEP_ERROR; // Division by zero
    //     }
    //     vm->reg[reg_dest] = vm->reg[reg_a] / imm8;
    //     break;
    // }
    // case DIV.w:
    // {
    //     uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
    //     vm->ip++;
    //     uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
    //     vm->ip++;
    //     uint8_t low = vm->romb[vm->ip];
    //     vm->ip++;
    //     uint8_t high = vm->romb[vm->ip];
    //     vm->ip++;
    //     uint16_t imm16 = 0x0000;
    //     imm16 |= (uint16_t)low;
    //     imm16 |= (uint16_t)(high << 8);
    //     if (imm16 == 0)
    //     {
    //         return STEP_ERROR; // Division by zero
    //     }
    //     vm->reg[reg_dest] = vm->reg[reg_a] / imm16;
    //     break;
    // }
    // case DIVR:
    // {
    //     uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
    //     vm->ip++;
    //     uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
    //     vm->ip++;
    //     uint8_t reg_b = vm->romb[vm->ip] % REG_COUNT;
    //     vm->ip++;
    //     if (vm->reg[reg_b] == 0)
    //     {
    //         return STEP_ERROR; // Division by zero
    //     }
    //     vm->reg[reg_dest] = vm->reg[reg_a] / vm->reg[reg_b];
    //     break;
    // }
    case CMP:
    {
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        uint8_t result = vm->reg[reg_a] - imm8;
        if (result == 0)
            vm->flags |= FLAG_ZERO;
        else
            vm->flags &= ~FLAG_ZERO;
        if ((result & 0x80) != 0)
            vm->flags |= FLAG_NEGATIVE;
        else
            vm->flags &= ~FLAG_NEGATIVE;
        break;
    }
    case CMP.w:
    {
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low;
        imm16 |= (uint16_t)(high << 8);
        uint16_t result = vm->reg[reg_a] - imm16;
        if (result == 0)
            vm->flags |= FLAG_ZERO;
        else
            vm->flags &= ~FLAG_ZERO;
        if ((result & 0x8000) != 0)
            vm->flags |= FLAG_NEGATIVE;
        else
            vm->flags &= ~FLAG_NEGATIVE;
        break;
    }
    case CMPR:
    {
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_b = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t result = vm->reg[reg_a] - vm->reg[reg_b];
        if (result == 0)
            vm->flags |= FLAG_ZERO;
        else
            vm->flags &= ~FLAG_ZERO;
        if ((result & 0x80) != 0)
            vm->flags |= FLAG_NEGATIVE;
        else
            vm->flags &= ~FLAG_NEGATIVE;
        break;
    }
    case INC:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_dest] + 1;
        break;
    }
    case DEC:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_dest] - 1;
        break;
    }

        // =====
        // ARITHMETIC
        // =====

        // =====
        // LOGIC
        // =====

    case AND:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] & imm8;
        break;
    }
    case AND.w:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low;
        imm16 |= (uint16_t)(high << 8);
        vm->reg[reg_dest] = vm->reg[reg_a] & imm16;
        break;
    }
    case ANDR:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_b = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] & vm->reg[reg_b];
        break;
    }
    case OR:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] | imm8;
        break;
    }
    case OR.w:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low;
        imm16 |= (uint16_t)(high << 8);
        vm->reg[reg_dest] = vm->reg[reg_a] | imm16;
        break;
    }
    case ORR:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_b = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] | vm->reg[reg_b];
        break;
    }
    case XOR:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] ^ imm8;
        break;
    }
    case XOR.w:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low;
        imm16 |= (uint16_t)(high << 8);
        vm->reg[reg_dest] = vm->reg[reg_a] ^ imm16;
        break;
    }
    case XORR:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_b = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] ^ vm->reg[reg_b];
        break;
    }
    case SHL:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] << imm8;
        break;
    }
    case SHR:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[reg_dest] = vm->reg[reg_a] >> imm8;
        break;
    }
    case ASR:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_a = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        int16_t val = (int16_t)vm->reg[reg_a];
        vm->reg[reg_dest] = (uint16_t)(val >> imm8);
        break;
    }

    case NOT:
    {
        uint8_t reg_dest = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[reg_dest] = ~vm->reg[reg_dest];
        break;
    }

        // =====
        // UNKNOWN OPCODE
        // =====

    default:
        return STEP_ERROR;
    }

    return STEP_OK;
}

// ==============================
// DRAW UTILITIES
// ==============================

void draw_sprite(VM* vm, int sprite_start_x, int sprite_start_y, int sprite_id, int sprite_w,
                 int sprite_h)
{
    int sprite_count = (int)(sizeof(sprites) / sizeof(sprites[0]));
    if (sprite_id >= sprite_count)
        sprite_id = 0; // use empty sprite for invalid id

    for (int sprite_y = 0; sprite_y < sprite_h; sprite_y++)
    {
        for (int sprite_x = 0; sprite_x < sprite_w; sprite_x++)
        {
            uint8_t pixel = sprites[sprite_id][sprite_y][sprite_x];
            if (pixel == 0)
                continue; // transparent

            int pixel_to_paint_x = sprite_start_x + sprite_x;
            int pixel_to_paint_y = sprite_start_y + sprite_y;

            if (pixel_to_paint_x < 0 || pixel_to_paint_y < 0 || pixel_to_paint_x >= FB_W ||
                pixel_to_paint_y >= FB_H)
                continue;

            int fb_index = RAM_FB_BASE + pixel_to_paint_y * FB_W + pixel_to_paint_x;

            vm->ram[fb_index] = pixel; // pixel = palette index
        }
    }
}

// ==============================
// MAIN
// ==============================

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow(
        "Fantasy VM (SDL2)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, FB_W * RENDER_SCALE,
        FB_H * RENDER_SCALE, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!win)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* ren =
        SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren)
    {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* tex =
        SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FB_W, FB_H);
    if (!tex)
    {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    uint32_t* scratch_rgba = (uint32_t*)SDL_malloc((size_t)FB_SIZE * sizeof(uint32_t));
    if (!scratch_rgba)
    {
        fprintf(stderr, "Out of memory for scratch_rgba\n");
        SDL_DestroyTexture(tex);
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    VM vm;
    memset(&vm, 0, sizeof(VM));
    vm.ip = 0x0000;
    vm.sp = 0xFFFF;
    // TODO: Load ROMA and ROMB from some source
    // For now, just fill ROMB with a simple program that does nothing
    vm.romb[0x0000] = OP_SYNC;
    vm.romb[0x0001] = OP_JMP;
    vm.romb[0x0002] = 0x00;
    vm.romb[0x0003] = 0x00;

    bool running = true;
    double next_frame_time = now_seconds() + VM_FRAME_DT;

    while (running)
    {
        while (true)
        {
            StepResult res = vm_step(&vm);
            if (res == STEP_HALT)
            {
                running = false;
                break;
            }
            if (res == STEP_SYNC)
            {
                break;
            }
            if (res == STEP_ERROR)
            {
                fprintf(stderr, "VM encountered an error during execution\n");
                running = false;
                break;
            }
        }
        if (!running)
        {
            break;
        }

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = false;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
            {
                running = false;
            }
        }
        if (!running)
        {
            break;
        }

        // TODO: Update input states in MIMO region of RAM
        // TODO: Render framebuffer from RAM to scratch_rgba
        // TODO: Render scratch_rgba to texture and present

        // ==============================
        // buffer rendering demo code
        // ==============================

        // memset(&vm.ram[RAM_FB_BASE], 0, FB_SIZE); // Clear framebuffer for demonstration

        // for (int i = 0; i < FB_SIZE; i++)
        // {
        //     vm.ram[RAM_FB_BASE + i] = 1;
        // }

        // for (int i = 0; i < FB_SIZE; i++)
        // {
        //     uint8_t color_id = vm.ram[RAM_FB_BASE + i];

        //     Color col = palette[color_id];
        //     scratch_rgba[i] = (0xFF << 24) | (col.r << 16) | (col.g << 8) | (col.b); // ARGB
        //     format
        // }

        // ==============================
        // drawing sprite RU_I demo code
        // ==============================

        memset(&vm.ram[RAM_FB_BASE], 0, FB_SIZE); // Clear framebuffer for demonstration

        uint16_t offset = 1;

        draw_sprite(&vm, offset, 1, 1, 8, 8); // Draw sprite RU_I
        offset += 8;
        draw_sprite(&vm, offset, 1, 2, 8, 8); // Draw sprite RU_L
        offset += 8;
        draw_sprite(&vm, offset, 1, 3, 8, 8); // Draw sprite RU_U
        offset += 8;
        draw_sprite(&vm, offset, 1, 4, 8, 8); // Draw sprite RU_SH
        offset += 8;
        draw_sprite(&vm, offset, 1, 5, 8, 8); // Draw sprite RU_A
        offset += 16;

        draw_sprite(&vm, offset, 1, 4, 8, 8); // Draw sprite RU_SH
        offset += 8;
        draw_sprite(&vm, offset, 1, 2, 8, 8); // Draw sprite RU_L
        offset += 8;
        draw_sprite(&vm, offset, 1, 3, 8, 8); // Draw sprite RU_U
        offset += 8;
        draw_sprite(&vm, offset, 1, 4, 8, 8); // Draw sprite RU_SH
        offset += 8;
        draw_sprite(&vm, offset, 1, 5, 8, 8); // Draw sprite RU_A

        for (int i = 0; i < FB_SIZE; i++)
        {
            uint8_t color_id = vm.ram[RAM_FB_BASE + i];

            Color col = palette[color_id];
            scratch_rgba[i] =
                (uint32_t)((0xFF << 24) | (col.r << 16) | (col.g << 8) | (col.b)); // ARGB format
        }

        SDL_UpdateTexture(tex, NULL, scratch_rgba, FB_W * sizeof(uint32_t));
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);

        sleep_until(next_frame_time);
        next_frame_time += VM_FRAME_DT;

        double t = now_seconds();
        if (t > next_frame_time + VM_FRAME_DT)
        {
            next_frame_time = t + VM_FRAME_DT;
        }
    }

    SDL_free(scratch_rgba);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

