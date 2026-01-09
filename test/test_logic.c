#include "opcodes.h"
#include "test_framework.h"
#include "test_vm.h"

// ========================================
// Logical Instructions Tests
// ========================================

// AND variants

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

    ASSERT(vm.reg[0] == 0xFF0A, "AND: 0xAA & 0x0F = 0x0A, high byte preserved, got 0x%04X",
           vm.reg[0]);
    ASSERT(NO_FLAG(&vm, FLAG_Z), "Zero flag should not be set");
    ASSERT(NO_FLAG(&vm, FLAG_C), "Carry flag should be cleared");
    ASSERT(NO_FLAG(&vm, FLAG_V), "Overflow flag should be cleared");

    return 1;
}

int test_and_zero(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xAB55;

    // Program: AND r0, 0xAA  (0x55 & 0xAA = 0x00)
    vm.romb[0] = OP_AND;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0xAA; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xAB00, "AND to zero: 0x55 & 0xAA = 0x00, high byte preserved, got 0x%04X",
           vm.reg[0]);
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set");

    return 1;
}

int test_andw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xF0F0;

    // Program: AND.w r0, 0x00FF
    vm.romb[0] = OP_ANDw;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0xFF; // imm16 low
    vm.romb[3] = 0x00; // imm16 high

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x00F0, "AND.w: 0xF0F0 & 0x00FF = 0x00F0, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_andr(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFFAA;
    vm.reg[1] = 0xAB33;

    // Program: ANDR r0, r1
    vm.romb[0] = OP_ANDR;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF22, "ANDR: 0xAA & 0x33 = 0x22, high byte preserved, got 0x%04X",
           vm.reg[0]);

    return 1;
}

int test_andrw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xF0F0;
    vm.reg[1] = 0x0F0F;

    // Program: ANDR.w r0, r1
    vm.romb[0] = OP_ANDRw;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x0000, "ANDR.w: 0xF0F0 & 0x0F0F = 0x0000, got 0x%04X", vm.reg[0]);
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set");

    return 1;
}

// OR variants

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

    ASSERT(vm.reg[1] == 0xAB0F, "OR: 0x0A | 0x05 = 0x0F, high byte preserved, got 0x%04X",
           vm.reg[1]);
    ASSERT(NO_FLAG(&vm, FLAG_C), "Carry flag should be cleared");
    ASSERT(NO_FLAG(&vm, FLAG_V), "Overflow flag should be cleared");

    return 1;
}

int test_or_zero(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF00;

    // Program: OR r0, 0x00
    vm.romb[0] = OP_OR;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x00; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF00, "OR with 0: should not change, high byte preserved, got 0x%04X",
           vm.reg[0]);
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set");

    return 1;
}

int test_orw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x0F00;

    // Program: OR.w r0, 0x00F0
    vm.romb[0] = OP_ORw;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0xF0; // imm16 low
    vm.romb[3] = 0x00; // imm16 high

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x0FF0, "OR.w: 0x0F00 | 0x00F0 = 0x0FF0, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_orr(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF0F;
    vm.reg[1] = 0xABF0;

    // Program: ORR r0, r1
    vm.romb[0] = OP_ORR;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFFFF, "ORR: 0x0F | 0xF0 = 0xFF, high byte preserved, got 0x%04X",
           vm.reg[0]);

    return 1;
}

int test_orrw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x0F00;
    vm.reg[1] = 0x00F0;

    // Program: ORR.w r0, r1
    vm.romb[0] = OP_ORRw;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x0FF0, "ORR.w: 0x0F00 | 0x00F0 = 0x0FF0, got 0x%04X", vm.reg[0]);

    return 1;
}

// XOR variants

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

    ASSERT(vm.reg[2] == 0x1200, "XOR: 0xFF ^ 0xFF = 0x00, high byte preserved, got 0x%04X",
           vm.reg[2]);
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set");

    return 1;
}

