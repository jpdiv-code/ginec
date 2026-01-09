#include "vm.h"

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "opcodes.h"

#define FLAG_ZERO 0x01u     // Z 0b0000001
#define FLAG_NEGATIVE 0x02u // N 0b0000010
#define FLAG_CARRY 0x04u    // C 0b0000100
#define FLAG_OVERFLOW 0x08u // V 0b0001000

#define MAX_BYTE_SHIFT 8
#define MAX_WORD_SHIFT 16

static inline void set_flags_NZ(VM* vm, uint32_t res, uint16_t byte_mask)
{
    uint16_t sign_bit =
        (byte_mask >> 1) + 1; // 0xFF -> 0x80 (0b10000000), 0xFFFF -> 0x8000 (0b10000000_00000000)

    if ((res & sign_bit) != 0)
    {
        vm->flags |= FLAG_NEGATIVE;
    }
    else
    {
        vm->flags &= (uint8_t)~FLAG_NEGATIVE;
    }

    if ((res & byte_mask) == 0)
    {
        vm->flags |= FLAG_ZERO;
    }
    else
    {
        vm->flags &= (uint8_t)~FLAG_ZERO;
    }
}

static inline void set_flags_on_ADD(VM* vm, uint16_t a, uint16_t b, uint32_t res,
                                    uint16_t byte_mask)
{
    if (res > byte_mask)
    {
        vm->flags |= FLAG_CARRY;
    }
    else
    {
        vm->flags &= (uint8_t)~FLAG_CARRY;
    }

    uint16_t sign_bit =
        (byte_mask >> 1) + 1; // 0xFF -> 0x80 (0b10000000), 0xFFFF -> 0x8000 (0b10000000_00000000)

    uint16_t a_sign = a & sign_bit;
    uint16_t b_sign = b & sign_bit;
    uint16_t r_sign = (uint16_t)res & sign_bit;

    if ((a_sign == b_sign) && (a_sign != r_sign))
    {
        vm->flags |= FLAG_OVERFLOW;
    }
    else
    {
        vm->flags &= (uint8_t)~FLAG_OVERFLOW;
    }

    set_flags_NZ(vm, res, byte_mask);
}

