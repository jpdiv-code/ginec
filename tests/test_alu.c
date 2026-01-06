#include "opcodes.h"
#include "test_framework.h"
#include "test_vm.h"

// ========================================
// Arithmetic Instructions Tests
// ========================================

int test_add(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF10;

    // Program: ADD r0, 0x05
    vm.romb[0] = OP_ADD;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x05; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF15, "ADD: 0x10 + 0x05 = 0x15, got 0x%04X", vm.reg[0]);
    ASSERT(NO_FLAG(&vm, FLAG_Z), "Zero flag should not be set");
    ASSERT(NO_FLAG(&vm, FLAG_N), "Negative flag should not be set");
    ASSERT(NO_FLAG(&vm, FLAG_C), "Carry flag should not be set");

    return 1;
}

int test_add_with_carry(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[1] = 0xABFF;

    // Program: ADD r1, 0x02  (0xFF + 0x02 = 0x101, carry out)
    vm.romb[0] = OP_ADD;
    vm.romb[1] = 1;    // r1
    vm.romb[2] = 0x02; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[1] == 0xAB01, "ADD with carry: 0xFF + 0x02 = 0x01, got 0x%04X", vm.reg[1]);
    ASSERT(HAS_FLAG(&vm, FLAG_C), "Carry flag should be set");
    ASSERT(NO_FLAG(&vm, FLAG_Z), "Zero flag should not be set");

    return 1;
}

int test_add_zero(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[2] = 0x1200;

    // Program: ADD r2, 0x00
    vm.romb[0] = OP_ADD;
    vm.romb[1] = 2;    // r2
    vm.romb[2] = 0x00; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[2] == 0x1200, "ADD 0 should not change value, got 0x%04X", vm.reg[2]);
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set");

    return 1;
}

int test_sub(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF20;

    // Program: SUB r0, 0x10
    vm.romb[0] = OP_SUB;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x10; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF10, "SUB: 0x20 - 0x10 = 0x10, got 0x%04X", vm.reg[0]);
    ASSERT(NO_FLAG(&vm, FLAG_C), "Carry flag should not be set (no borrow)");
    ASSERT(NO_FLAG(&vm, FLAG_Z), "Zero flag should not be set");

    return 1;
}

int test_sub_with_borrow(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[1] = 0xAB05;

    // Program: SUB r1, 0x10  (0x05 - 0x10, borrow occurs)
    vm.romb[0] = OP_SUB;
    vm.romb[1] = 1;    // r1
    vm.romb[2] = 0x10; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[1] == 0xABF5, "SUB with borrow: 0x05 - 0x10 = 0xF5, got 0x%04X", vm.reg[1]);
    ASSERT(HAS_FLAG(&vm, FLAG_C), "Carry flag should be set (borrow occurred)");
    ASSERT(HAS_FLAG(&vm, FLAG_N), "Negative flag should be set");

    return 1;
}

int test_mul(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF05;

    // Program: MUL r0, 0x03
    vm.romb[0] = OP_MUL;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x03; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF0F, "MUL: 0x05 * 0x03 = 0x0F, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_addw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x1000;

    // Program: ADD.w r0, 0x0234
    vm.romb[0] = OP_ADDw;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x34; // imm16 low
    vm.romb[3] = 0x02; // imm16 high

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x1234, "ADD.w: 0x1000 + 0x0234 = 0x1234, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_addr(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF10;
    vm.reg[1] = 0xAB20;

    // Program: ADDR r0, r1
    vm.romb[0] = OP_ADDR;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF30, "ADDR: 0x10 + 0x20 = 0x30, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_inc(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF42;

    // Program: INC r0
    vm.romb[0] = OP_INC;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF43, "INC: 0x42 + 1 = 0x43, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_dec(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[1] = 0xAB10;

    // Program: DEC r1
    vm.romb[0] = OP_DEC;
    vm.romb[1] = 1; // r1

    vm_step(&vm);

    ASSERT(vm.reg[1] == 0xAB0F, "DEC: 0x10 - 1 = 0x0F, got 0x%04X", vm.reg[1]);

    return 1;
}

int test_cmp(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF42;

    // Program: CMP r0, 0x42  (should be equal)
    vm.romb[0] = OP_CMP;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x42; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF42, "CMP should not modify register");
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set (equal)");
    ASSERT(NO_FLAG(&vm, FLAG_C), "Carry flag should not be set");

    return 1;
}

