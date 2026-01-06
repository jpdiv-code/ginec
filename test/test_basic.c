#include "opcodes.h"
#include "test_framework.h"
#include "test_vm.h"

// ========================================
// OTHER Instructions Tests
// ========================================

int test_nop(void)
{
    VM vm;
    init_test_vm(&vm);

    // Program: NOP, HLT
    vm.romb[0] = OP_NOP;
    vm.romb[1] = OP_HLT;

    vm_step(&vm);
    ASSERT(vm.ip == 1, "IP should advance after NOP");

    return 1;
}

int test_hlt(void)
{
    VM vm;
    init_test_vm(&vm);

    // Program: HLT
    vm.romb[0] = OP_HLT;

    StepResult res = vm_step(&vm);
    ASSERT(res == STEP_HALT, "HLT should return STEP_HALT");

    return 1;
}

int test_sync(void)
{
    VM vm;
    init_test_vm(&vm);

    // Program: SYNC, HLT
    vm.romb[0] = OP_SYNC;
    vm.romb[1] = OP_HLT;

    StepResult res = vm_step(&vm);
    ASSERT(res == STEP_SYNC, "SYNC should return STEP_SYNC");
    ASSERT(vm.ip == 1, "IP should advance after SYNC");

    return 1;
}

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
    printf("GINEC VM - Basic Instruction Tests\n");
    printf("========================================\n\n");

    // OTHER
    run_test("NOP", test_nop);
    run_test("HLT", test_hlt);
    run_test("SYNC", test_sync);

    // REG
    run_test("LDI", test_ldi);
    run_test("LDI.w", test_ldiw);
    run_test("MOV", test_mov);
    run_test("MOV.w", test_movw);
    run_test("SRB", test_srb);

    // STACK
    run_test("PUSHI", test_pushi);
    run_test("PUSHI.w", test_pushiw);
    run_test("PUSH/POP", test_push_pop);
    run_test("PUSH.w/POP.w", test_pushw_popw);
    run_test("DUP", test_dup);
    run_test("SWP", test_swp);
    run_test("ADJSP", test_adjsp);

    print_summary();

    return tests_failed == 0 ? 0 : 1;
}