int test_xor_same_bits(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xABAA;

    // Program: XOR r0, 0xAA
    vm.romb[0] = OP_XOR;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0xAA; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xAB00, "XOR: 0xAA ^ 0xAA = 0x00, high byte preserved, got 0x%04X",
           vm.reg[0]);
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set");

    return 1;
}

int test_xorw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFFFF;

    // Program: XOR.w r0, 0xAAAA
    vm.romb[0] = OP_XORw;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0xAA; // imm16 low
    vm.romb[3] = 0xAA; // imm16 high

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x5555, "XOR.w: 0xFFFF ^ 0xAAAA = 0x5555, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_xorr(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF5A;
    vm.reg[1] = 0xABA5;

    // Program: XORR r0, r1
    vm.romb[0] = OP_XORR;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFFFF, "XORR: 0x5A ^ 0xA5 = 0xFF, high byte preserved, got 0x%04X",
           vm.reg[0]);

    return 1;
}

int test_xorrw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xAAAA;
    vm.reg[1] = 0xAAAA;

    // Program: XORR.w r0, r1
    vm.romb[0] = OP_XORRw;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x0000, "XORR.w: 0xAAAA ^ 0xAAAA = 0x0000, got 0x%04X", vm.reg[0]);
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set");

    return 1;
}

// NOT variants

int test_not(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xAB0F;

    // Program: NOT r0
    vm.romb[0] = OP_NOT;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xABF0, "NOT: ~0x0F = 0xF0, high byte preserved, got 0x%04X", vm.reg[0]);
    ASSERT(HAS_FLAG(&vm, FLAG_N), "Negative flag should be set");

    return 1;
}

int test_notw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x0F0F;

    // Program: NOT.w r0
    vm.romb[0] = OP_NOTw;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xF0F0, "NOT.w: ~0x0F0F = 0xF0F0, got 0x%04X", vm.reg[0]);
    ASSERT(HAS_FLAG(&vm, FLAG_N), "Negative flag should be set");

    return 1;
}

// Shift Instructions Tests

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

    ASSERT(vm.reg[0] == 0xFF14, "SHL: 0x05 << 2 = 0x14, high byte preserved, got 0x%04X",
           vm.reg[0]);

    return 1;
}

int test_shl_overflow(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xABFF;

    // Program: SHL r0, 1  (bit shifts out)
    vm.romb[0] = OP_SHL;
    vm.romb[1] = 0; // r0
    vm.romb[2] = 1; // shift count

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xABFE, "SHL: 0xFF << 1 = 0xFE, high byte preserved, got 0x%04X",
           vm.reg[0]);
    // Note: ISA says C MAY capture shifted-out bit, but not guaranteed

    return 1;
}

int test_shlw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x1234;

    // Program: SHL.w r0, 4
    vm.romb[0] = OP_SHLw;
    vm.romb[1] = 0; // r0
    vm.romb[2] = 4; // shift count

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x2340, "SHL.w: 0x1234 << 4 = 0x2340, got 0x%04X", vm.reg[0]);

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

    ASSERT(vm.reg[1] == 0xAB08, "SHR: 0x20 >> 2 = 0x08, high byte preserved, got 0x%04X",
           vm.reg[1]);

    return 1;
}

int test_shr_with_carry(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF0F;

    // Program: SHR r0, 1  (bit shifts out)
    vm.romb[0] = OP_SHR;
    vm.romb[1] = 0; // r0
    vm.romb[2] = 1; // shift count

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF07, "SHR: 0x0F >> 1 = 0x07, high byte preserved, got 0x%04X",
           vm.reg[0]);
    // Note: ISA says C MAY capture shifted-out bit, but not guaranteed

    return 1;
}

int test_shrw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x1234;

    // Program: SHR.w r0, 4
    vm.romb[0] = OP_SHRw;
    vm.romb[1] = 0; // r0
    vm.romb[2] = 4; // shift count

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x0123, "SHR.w: 0x1234 >> 4 = 0x0123, got 0x%04X", vm.reg[0]);

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

    ASSERT(vm.reg[2] == 0x12FC,
           "ASR: 0xF0 >> 2 = 0xFC (sign extended), high byte preserved, got 0x%04X", vm.reg[2]);
    ASSERT(HAS_FLAG(&vm, FLAG_N), "Negative flag should be set");

    return 1;
}