// ========================================
// Logical Instructions Tests
// ========================================

int test_and(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFFAA;

    // Program: AND r0, 0x0F
    vm.romb[0] = OP_AND;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x0F; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF0A, "AND: 0xAA & 0x0F = 0x0A, got 0x%04X", vm.reg[0]);
    ASSERT(NO_FLAG(&vm, FLAG_Z), "Zero flag should not be set");

    return 1;
}

int test_or(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[1] = 0xAB0A;

    // Program: OR r1, 0x05
    vm.romb[0] = OP_OR;
    vm.romb[1] = 1;    // r1
    vm.romb[2] = 0x05; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[1] == 0xAB0F, "OR: 0x0A | 0x05 = 0x0F, got 0x%04X", vm.reg[1]);

    return 1;
}

int test_xor(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[2] = 0x12FF;

    // Program: XOR r2, 0xFF  (should result in 0x00)
    vm.romb[0] = OP_XOR;
    vm.romb[1] = 2;    // r2
    vm.romb[2] = 0xFF; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[2] == 0x1200, "XOR: 0xFF ^ 0xFF = 0x00, got 0x%04X", vm.reg[2]);
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set");

    return 1;
}

int test_not(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xAB0F;

    // Program: NOT r0
    vm.romb[0] = OP_NOT;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xABF0, "NOT: ~0x0F = 0xF0, got 0x%04X", vm.reg[0]);

    return 1;
}

// ========================================
// Shift Instructions Tests
// ========================================

int test_shl(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF05;

    // Program: SHL r0, 2
    vm.romb[0] = OP_SHL;
    vm.romb[1] = 0; // r0
    vm.romb[2] = 2; // shift count

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF14, "SHL: 0x05 << 2 = 0x14, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_shr(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[1] = 0xAB20;

    // Program: SHR r1, 2
    vm.romb[0] = OP_SHR;
    vm.romb[1] = 1; // r1
    vm.romb[2] = 2; // shift count

    vm_step(&vm);

    ASSERT(vm.reg[1] == 0xAB08, "SHR: 0x20 >> 2 = 0x08, got 0x%04X", vm.reg[1]);

    return 1;
}

int test_asr(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[2] = 0x12F0; // 0xF0 = -16 in signed 8-bit

    // Program: ASR r2, 2  (arithmetic shift right preserves sign)
    vm.romb[0] = OP_ASR;
    vm.romb[1] = 2; // r2
    vm.romb[2] = 2; // shift count

    vm_step(&vm);

    ASSERT(vm.reg[2] == 0x12FC, "ASR: 0xF0 >> 2 = 0xFC (sign extended), got 0x%04X", vm.reg[2]);

    return 1;
}

int test_sext8w(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x00F0; // -16 in signed 8-bit

    // Program: SEXT8.w r0
    vm.romb[0] = OP_SEXT8w;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFFF0, "SEXT8.w: sign extend 0xF0 to 0xFFF0, got 0x%04X", vm.reg[0]);

    return 1;
}

// ========================================
// Main test runner
// ========================================

int main(void)
{
    printf("\n========================================\n");
    printf("GINEC VM - ALU Instruction Tests\n");
    printf("========================================\n\n");

    // Arithmetic
    run_test("ADD", test_add);
    run_test("ADD with carry", test_add_with_carry);
    run_test("ADD zero flag", test_add_zero);
    run_test("SUB", test_sub);
    run_test("SUB with borrow", test_sub_with_borrow);
    run_test("MUL", test_mul);
    run_test("ADD.w", test_addw);
    run_test("ADDR", test_addr);
    run_test("INC", test_inc);
    run_test("DEC", test_dec);
    run_test("CMP", test_cmp);

    // Logical
    run_test("AND", test_and);
    run_test("OR", test_or);
    run_test("XOR", test_xor);
    run_test("NOT", test_not);

    // Shift
    run_test("SHL", test_shl);
    run_test("SHR", test_shr);
    run_test("ASR", test_asr);
    run_test("SEXT8.w", test_sext8w);

    print_summary();

    return tests_failed == 0 ? 0 : 1;
}