static inline void set_flags_on_SUB(VM* vm, uint16_t a, uint16_t b, uint32_t res,
                                    uint16_t byte_mask)
{
    if (a < b)
    {
        vm->flags |= FLAG_CARRY;
    }
    else
    {
        vm->flags &= (uint8_t)~FLAG_CARRY;
    }

    uint16_t sign_bit =
        (byte_mask >> 1) + 1; // 0xFF -> 0x80 (0b10000000), 0xFFFF -> 0x8000 (0b10000000_00000000)

    uint16_t a_sign = a & sign_bit;
    uint16_t b_sign = b & sign_bit;
    uint16_t r_sign = (uint16_t)res & sign_bit;

    if ((a_sign != b_sign) && (a_sign != r_sign))
    {
        vm->flags |= FLAG_OVERFLOW;
    }
    else
    {
        vm->flags &= (uint8_t)~FLAG_OVERFLOW;
    }

    set_flags_NZ(vm, res, byte_mask);
}

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
        uint8_t rand_val = (uint8_t)(rand() & 0xFF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (rand_val & 0x00FF);
        break;
    }
    case OP_RANDw:
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
        uint8_t low = (uint8_t)(vm->reg[rs] & 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (low & 0x00FF);
        break;
    }
    case OP_MOVw:
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
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (imm8 & 0x00FF);
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
    case OP_SRB:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint8_t high = (uint8_t)((vm->reg[rd] >> 8) & 0x00FF);
        uint16_t swapped = 0x0000;
        swapped |= (uint16_t)low << 8;
        swapped |= (uint16_t)high;
        vm->reg[rd] = swapped;
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
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (val8 & 0x00FF);
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

    case OP_ADJSP:
    {
        int8_t imm8 = (int8_t)vm->romb[vm->ip];
        vm->ip++;
        vm->sp = (uint16_t)((int32_t)vm->sp + imm8);
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
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (val8 & 0x00FF);
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
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
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
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
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
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (val8 & 0x00FF);
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
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t addr = vm->reg[ra];
        uint8_t val8 = (uint8_t)(vm->reg[rs] & 0x00FF);
        vm->ram[addr] = val8;
        break;
    }
    case OP_STRw:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t addr = vm->reg[ra];
        uint16_t val16 = vm->reg[rs];
        uint8_t low_val = (uint8_t)(val16 & 0x00FF);
        uint8_t high_val = (uint8_t)((val16 >> 8) & 0x00FF);
        vm->ram[addr] = low_val;
        vm->ram[addr + 1] = high_val;
        break;
    }

    case OP_LDRI:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        int8_t off8 = (int8_t)vm->romb[vm->ip];
        vm->ip++;
        uint16_t addr = (uint16_t)(vm->reg[ra] + off8);
        uint8_t val8 = vm->ram[addr];
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (val8 & 0x00FF);
        break;
    }
    case OP_LDRIw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        int8_t off8 = (int8_t)vm->romb[vm->ip];
        vm->ip++;
        uint16_t addr = (uint16_t)(vm->reg[ra] + off8);
        uint8_t low_val = vm->ram[addr];
        uint8_t high_val = vm->ram[addr + 1];
        uint16_t val16 = 0x0000;
        val16 |= (uint16_t)low_val;
        val16 |= (uint16_t)(high_val << 8);
        vm->reg[rd] = val16;
        break;
    }
    case OP_STRI:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        int8_t off8 = (int8_t)vm->romb[vm->ip];
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t addr = (uint16_t)(vm->reg[ra] + off8);
        uint8_t val8 = (uint8_t)(vm->reg[rs] & 0x00FF);
        vm->ram[addr] = val8;
        break;
    }
    case OP_STRIw:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        int8_t off8 = (int8_t)vm->romb[vm->ip];
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t addr = (uint16_t)(vm->reg[ra] + off8);
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
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (val8 & 0x00FF);
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
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (val8 & 0x00FF);
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

    case OP_CPYRA:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t count = vm->romb[vm->ip];
        vm->ip++;

        uint16_t ram_addr = vm->reg[rd];
        uint16_t roma_addr = vm->reg[rs];

        for (uint16_t i = 0; i < count; i++)
        {
            vm->ram[ram_addr + i] = vm->roma[roma_addr + i];
        }
        break;
    }
    case OP_SPRRA:
    {
        uint8_t rc = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t width = vm->romb[vm->ip];
        vm->ip++;
        uint8_t height = vm->romb[vm->ip];
        vm->ip++;

        uint8_t x = (uint8_t)(vm->reg[rc] & 0x00FF);
        uint8_t y = (uint8_t)((vm->reg[rc] >> 8) & 0x00FF);
        uint16_t sprite_addr = vm->reg[ra];

        // Render sprite to framebuffer (starts at 0x0100, size 180x136)
        const uint16_t fb_start = RAM_FB_BASE;
        const uint16_t fb_width = FB_W;
        const uint16_t fb_height = FB_H;

        for (uint16_t row = 0; row < height; row++)
        {
            uint16_t screen_y = y + row;
            if (screen_y >= fb_height)
                break;

            for (uint16_t col = 0; col < width; col++)
            {
                uint16_t screen_x = x + col;
                if (screen_x >= fb_width)
                    continue;

                uint16_t fb_addr = fb_start + (screen_y * fb_width) + screen_x;
                uint16_t sprite_offset = (row * width) + col;
                uint16_t roma_addr = sprite_addr + sprite_offset;

                vm->ram[fb_addr] = vm->roma[roma_addr];
            }
        }
        break;
    }

        // =====
        // ALU
        // =====

    case OP_ADD:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        uint8_t low = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint16_t res16 = (uint16_t)low + (uint16_t)imm8;
        set_flags_on_ADD(vm, (uint16_t)low, (uint16_t)imm8, (uint32_t)res16, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res16 & 0x00FF);
        break;
    }
    case OP_SUB:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        uint8_t low = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint16_t res16 = (uint16_t)low - (uint16_t)imm8;
        set_flags_on_SUB(vm, (uint16_t)low, (uint16_t)imm8, (uint32_t)res16, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res16 & 0x00FF);
        break;
    }
    case OP_MUL:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        uint8_t low = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint16_t res16 = (uint16_t)low * (uint16_t)imm8;
        set_flags_NZ(vm, (uint32_t)res16, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res16 & 0x00FF);
        break;
    }
    case OP_AND:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        uint8_t low = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint8_t res8 = low & imm8;
        set_flags_NZ(vm, (uint32_t)res8, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res8 & 0x00FF);
        break;
    }
    case OP_OR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        uint8_t low = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint8_t res8 = low | imm8;
        set_flags_NZ(vm, (uint32_t)res8, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res8 & 0x00FF);
        break;
    }
    case OP_XOR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        uint8_t low = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint8_t res8 = low ^ imm8;
        set_flags_NZ(vm, (uint32_t)res8, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res8 & 0x00FF);
        break;
    }
    case OP_CMP:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        uint8_t low = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint16_t res16 = (uint16_t)low - (uint16_t)imm8;
        set_flags_on_SUB(vm, (uint16_t)low, (uint16_t)imm8, (uint32_t)res16, 0x00FF);
        break;
    }

    case OP_ADDw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low_imm = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high_imm = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low_imm;
        imm16 |= (uint16_t)(high_imm << 8);
        uint16_t a = vm->reg[rd];
        uint32_t res32 = (uint32_t)a + (uint32_t)imm16;
        set_flags_on_ADD(vm, a, imm16, res32, 0xFFFF);
        vm->reg[rd] = (uint16_t)(res32 & 0xFFFF);
        break;
    }
    case OP_SUBw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low_imm = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high_imm = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low_imm;
        imm16 |= (uint16_t)(high_imm << 8);
        uint16_t a = vm->reg[rd];
        uint32_t res32 = (uint32_t)a - (uint32_t)imm16;
        set_flags_on_SUB(vm, a, imm16, res32, 0xFFFF);
        vm->reg[rd] = (uint16_t)(res32 & 0xFFFF);
        break;
    }
    case OP_MULw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low_imm = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high_imm = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low_imm;
        imm16 |= (uint16_t)(high_imm << 8);
        uint16_t a = vm->reg[rd];
        uint32_t res32 = (uint32_t)a * (uint32_t)imm16;
        set_flags_NZ(vm, res32, 0xFFFF);
        vm->reg[rd] = (uint16_t)(res32 & 0xFFFF);
        break;
    }
    case OP_ANDw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low_imm = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high_imm = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low_imm;
        imm16 |= (uint16_t)(high_imm << 8);
        uint16_t a = vm->reg[rd];
        uint16_t res16 = a & imm16;
        set_flags_NZ(vm, (uint32_t)res16, 0xFFFF);
        vm->reg[rd] = res16;
        break;
    }
    case OP_ORw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low_imm = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high_imm = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low_imm;
        imm16 |= (uint16_t)(high_imm << 8);
        uint16_t a = vm->reg[rd];
        uint16_t res16 = a | imm16;
        set_flags_NZ(vm, (uint32_t)res16, 0xFFFF);
        vm->reg[rd] = res16;
        break;
    }
    case OP_XORw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low_imm = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high_imm = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low_imm;
        imm16 |= (uint16_t)(high_imm << 8);
        uint16_t a = vm->reg[rd];
        uint16_t res16 = a ^ imm16;
        set_flags_NZ(vm, (uint32_t)res16, 0xFFFF);
        vm->reg[rd] = res16;
        break;
    }
    case OP_CMPw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low_imm = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high_imm = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low_imm;
        imm16 |= (uint16_t)(high_imm << 8);
        uint16_t a = vm->reg[rd];
        uint32_t res32 = (uint32_t)a - (uint32_t)imm16;
        set_flags_on_SUB(vm, a, imm16, res32, 0xFFFF);
        break;
    }

    case OP_ADDR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t a = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint8_t b = (uint8_t)(vm->reg[rs] & 0x00FF);
        uint16_t res16 = (uint16_t)a + (uint16_t)b;
        set_flags_on_ADD(vm, (uint16_t)a, (uint16_t)b, (uint32_t)res16, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res16 & 0x00FF);
        break;
    }
    case OP_SUBR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t a = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint8_t b = (uint8_t)(vm->reg[rs] & 0x00FF);
        uint16_t res16 = (uint16_t)a - (uint16_t)b;
        set_flags_on_SUB(vm, (uint16_t)a, (uint16_t)b, (uint32_t)res16, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res16 & 0x00FF);
        break;
    }
    case OP_MULR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t a = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint8_t b = (uint8_t)(vm->reg[rs] & 0x00FF);
        uint16_t res16 = (uint16_t)a * (uint16_t)b;
        set_flags_NZ(vm, (uint32_t)res16, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res16 & 0x00FF);
        break;
    }
    case OP_ANDR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t a = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint8_t b = (uint8_t)(vm->reg[rs] & 0x00FF);
        uint8_t res8 = a & b;
        set_flags_NZ(vm, (uint32_t)res8, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res8 & 0x00FF);
        break;
    }
    case OP_ORR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t a = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint8_t b = (uint8_t)(vm->reg[rs] & 0x00FF);
        uint8_t res8 = a | b;
        set_flags_NZ(vm, (uint32_t)res8, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res8 & 0x00FF);
        break;
    }
    case OP_XORR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t a = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint8_t b = (uint8_t)(vm->reg[rs] & 0x00FF);
        uint8_t res8 = a ^ b;
        set_flags_NZ(vm, (uint32_t)res8, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res8 & 0x00FF);
        break;
    }
    case OP_CMPR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t a = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint8_t b = (uint8_t)(vm->reg[rs] & 0x00FF);
        uint16_t res16 = (uint16_t)a - (uint16_t)b;
        set_flags_on_SUB(vm, (uint16_t)a, (uint16_t)b, (uint32_t)res16, 0x00FF);
        break;
    }

    case OP_ADDRw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t a = vm->reg[rd];
        uint16_t b = vm->reg[rs];
        uint32_t res32 = (uint32_t)a + (uint32_t)b;
        set_flags_on_ADD(vm, a, b, res32, 0xFFFF);
        vm->reg[rd] = (uint16_t)(res32 & 0xFFFF);
        break;
    }
    case OP_SUBRw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t a = vm->reg[rd];
        uint16_t b = vm->reg[rs];
        uint32_t res32 = (uint32_t)a - (uint32_t)b;
        set_flags_on_SUB(vm, a, b, res32, 0xFFFF);
        vm->reg[rd] = (uint16_t)(res32 & 0xFFFF);
        break;
    }
    case OP_MULRw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t a = vm->reg[rd];
        uint16_t b = vm->reg[rs];
        uint32_t res32 = (uint32_t)a * (uint32_t)b;
        set_flags_NZ(vm, res32, 0xFFFF);
        vm->reg[rd] = (uint16_t)(res32 & 0xFFFF);
        break;
    }
    case OP_ANDRw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t a = vm->reg[rd];
        uint16_t b = vm->reg[rs];
        uint16_t res16 = a & b;
        set_flags_NZ(vm, (uint32_t)res16, 0xFFFF);
        vm->reg[rd] = res16;
        break;
    }
    case OP_ORRw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t a = vm->reg[rd];
        uint16_t b = vm->reg[rs];
        uint16_t res16 = a | b;
        set_flags_NZ(vm, (uint32_t)res16, 0xFFFF);
        vm->reg[rd] = res16;
        break;
    }
    case OP_XORRw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t a = vm->reg[rd];
        uint16_t b = vm->reg[rs];
        uint16_t res16 = a ^ b;
        set_flags_NZ(vm, (uint32_t)res16, 0xFFFF);
        vm->reg[rd] = res16;
        break;
    }
    case OP_CMPRw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t a = vm->reg[rd];
        uint16_t b = vm->reg[rs];
        uint32_t res32 = (uint32_t)a - (uint32_t)b;
        set_flags_on_SUB(vm, a, b, res32, 0xFFFF);
        break;
    }

    case OP_INC:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint16_t res16 = (uint16_t)low + 1;
        set_flags_on_ADD(vm, (uint16_t)low, 1, (uint32_t)res16, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res16 & 0x00FF);
        break;
    }
    case OP_INCw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t a = vm->reg[rd];
        uint32_t res32 = (uint32_t)a + 1;
        set_flags_on_ADD(vm, a, 1, res32, 0xFFFF);
        vm->reg[rd] = (uint16_t)(res32 & 0xFFFF);
        break;
    }
    case OP_DEC:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint16_t res16 = (uint16_t)low - 1;
        set_flags_on_SUB(vm, (uint16_t)low, 1, (uint32_t)res16, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res16 & 0x00FF);
        break;
    }
    case OP_DECw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t a = vm->reg[rd];
        uint32_t res32 = (uint32_t)a - 1;
        set_flags_on_SUB(vm, a, 1, res32, 0xFFFF);
        vm->reg[rd] = (uint16_t)(res32 & 0xFFFF);
        break;
    }

    case OP_NOT:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint8_t res8 = ~low;
        set_flags_NZ(vm, (uint32_t)res8, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res8 & 0x00FF);
        break;
    }
    case OP_NOTw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t a = vm->reg[rd];
        uint16_t res16 = ~a;
        set_flags_NZ(vm, (uint32_t)res16, 0xFFFF);
        vm->reg[rd] = res16;
        break;
    }
    case OP_SHL:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t shift = vm->romb[vm->ip] % MAX_BYTE_SHIFT;
        vm->ip++;
        uint8_t low = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint16_t res16 = (uint16_t)((uint16_t)low << shift);
        set_flags_NZ(vm, (uint32_t)res16, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res16 & 0x00FF);
        break;
    }
    case OP_SHLw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t shift = vm->romb[vm->ip] % MAX_WORD_SHIFT;
        vm->ip++;
        uint16_t a = vm->reg[rd];
        uint32_t res32 = (uint32_t)a << shift;
        set_flags_NZ(vm, res32, 0xFFFF);
        vm->reg[rd] = (uint16_t)(res32 & 0xFFFF);
        break;
    }
    case OP_SHR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t shift = vm->romb[vm->ip] % MAX_BYTE_SHIFT;
        vm->ip++;
        uint8_t low = (uint8_t)(vm->reg[rd] & 0x00FF);
        uint16_t res16 = (uint16_t)low >> shift;
        set_flags_NZ(vm, (uint32_t)res16, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | (res16 & 0x00FF);
        break;
    }
    case OP_SHRw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t shift = vm->romb[vm->ip] % MAX_WORD_SHIFT;
        vm->ip++;
        uint16_t a = vm->reg[rd];
        uint32_t res32 = (uint32_t)a >> shift;
        set_flags_NZ(vm, res32, 0xFFFF);
        vm->reg[rd] = (uint16_t)(res32 & 0xFFFF);
        break;
    }
    case OP_ASR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t shift = vm->romb[vm->ip] % MAX_BYTE_SHIFT;
        vm->ip++;
        int8_t low = (int8_t)(vm->reg[rd] & 0x00FF);
        int16_t res16 = (int16_t)low >> shift;
        set_flags_NZ(vm, (uint32_t)res16, 0x00FF);
        vm->reg[rd] = (vm->reg[rd] & 0xFF00) | ((uint16_t)res16 & 0x00FF);
        break;
    }
    case OP_ASRw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t shift = vm->romb[vm->ip] % MAX_WORD_SHIFT;
        vm->ip++;
        int16_t a = (int16_t)(vm->reg[rd]);
        int32_t res32 = (int32_t)a >> shift;
        set_flags_NZ(vm, (uint32_t)res32, 0xFFFF);
        vm->reg[rd] = (uint16_t)(res32 & 0xFFFF);
        break;
    }

    case OP_SEXT8w:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        int8_t low = (int8_t)(vm->reg[rd] & 0x00FF);
        vm->reg[rd] = (uint16_t)(int16_t)low;
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
    case OP_JZ:
    {
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        if (vm->flags & FLAG_ZERO)
        {
            uint16_t addr = 0x0000;
            addr |= (uint16_t)low;
            addr |= (uint16_t)(high << 8);
            vm->ip = addr;
        }
        break;
    }
    case OP_JNZ:
    {
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        if (!(vm->flags & FLAG_ZERO))
        {
            uint16_t addr = 0x0000;
            addr |= (uint16_t)low;
            addr |= (uint16_t)(high << 8);
            vm->ip = addr;
        }
        break;
    }
    case OP_JC:
    {
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        if (vm->flags & FLAG_CARRY)
        {
            uint16_t addr = 0x0000;
            addr |= (uint16_t)low;
            addr |= (uint16_t)(high << 8);
            vm->ip = addr;
        }
        break;
    }
    case OP_JNC:
    {
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        if (!(vm->flags & FLAG_CARRY))
        {
            uint16_t addr = 0x0000;
            addr |= (uint16_t)low;
            addr |= (uint16_t)(high << 8);
            vm->ip = addr;
        }
        break;
    }
    case OP_JLT:
    {
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint8_t n = (vm->flags & FLAG_NEGATIVE) ? 1 : 0;
        uint8_t v = (vm->flags & FLAG_OVERFLOW) ? 1 : 0;
        if (n != v)
        {
            uint16_t addr = 0x0000;
            addr |= (uint16_t)low;
            addr |= (uint16_t)(high << 8);
            vm->ip = addr;
        }
        break;
    }
    case OP_JGE:
    {
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint8_t n = (vm->flags & FLAG_NEGATIVE) ? 1 : 0;
        uint8_t v = (vm->flags & FLAG_OVERFLOW) ? 1 : 0;
        if (n == v)
        {
            uint16_t addr = 0x0000;
            addr |= (uint16_t)low;
            addr |= (uint16_t)(high << 8);
            vm->ip = addr;
        }
        break;
    }

    case OP_JMPR:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->ip = vm->reg[ra];
        break;
    }
    case OP_JZR:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        if (vm->flags & FLAG_ZERO)
        {
            vm->ip = vm->reg[ra];
        }
        break;
    }
    case OP_JNZR:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        if (!(vm->flags & FLAG_ZERO))
        {
            vm->ip = vm->reg[ra];
        }
        break;
    }
    case OP_JCR:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        if (vm->flags & FLAG_CARRY)
        {
            vm->ip = vm->reg[ra];
        }
        break;
    }
    case OP_JNCR:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        if (!(vm->flags & FLAG_CARRY))
        {
            vm->ip = vm->reg[ra];
        }
        break;
    }
    case OP_JLTR:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t n = (vm->flags & FLAG_NEGATIVE) ? 1 : 0;
        uint8_t v = (vm->flags & FLAG_OVERFLOW) ? 1 : 0;
        if (n != v)
        {
            vm->ip = vm->reg[ra];
        }
        break;
    }
    case OP_JGER:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t n = (vm->flags & FLAG_NEGATIVE) ? 1 : 0;
        uint8_t v = (vm->flags & FLAG_OVERFLOW) ? 1 : 0;
        if (n == v)
        {
            vm->ip = vm->reg[ra];
        }
        break;
    }

        // =====
        // CALL STACK
        // =====

    case OP_PUSHCw:
    {
        uint8_t rs = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t value = vm->reg[rs];
        uint8_t low = (uint8_t)(value & 0x00FF);
        uint8_t high = (uint8_t)((value >> 8) & 0x00FF);
        vm->ram[vm->csp] = high;
        vm->csp--;
        vm->ram[vm->csp] = low;
        vm->csp--;
        break;
    }
    case OP_POPCw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->csp++;
        uint8_t low = vm->ram[vm->csp];
        vm->csp++;
        uint8_t high = vm->ram[vm->csp];
        uint16_t value = (uint16_t)(low | (high << 8));
        vm->reg[rd] = value;
        break;
    }
    case OP_ADJCSP:
    {
        int8_t offset = (int8_t)vm->romb[vm->ip];
        vm->ip++;
        vm->csp = (uint16_t)((int32_t)vm->csp + (int32_t)offset);
        break;
    }

        // =====
        // CALLS
        // =====

    case OP_CALL:
    {
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        uint16_t ret_addr = vm->ip;
        uint8_t ret_low = (uint8_t)(ret_addr & 0x00FF);
        uint8_t ret_high = (uint8_t)((ret_addr >> 8) & 0x00FF);
        vm->ram[vm->csp] = ret_high;
        vm->csp--;
        vm->ram[vm->csp] = ret_low;
        vm->csp--;
        vm->ip = addr;
        break;
    }
    case OP_CALLR:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t addr = vm->reg[ra];
        uint16_t ret_addr = vm->ip;
        uint8_t ret_low = (uint8_t)(ret_addr & 0x00FF);
        uint8_t ret_high = (uint8_t)((ret_addr >> 8) & 0x00FF);
        vm->ram[vm->csp] = ret_high;
        vm->csp--;
        vm->ram[vm->csp] = ret_low;
        vm->csp--;
        vm->ip = addr;
        break;
    }
    case OP_RET:
    {
        vm->csp++;
        uint8_t ret_low = vm->ram[vm->csp];
        vm->csp++;
        uint8_t ret_high = vm->ram[vm->csp];
        uint16_t ret_addr = 0x0000;
        ret_addr |= (uint16_t)ret_low;
        ret_addr |= (uint16_t)(ret_high << 8);
        vm->ip = ret_addr;
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