int test_asr_positive(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xAB40; // 0x40 = +64 in signed 8-bit

    // Program: ASR r0, 2
    vm.romb[0] = OP_ASR;
    vm.romb[1] = 0; // r0
    vm.romb[2] = 2; // shift count

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xAB10, "ASR positive: 0x40 >> 2 = 0x10, high byte preserved, got 0x%04X",
           vm.reg[0]);
    ASSERT(NO_FLAG(&vm, FLAG_N), "Negative flag should not be set");

    return 1;
}

int test_asrw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xF000; // Negative in 16-bit signed

    // Program: ASR.w r0, 4
    vm.romb[0] = OP_ASRw;
    vm.romb[1] = 0; // r0
    vm.romb[2] = 4; // shift count

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF00, "ASR.w: 0xF000 >> 4 = 0xFF00 (sign extended), got 0x%04X",
           vm.reg[0]);
    ASSERT(HAS_FLAG(&vm, FLAG_N), "Negative flag should be set");

    return 1;
}

int test_asrw_positive(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x4000; // Positive in 16-bit signed

    // Program: ASR.w r0, 4
    vm.romb[0] = OP_ASRw;
    vm.romb[1] = 0; // r0
    vm.romb[2] = 4; // shift count

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x0400, "ASR.w positive: 0x4000 >> 4 = 0x0400, got 0x%04X", vm.reg[0]);
    ASSERT(NO_FLAG(&vm, FLAG_N), "Negative flag should not be set");

    return 1;
}

// Sign extension

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
    // Note: SEXT8.w may not set flags according to current implementation

    return 1;
}

int test_sext8w_positive(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF7F; // +127 in signed 8-bit

    // Program: SEXT8.w r0
    vm.romb[0] = OP_SEXT8w;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x007F, "SEXT8.w positive: sign extend 0x7F to 0x007F, got 0x%04X",
           vm.reg[0]);
    ASSERT(NO_FLAG(&vm, FLAG_N), "Negative flag should not be set");

    return 1;
}

// ========================================
// Main test runner
// ========================================

int main(void)
{
    printf("\n========================================\n");
    printf("GINEC VM - Logic Instructions Tests\n");
    printf("========================================\n\n");

    // AND variants
    run_test("AND", test_and);
    run_test("AND zero result", test_and_zero);
    run_test("AND.w", test_andw);
    run_test("ANDR", test_andr);
    run_test("ANDR.w", test_andrw);

    // OR variants
    run_test("OR", test_or);
    run_test("OR zero", test_or_zero);
    run_test("OR.w", test_orw);
    run_test("ORR", test_orr);
    run_test("ORR.w", test_orrw);

    // XOR variants
    run_test("XOR", test_xor);
    run_test("XOR same bits", test_xor_same_bits);
    run_test("XOR.w", test_xorw);
    run_test("XORR", test_xorr);
    run_test("XORR.w", test_xorrw);

    // NOT variants
    run_test("NOT", test_not);
    run_test("NOT.w", test_notw);

    // Shift operations
    run_test("SHL", test_shl);
    run_test("SHL overflow", test_shl_overflow);
    run_test("SHL.w", test_shlw);
    run_test("SHR", test_shr);
    run_test("SHR with carry", test_shr_with_carry);
    run_test("SHR.w", test_shrw);
    run_test("ASR", test_asr);
    run_test("ASR positive", test_asr_positive);
    run_test("ASR.w", test_asrw);
    run_test("ASR.w positive", test_asrw_positive);

    // Sign extension
    run_test("SEXT8.w", test_sext8w);
    run_test("SEXT8.w positive", test_sext8w_positive);

    print_summary();

    return tests_failed == 0 ? 0 : 1;
}
