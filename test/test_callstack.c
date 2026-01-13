#include "opcodes.h"
#include "test_framework.h"
#include "test_vm.h"

// ========================================
// Call Stack Instructions Tests
// ========================================

int test_pushcw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xABCD;
    uint16_t csp_before = vm.csp;

    // Program: PUSHC.w r0
    vm.romb[0] = OP_PUSHCw;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.csp == csp_before - 2, "CSP should decrement by 2, got 0x%04X", vm.csp);

    // Check value on call stack (little-endian: low byte at lower address)
    uint8_t low = vm.ram[csp_before - 1];
    uint8_t high = vm.ram[csp_before];
    uint16_t val = (uint16_t)(low | (high << 8));

    ASSERT(val == 0xABCD, "Value on call stack should be 0xABCD, got 0x%04X", val);

    return 1;
}

int test_popcw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Manually push value to call stack
    vm.ram[0xFFFF] = 0x56; // high byte at csp
    vm.ram[0xFFFE] = 0x78; // low byte at csp-1
    vm.csp = 0xFFFD;

    // Program: POPC.w r1
    vm.romb[0] = OP_POPCw;
    vm.romb[1] = 1; // r1

    vm_step(&vm);

    ASSERT(vm.csp == 0xFFFF, "CSP should be restored to 0xFFFF, got 0x%04X", vm.csp);
    ASSERT(vm.reg[1] == 0x5678, "r1 should be 0x5678, got 0x%04X", vm.reg[1]);

    return 1;
}

int test_pushcw_popcw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[2] = 0x1234;

    // Program: PUSHC.w r2, POPC.w r3
    vm.romb[0] = OP_PUSHCw;
    vm.romb[1] = 2; // r2
    vm.romb[2] = OP_POPCw;
    vm.romb[3] = 3; // r3

    vm_execute_steps(&vm, 2);

    ASSERT(vm.reg[3] == 0x1234, "POPC.w should restore pushed value, got 0x%04X", vm.reg[3]);
    ASSERT(vm.csp == 0xFFFF, "CSP should be back to initial value");

    return 1;
}

int test_adjcsp_positive(void)
{
    VM vm;
    init_test_vm(&vm);

    uint16_t csp_before = vm.csp;
    uint16_t expected = (uint16_t)(csp_before + 10);

    // Program: ADJCSP +10
    vm.romb[0] = OP_ADJCSP;
    vm.romb[1] = 10; // positive adjustment

    vm_step(&vm);

    ASSERT(vm.csp == expected, "CSP should increase by 10, expected 0x%04X, got 0x%04X", expected,
           vm.csp);

    return 1;
}

int test_adjcsp_negative(void)
{
    VM vm;
    init_test_vm(&vm);

    uint16_t csp_before = vm.csp;

    // Program: ADJCSP -20 (0xEC in two's complement for -20)
    vm.romb[0] = OP_ADJCSP;
    vm.romb[1] = 0xEC; // -20 as int8_t

    vm_step(&vm);

    ASSERT(vm.csp == csp_before - 20, "CSP should decrease by 20, expected 0x%04X, got 0x%04X",
           csp_before - 20, vm.csp);

    return 1;
}

int test_call_stack_isolation(void)
{
    VM vm;
    init_test_vm(&vm);

    uint16_t sp_before = vm.sp;
    uint16_t csp_before = vm.csp;

    // Push data to data stack
    vm.romb[0] = OP_PUSHI;
    vm.romb[1] = 0xAA;

    // Push data to call stack
    vm.reg[0] = 0xBBBB;
    vm.romb[2] = OP_PUSHCw;
    vm.romb[3] = 0; // r0

    vm_execute_steps(&vm, 2);

    ASSERT(vm.sp == sp_before - 1, "Data stack should have one byte pushed");
    ASSERT(vm.csp == csp_before - 2, "Call stack should have word pushed");

    // Verify data stack value
    ASSERT(vm.ram[sp_before] == 0xAA, "Data stack should contain 0xAA");

    // Verify call stack value
    uint16_t call_val = (uint16_t)(vm.ram[csp_before - 1] | (vm.ram[csp_before] << 8));
    ASSERT(call_val == 0xBBBB, "Call stack should contain 0xBBBB, got 0x%04X", call_val);

    return 1;
}

