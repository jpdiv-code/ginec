#include "opcodes.h"
#include "test_framework.h"
#include "test_vm.h"

// ========================================
// REG Instructions Tests
// ========================================

int test_ldi(void)
{
    VM vm;
    init_test_vm(&vm);

    // Program: LDI r0, 0x42
    vm.romb[0] = OP_LDI;
    vm.romb[1] = 0; // r0
    vm.romb[2] = 0x42;

    vm.reg[0] = 0xABCD; // Set high byte to test preservation
    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xAB42, "LDI should load 0x42 into low byte, expected 0xAB42, got 0x%04X",
           vm.reg[0]);
    ASSERT(vm.ip == 3, "IP should advance by 3");

    return 1;
}

int test_ldiw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Program: LDI.w r1, 0x1234
    vm.romb[0] = OP_LDIw;
    vm.romb[1] = 1;    // r1
    vm.romb[2] = 0x34; // low byte
    vm.romb[3] = 0x12; // high byte

    vm_step(&vm);

    ASSERT(vm.reg[1] == 0x1234, "LDI.w should load 0x1234, got 0x%04X", vm.reg[1]);
    ASSERT(vm.ip == 4, "IP should advance by 4");

    return 1;
}

int test_mov(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup
    vm.reg[0] = 0x1234;
    vm.reg[1] = 0xABCD;

    // Program: MOV r1, r0
    vm.romb[0] = OP_MOV;
    vm.romb[1] = 1; // rd = r1
    vm.romb[2] = 0; // rs = r0

    vm_step(&vm);

    ASSERT(vm.reg[1] == 0xAB34, "MOV should copy low byte: expected 0xAB34, got 0x%04X", vm.reg[1]);
    ASSERT(vm.reg[0] == 0x1234, "Source register should not change");

    return 1;
}

int test_movw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup
    vm.reg[2] = 0x5678;
    vm.reg[3] = 0xFFFF;

    // Program: MOV.w r3, r2
    vm.romb[0] = OP_MOVw;
    vm.romb[1] = 3; // rd = r3
    vm.romb[2] = 2; // rs = r2

    vm_step(&vm);

    ASSERT(vm.reg[3] == 0x5678, "MOV.w should copy full word: expected 0x5678, got 0x%04X",
           vm.reg[3]);
    ASSERT(vm.reg[2] == 0x5678, "Source register should not change");

    return 1;
}

int test_srb(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup
    vm.reg[0] = 0x1234;

    // Program: SRB r0
    vm.romb[0] = OP_SRB;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x3412, "SRB should swap bytes: expected 0x3412, got 0x%04X", vm.reg[0]);

    return 1;
}

// ========================================
// Main test runner
// ========================================

int main(void)
{
    printf("\n========================================\n");
    printf("GINEC VM - REG Instructions Tests\n");
    printf("========================================\n\n");

    run_test("LDI", test_ldi);
    run_test("LDI.w", test_ldiw);
    run_test("MOV", test_mov);
    run_test("MOV.w", test_movw);
    run_test("SRB", test_srb);

    print_summary();

    return tests_failed == 0 ? 0 : 1;
}
