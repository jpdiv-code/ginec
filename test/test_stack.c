#include "opcodes.h"
#include "test_framework.h"
#include "test_vm.h"

// ========================================
// STACK Instructions Tests
// ========================================

int test_pushi(void)
{
    VM vm;
    init_test_vm(&vm);

    // Program: PUSHI 0x55
    vm.romb[0] = OP_PUSHI;
    vm.romb[1] = 0x55;

    uint16_t sp_before = vm.sp;
    vm_step(&vm);

    ASSERT(vm.sp == sp_before - 1, "SP should decrement by 1");
    ASSERT(vm.ram[sp_before] == 0x55, "Value should be on stack at old SP");

    return 1;
}

int test_pushiw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Program: PUSHI.w 0xABCD
    vm.romb[0] = OP_PUSHIw;
    vm.romb[1] = 0xCD; // low
    vm.romb[2] = 0xAB; // high

    uint16_t sp_before = vm.sp;
    vm_step(&vm);

    ASSERT(vm.sp == sp_before - 2, "SP should decrement by 2");
    ASSERT(vm.ram[sp_before - 1] == 0xCD, "Low byte at sp+1");
    ASSERT(vm.ram[sp_before] == 0xAB, "High byte at old sp");

    return 1;
}

int test_push_pop(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup
    vm.reg[0] = 0xAB42;

    // Program: PUSH r0, POP r1
    vm.romb[0] = OP_PUSH;
    vm.romb[1] = 0; // r0
    vm.romb[2] = OP_POP;
    vm.romb[3] = 1; // r1

    vm.reg[1] = 0xFFFF;

    vm_execute_steps(&vm, 2);

    ASSERT(vm.reg[1] == 0xFF42, "POP should restore low byte: expected 0xFF42, got 0x%04X",
           vm.reg[1]);

    return 1;
}

int test_pushw_popw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup
    vm.reg[2] = 0x1234;

    // Program: PUSH.w r2, POP.w r3
    vm.romb[0] = OP_PUSHw;
    vm.romb[1] = 2; // r2
    vm.romb[2] = OP_POPw;
    vm.romb[3] = 3; // r3

    vm_execute_steps(&vm, 2);

    ASSERT(vm.reg[3] == 0x1234, "POP.w should restore word: expected 0x1234, got 0x%04X",
           vm.reg[3]);

    return 1;
}

int test_dup(void)
{
    VM vm;
    init_test_vm(&vm);

    // Program: PUSHI 0x99, DUP, POP r0, POP r1
    vm.romb[0] = OP_PUSHI;
    vm.romb[1] = 0x99;
    vm.romb[2] = OP_DUP;
    vm.romb[3] = OP_POP;
    vm.romb[4] = 0; // r0
    vm.romb[5] = OP_POP;
    vm.romb[6] = 1; // r1

    vm_execute_steps(&vm, 4);

    ASSERT((vm.reg[0] & 0xFF) == 0x99, "First pop should be 0x99");
    ASSERT((vm.reg[1] & 0xFF) == 0x99, "Second pop should also be 0x99 (duplicated)");

    return 1;
}

int test_swp(void)
{
    VM vm;
    init_test_vm(&vm);

    // Program: PUSHI 0xAA, PUSHI 0xBB, SWP, POP r0, POP r1
    vm.romb[0] = OP_PUSHI;
    vm.romb[1] = 0xAA;
    vm.romb[2] = OP_PUSHI;
    vm.romb[3] = 0xBB;
    vm.romb[4] = OP_SWP;
    vm.romb[5] = OP_POP;
    vm.romb[6] = 0; // r0
    vm.romb[7] = OP_POP;
    vm.romb[8] = 1; // r1

    vm_execute_steps(&vm, 5);

    ASSERT((vm.reg[0] & 0xFF) == 0xAA, "After swap, first pop should be 0xAA");
    ASSERT((vm.reg[1] & 0xFF) == 0xBB, "After swap, second pop should be 0xBB");

    return 1;
}

int test_swpw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Program: PUSHI.w 0x1234, PUSHI.w 0x5678, SWP.w, POP.w r0, POP.w r1
    vm.romb[0] = OP_PUSHIw;
    vm.romb[1] = 0x34; // low byte of 0x1234
    vm.romb[2] = 0x12; // high byte
    vm.romb[3] = OP_PUSHIw;
    vm.romb[4] = 0x78; // low byte of 0x5678
    vm.romb[5] = 0x56; // high byte
    vm.romb[6] = OP_SWPw;
    vm.romb[7] = OP_POPw;
    vm.romb[8] = 0; // r0
    vm.romb[9] = OP_POPw;
    vm.romb[10] = 1; // r1

    vm_execute_steps(&vm, 5);

    ASSERT(vm.reg[0] == 0x1234, "After swap, first pop should be 0x1234, got 0x%04X", vm.reg[0]);
    ASSERT(vm.reg[1] == 0x5678, "After swap, second pop should be 0x5678, got 0x%04X", vm.reg[1]);

    return 1;
}

int test_dupw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Program: PUSHI.w 0xABCD, DUP.w, POP.w r0, POP.w r1
    vm.romb[0] = OP_PUSHIw;
    vm.romb[1] = 0xCD; // low byte
    vm.romb[2] = 0xAB; // high byte
    vm.romb[3] = OP_DUPw;
    vm.romb[4] = OP_POPw;
    vm.romb[5] = 0; // r0
    vm.romb[6] = OP_POPw;
    vm.romb[7] = 1; // r1

    vm_execute_steps(&vm, 4);

    ASSERT(vm.reg[0] == 0xABCD, "First pop should be 0xABCD, got 0x%04X", vm.reg[0]);
    ASSERT(vm.reg[1] == 0xABCD, "Second pop should also be 0xABCD (duplicated), got 0x%04X",
           vm.reg[1]);

    return 1;
}

int test_adjsp(void)
{
    VM vm;
    init_test_vm(&vm);

    uint16_t sp_before = vm.sp;

    // Program: ADJSP -10 (0xF6 in two's complement)
    vm.romb[0] = OP_ADJSP;
    vm.romb[1] = 0xF6; // -10 as int8_t

    vm_step(&vm);

    ASSERT(vm.sp == sp_before - 10, "SP should decrease by 10, expected 0x%04X, got 0x%04X",
           sp_before - 10, vm.sp);

    return 1;
}

// ========================================
// Main test runner
// ========================================

int main(void)
{
    printf("\n========================================\n");
    printf("GINEC VM - STACK Instructions Tests\n");
    printf("========================================\n\n");

    run_test("PUSHI", test_pushi);
    run_test("PUSHI.w", test_pushiw);
    run_test("PUSH/POP", test_push_pop);
    run_test("PUSH.w/POP.w", test_pushw_popw);
    run_test("DUP", test_dup);
    run_test("DUP.w", test_dupw);
    run_test("SWP", test_swp);
    run_test("SWP.w", test_swpw);
    run_test("ADJSP", test_adjsp);

    print_summary();

    return tests_failed == 0 ? 0 : 1;
}