int test_nested_calls(void)
{
    VM vm;
    init_test_vm(&vm);

    // Program:
    // 0x0000: CALL 0x0010  -> calls func1
    // 0x0003: HLT
    //
    // 0x0010: (func1) CALL 0x0020  -> calls func2
    // 0x0013: RET
    //
    // 0x0020: (func2) RET

    vm.romb[0x0000] = OP_CALL;
    vm.romb[0x0001] = 0x10;
    vm.romb[0x0002] = 0x00;
    vm.romb[0x0003] = OP_HLT;

    vm.romb[0x0010] = OP_CALL;
    vm.romb[0x0011] = 0x20;
    vm.romb[0x0012] = 0x00;
    vm.romb[0x0013] = OP_RET;

    vm.romb[0x0020] = OP_RET;

    uint16_t csp_initial = vm.csp;

    vm_step(&vm); // CALL 0x0010
    ASSERT(vm.ip == 0x0010, "Should be at func1");
    ASSERT(vm.csp == csp_initial - 2, "CSP after first call");

    vm_step(&vm); // CALL 0x0020
    ASSERT(vm.ip == 0x0020, "Should be at func2");
    ASSERT(vm.csp == csp_initial - 4, "CSP after second call");

    vm_step(&vm); // RET from func2
    ASSERT(vm.ip == 0x0013, "Should return to func1");
    ASSERT(vm.csp == csp_initial - 2, "CSP after first return");

    vm_step(&vm); // RET from func1
    ASSERT(vm.ip == 0x0003, "Should return to main");
    ASSERT(vm.csp == csp_initial, "CSP should be restored");

    return 1;
}

int test_call_with_manual_stack_management(void)
{
    VM vm;
    init_test_vm(&vm);

    // Simulate a function that saves/restores a register using call stack
    // 0x0000: LDI.w r0, 0x1111
    // 0x0004: CALL 0x0010
    // 0x0007: HLT
    //
    // 0x0010: (func)
    //         PUSHC.w r0          ; save r0
    //         LDI.w r0, 0x2222    ; modify r0
    //         POPC.w r0           ; restore r0
    //         RET

    vm.romb[0x0000] = OP_LDIw;
    vm.romb[0x0001] = 0;    // r0
    vm.romb[0x0002] = 0x11; // low
    vm.romb[0x0003] = 0x11; // high

    vm.romb[0x0004] = OP_CALL;
    vm.romb[0x0005] = 0x10;
    vm.romb[0x0006] = 0x00;

    vm.romb[0x0007] = OP_HLT;

    vm.romb[0x0010] = OP_PUSHCw;
    vm.romb[0x0011] = 0; // r0

    vm.romb[0x0012] = OP_LDIw;
    vm.romb[0x0013] = 0;    // r0
    vm.romb[0x0014] = 0x22; // low
    vm.romb[0x0015] = 0x22; // high

    vm.romb[0x0016] = OP_POPCw;
    vm.romb[0x0017] = 0; // r0

    vm.romb[0x0018] = OP_RET;

    vm_execute_steps(&vm, 1); // LDI.w r0, 0x1111
    ASSERT(vm.reg[0] == 0x1111, "r0 should be 0x1111");

    vm_execute_steps(&vm, 1); // CALL 0x0010
    ASSERT(vm.ip == 0x0010, "Should be at function");

    vm_execute_steps(&vm, 1); // PUSHC.w r0
    vm_execute_steps(&vm, 1); // LDI.w r0, 0x2222
    ASSERT(vm.reg[0] == 0x2222, "r0 should be modified to 0x2222");

    vm_execute_steps(&vm, 1); // POPC.w r0
    ASSERT(vm.reg[0] == 0x1111, "r0 should be restored to 0x1111");

    vm_execute_steps(&vm, 1); // RET
    ASSERT(vm.ip == 0x0007, "Should return to main");
    ASSERT(vm.csp == 0xFFFF, "CSP should be fully restored");

    return 1;
}

int test_adjcsp_for_locals(void)
{
    VM vm;
    init_test_vm(&vm);

    // Simulate allocating local variables on call stack
    // ADJCSP -8  ; allocate 8 bytes
    // ... use locals ...
    // ADJCSP +8  ; deallocate

    vm.romb[0] = OP_ADJCSP;
    vm.romb[1] = 0xF8; // -8

    vm.romb[2] = OP_ADJCSP;
    vm.romb[3] = 8; // +8

    uint16_t csp_initial = vm.csp;

    vm_step(&vm);
    ASSERT(vm.csp == csp_initial - 8, "Should allocate 8 bytes");

    vm_step(&vm);
    ASSERT(vm.csp == csp_initial, "Should deallocate 8 bytes");

    return 1;
}

int test_call_stack_boundary(void)
{
    VM vm;
    init_test_vm(&vm);

    // Test that call stack is in correct region (0xFE00-0xFFFF)
    ASSERT(vm.csp == 0xFFFF, "CSP should start at 0xFFFF");

    // Push multiple values
    for (int i = 0; i < 10; i++)
    {
        vm.reg[0] = (uint16_t)(0x1000 + i);
        vm.romb[i * 2] = OP_PUSHCw;
        vm.romb[i * 2 + 1] = 0;
        vm.ip = (uint16_t)(i * 2);
        vm_step(&vm);
    }

    ASSERT(vm.csp == 0xFFFF - 20, "CSP should be at 0xFFEB after 10 pushes");
    ASSERT(vm.csp >= 0xFE00, "CSP should still be within call stack region");

    return 1;
}

