#include "vm.h"

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "opcodes.h"

#define FLAG_ZERO 0x01u     // Z
#define FLAG_NEGATIVE 0x02u // N
#define FLAG_CARRY 0x04u    // C
#define FLAG_OVERFLOW 0x08u // V

void set_flags_NZ(VM* vm, uint32_t res, uint32_t byte_mask)
{
    if ((res & ((byte_mask + 1u) >> 1)) != 0)
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

void set_flags_on_ADD(VM* vm, uint32_t a, uint32_t b, uint32_t res, uint32_t byte_mask)
{
    if (res > byte_mask)
    {
        vm->flags |= FLAG_CARRY;
    }
    else
    {
        vm->flags &= (uint8_t)~FLAG_CARRY;
    }

    if (((~(a ^ b)) & (a ^ res) & byte_mask) != 0)
    {
        vm->flags |= FLAG_OVERFLOW;
    }
    else
    {
        vm->flags &= (uint8_t)~FLAG_OVERFLOW;
    }

    set_flags_NZ(vm, res, byte_mask);
}

void set_flags_on_SUB(VM* vm, uint32_t a, uint32_t b, uint32_t res, uint32_t byte_mask)
{
    if (a < b)
    {
        vm->flags |= FLAG_CARRY;
    }
    else
    {
        vm->flags &= (uint8_t)~FLAG_CARRY;
    }

    if (((a ^ b) & (a ^ res) & byte_mask) != 0)
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
        // ARITHMETIC
        // =====

    case OP_ADD:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        uint16_t res16 = (uint8_t)(vm->reg[rd] & 0xFF) + imm8;
        set_flags_on_ADD(vm, vm->reg[rd], imm8, res16, 0xFF);
        vm->reg[rd] &= 0xFF00;
        vm->reg[rd] |= (uint8_t)res16;
        break;
    }
    case OP_ADDw:
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
        uint32_t res32 = vm->reg[rd] + imm16;
        vm->reg[rd] = (uint16_t)(res32 & 0xFFFF);
        set_flags_on_ADD(vm, vm->reg[rd], imm16, res32, 0xFFFF);
        break;
    }
    case OP_ADDR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t rd_val = vm->reg[rd];
        uint32_t res32 = (uint32_t)vm->reg[ra] + (uint32_t)rd_val;
        vm->reg[rd] = (uint16_t)(res32 & 0xFFFF);
        set_flags_on_ADD(vm, vm->reg[ra], rd_val, res32, 0xFFFF);
        break;
    }

    case OP_SUB:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        uint16_t res16 = (vm->reg[rd] & 0xFF) - imm8;
        set_flags_on_SUB(vm, (uint8_t)vm->reg[rd], imm8, res16, 0xFF);
        vm->reg[rd] &= 0xFF00;
        vm->reg[rd] |= (uint8_t)res16;
        break;
    }
    case OP_SUBw:
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
        uint16_t res16 = vm->reg[rd] - imm16;
        vm->reg[rd] = res16;
        set_flags_on_SUB(vm, vm->reg[rd], imm16, res16, 0xFFFF);
        break;
    }
    case OP_SUBR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t res16 = vm->reg[rd] - vm->reg[ra];
        vm->reg[rd] = res16;
        set_flags_on_SUB(vm, vm->reg[rd], vm->reg[ra], res16, 0xFFFF);
        break;
    }

    case OP_MUL:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        uint16_t res16 = vm->reg[rd] * imm8;
        vm->reg[rd] = res16;
        set_flags_NZ(vm, res16, 0xFF);
        break;
    }
    case OP_MULw:
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
        uint32_t res32 = vm->reg[rd] * imm16;
        vm->reg[rd] = res32 & 0xFFFF;
        set_flags_NZ(vm, res32, 0xFFFF);
        break;
    }
    case OP_MULR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint32_t res32 = vm->reg[ra] * vm->reg[rd];
        vm->reg[rd] = res32 & 0xFFFF;
        set_flags_NZ(vm, res32, 0xFFFF);
        break;
    }

    case OP_CMP:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        uint16_t res16 = vm->reg[ra] - imm8;
        set_flags_on_SUB(vm, vm->reg[ra], imm8, res16, 0xFF);
        break;
    }
    case OP_CMPw:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low;
        imm16 |= (uint16_t)(high << 8);
        uint16_t result = vm->reg[ra] - imm16;
        set_flags_on_SUB(vm, vm->reg[ra], imm16, result, 0xFFFF);
        break;
    }
    case OP_CMPR:
    {
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_b = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t res16 = vm->reg[ra] - vm->reg[reg_b];
        set_flags_on_SUB(vm, vm->reg[ra], vm->reg[reg_b], res16, 0xFFFF);
        break;
    }

    case OP_INC:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t original = vm->reg[rd];
        uint16_t res16 = original + 1;
        vm->reg[rd] = res16;
        set_flags_on_ADD(vm, original, 1, res16, 0xFF);
        set_flags_NZ(vm, res16, 0xFF);
        break;
    }
    case OP_DEC:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint16_t res16 = vm->reg[rd] - 1;
        vm->reg[rd] = res16;
        set_flags_on_SUB(vm, vm->reg[rd], 1, res16, 0xFF);
        set_flags_NZ(vm, res16, 0xFF);
        break;
    }

        // =====
        // LOGIC
        // =====

    case OP_AND:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[rd] = vm->reg[ra] & imm8;
        set_flags_NZ(vm, vm->reg[rd], 0xFF);
        break;
    }
    case OP_ANDw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low;
        imm16 |= (uint16_t)(high << 8);
        vm->reg[rd] = vm->reg[ra] & imm16;
        set_flags_NZ(vm, vm->reg[rd], 0xFFFF);
        break;
    }
    case OP_ANDR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_b = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[rd] = vm->reg[ra] & vm->reg[reg_b];
        set_flags_NZ(vm, vm->reg[rd], 0xFF);
        break;
    }

    case OP_OR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[rd] = vm->reg[ra] | imm8;
        set_flags_NZ(vm, vm->reg[rd], 0xFF);
        break;
    }
    case OP_ORw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low;
        imm16 |= (uint16_t)(high << 8);
        vm->reg[rd] = vm->reg[ra] | imm16;
        set_flags_NZ(vm, vm->reg[rd], 0xFFFF);
        break;
    }
    case OP_ORR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_b = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[rd] = vm->reg[ra] | vm->reg[reg_b];
        set_flags_NZ(vm, vm->reg[rd], 0xFF);
        break;
    }

    case OP_XOR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[rd] = vm->reg[ra] ^ imm8;
        set_flags_NZ(vm, vm->reg[rd], 0xFF);
        break;
    }
    case OP_XORw:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        vm->ip++;
        uint16_t imm16 = 0x0000;
        imm16 |= (uint16_t)low;
        imm16 |= (uint16_t)(high << 8);
        vm->reg[rd] = vm->reg[ra] ^ imm16;
        set_flags_NZ(vm, vm->reg[rd], 0xFFFF);
        break;
    }
    case OP_XORR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t reg_b = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[rd] = vm->reg[ra] ^ vm->reg[reg_b];
        set_flags_NZ(vm, vm->reg[rd], 0xFF);
        break;
    }

    case OP_SHL:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[rd] = vm->reg[ra] << imm8;
        break;
    }
    case OP_SHR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        vm->reg[rd] = vm->reg[ra] >> imm8;
        break;
    }
    case OP_ASR:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t ra = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        uint8_t imm8 = vm->romb[vm->ip];
        vm->ip++;
        int16_t val = (int16_t)vm->reg[ra];
        vm->reg[rd] = (uint16_t)(val >> imm8);
        break;
    }

    case OP_NOT:
    {
        uint8_t rd = vm->romb[vm->ip] % REG_COUNT;
        vm->ip++;
        vm->reg[rd] = ~vm->reg[rd];
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
        // UNKNOWN OPCODE
        // =====

    default:
        return STEP_ERROR;
    }

    return STEP_OK;
}