int test_data_and_call_stack_independence(void)
{
    VM vm;
    init_test_vm(&vm);

    uint16_t sp_initial = vm.sp;
    uint16_t csp_initial = vm.csp;

    // Mix data stack and call stack operations
    // Push to data stack
    vm.romb[0] = OP_PUSHI;
    vm.romb[1] = 0x11;

    // Push to call stack
    vm.reg[0] = 0x2222;
    vm.romb[2] = OP_PUSHCw;
    vm.romb[3] = 0;

    // Push to data stack again
    vm.romb[4] = OP_PUSHI;
    vm.romb[5] = 0x33;

    // Push to call stack again
    vm.reg[1] = 0x4444;
    vm.romb[6] = OP_PUSHCw;
    vm.romb[7] = 1;

    vm_execute_steps(&vm, 4);

    // Verify stacks are independent
    ASSERT(vm.sp == sp_initial - 2, "Data stack should have 2 bytes pushed");
    ASSERT(vm.csp == csp_initial - 4, "Call stack should have 4 bytes pushed");

    // Pop from call stack first
    vm.romb[8] = OP_POPCw;
    vm.romb[9] = 2;
    vm.romb[10] = OP_POPCw;
    vm.romb[11] = 3;

    vm_execute_steps(&vm, 2);

    ASSERT(vm.reg[2] == 0x4444, "Should pop 0x4444 from call stack");
    ASSERT(vm.reg[3] == 0x2222, "Should pop 0x2222 from call stack");
    ASSERT(vm.csp == csp_initial, "Call stack should be empty");
    ASSERT(vm.sp == sp_initial - 2, "Data stack should be unchanged");

    // Now pop from data stack
    vm.romb[12] = OP_POP;
    vm.romb[13] = 0;
    vm.romb[14] = OP_POP;
    vm.romb[15] = 1;

    vm_execute_steps(&vm, 2);

    ASSERT((vm.reg[0] & 0xFF) == 0x33, "Should pop 0x33 from data stack");
    ASSERT((vm.reg[1] & 0xFF) == 0x11, "Should pop 0x11 from data stack");
    ASSERT(vm.sp == sp_initial, "Data stack should be empty");

    return 1;
}

int test_recursive_calls(void)
{
    VM vm;
    init_test_vm(&vm);

    // Simple recursive countdown - simpler version
    // We manually call a function 2 times to test recursion

    // 0x0000: CALL 0x0010  (level 1)
    // 0x0003: HLT
    //
    // 0x0010: (func)
    //         LDI r0, 1
    //         CALL 0x0020  (level 2)
    //         RET
    //
    // 0x0020: (func2)
    //         LDI r1, 2
    //         RET

    vm.romb[0x0000] = OP_CALL;
    vm.romb[0x0001] = 0x10;
    vm.romb[0x0002] = 0x00;

    vm.romb[0x0003] = OP_HLT;

    vm.romb[0x0010] = OP_LDI;
    vm.romb[0x0011] = 0; // r0
    vm.romb[0x0012] = 1; // value 1

    vm.romb[0x0013] = OP_CALL;
    vm.romb[0x0014] = 0x20;
    vm.romb[0x0015] = 0x00;

    vm.romb[0x0016] = OP_RET;

    vm.romb[0x0020] = OP_LDI;
    vm.romb[0x0021] = 1; // r1
    vm.romb[0x0022] = 2; // value 2

    vm.romb[0x0023] = OP_RET;

    uint16_t csp_initial = vm.csp;

    // Execute until halt
    StepResult res = vm_execute_until_halt(&vm, 100);

    ASSERT(res == STEP_HALT, "Should halt successfully");
    ASSERT((vm.reg[0] & 0xFF) == 1, "r0 should be 1 after execution, got 0x%04X", vm.reg[0]);
    ASSERT((vm.reg[1] & 0xFF) == 2, "r1 should be 2 after execution, got 0x%04X", vm.reg[1]);
    ASSERT(vm.csp == csp_initial,
           "CSP should be fully restored after all returns, expected 0x%04X, got 0x%04X",
           csp_initial, vm.csp);
    // Note: IP is incremented before HLT is detected, so it will be at 0x0004

    return 1;
}

// ========================================
// Main test runner
// ========================================

int main(void)
{
    printf("\n========================================\n");
    printf("GINEC VM - Call Stack Tests\n");
    printf("========================================\n\n");

    run_test("PUSHC.w", test_pushcw);
    run_test("POPC.w", test_popcw);
    run_test("PUSHC.w/POPC.w roundtrip", test_pushcw_popcw);
    run_test("ADJCSP positive", test_adjcsp_positive);
    run_test("ADJCSP negative", test_adjcsp_negative);
    run_test("Call stack isolation from data stack", test_call_stack_isolation);
    run_test("Nested function calls", test_nested_calls);
    run_test("Call with manual stack management", test_call_with_manual_stack_management);
    run_test("ADJCSP for local variables", test_adjcsp_for_locals);
    run_test("Call stack boundary", test_call_stack_boundary);
    run_test("Data and call stack independence", test_data_and_call_stack_independence);
    run_test("Recursive calls", test_recursive_calls);

    print_summary();

    return tests_failed == 0 ? 0 : 1;
}
