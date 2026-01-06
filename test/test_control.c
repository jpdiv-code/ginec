#include "opcodes.h"
#include "test_framework.h"
#include "test_vm.h"

// ========================================
// Jump Instructions Tests
// ========================================

int test_jmp(void)
{
    VM vm;
    init_test_vm(&vm);

    // Program: JMP 0x0100
    vm.romb[0] = OP_JMP;
    vm.romb[1] = 0x00; // addr low
    vm.romb[2] = 0x01; // addr high

    vm_step(&vm);

    ASSERT(vm.ip == 0x0100, "JMP should set IP to 0x0100, got 0x%04X", vm.ip);

    return 1;
}

int test_jz_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = FLAG_Z; // Set zero flag

    // Program: JZ 0x0050
    vm.romb[0] = OP_JZ;
    vm.romb[1] = 0x50; // addr low
    vm.romb[2] = 0x00; // addr high

    vm_step(&vm);

    ASSERT(vm.ip == 0x0050, "JZ should jump when Z flag is set, got 0x%04X", vm.ip);

    return 1;
}

int test_jz_not_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = 0; // Zero flag not set

    // Program: JZ 0x0050
    vm.romb[0] = OP_JZ;
    vm.romb[1] = 0x50; // addr low
    vm.romb[2] = 0x00; // addr high

    vm_step(&vm);

    ASSERT(vm.ip == 3, "JZ should not jump when Z flag is clear, got 0x%04X", vm.ip);

    return 1;
}

int test_jnz_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = 0; // Zero flag NOT set

    // Program: JNZ 0x0200
    vm.romb[0] = OP_JNZ;
    vm.romb[1] = 0x00; // addr low
    vm.romb[2] = 0x02; // addr high

    vm_step(&vm);

    ASSERT(vm.ip == 0x0200, "JNZ should jump when Z flag is clear, got 0x%04X", vm.ip);

    return 1;
}

int test_jnz_not_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = FLAG_Z; // Zero flag IS set

    // Program: JNZ 0x0200
    vm.romb[0] = OP_JNZ;
    vm.romb[1] = 0x00; // addr low
    vm.romb[2] = 0x02; // addr high

    vm_step(&vm);

    ASSERT(vm.ip == 3, "JNZ should not jump when Z flag is set, got 0x%04X", vm.ip);

    return 1;
}

int test_jc_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = FLAG_C; // Set carry flag

    // Program: JC 0x0300
    vm.romb[0] = OP_JC;
    vm.romb[1] = 0x00; // addr low
    vm.romb[2] = 0x03; // addr high

    vm_step(&vm);

    ASSERT(vm.ip == 0x0300, "JC should jump when C flag is set, got 0x%04X", vm.ip);

    return 1;
}

int test_jc_not_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = 0; // Carry flag NOT set

    // Program: JC 0x0300
    vm.romb[0] = OP_JC;
    vm.romb[1] = 0x00; // addr low
    vm.romb[2] = 0x03; // addr high

    vm_step(&vm);

    ASSERT(vm.ip == 3, "JC should not jump when C flag is clear, got 0x%04X", vm.ip);

    return 1;
}

int test_jnc_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = 0; // Carry flag NOT set

    // Program: JNC 0x0400
    vm.romb[0] = OP_JNC;
    vm.romb[1] = 0x00; // addr low
    vm.romb[2] = 0x04; // addr high

    vm_step(&vm);

    ASSERT(vm.ip == 0x0400, "JNC should jump when C flag is clear, got 0x%04X", vm.ip);

    return 1;
}

int test_jnc_not_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = FLAG_C; // Carry flag IS set

    // Program: JNC 0x0400
    vm.romb[0] = OP_JNC;
    vm.romb[1] = 0x00; // addr low
    vm.romb[2] = 0x04; // addr high

    vm_step(&vm);

    ASSERT(vm.ip == 3, "JNC should not jump when C flag is set, got 0x%04X", vm.ip);

    return 1;
}

int test_jlt_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    // Signed less-than: (N != V)
    vm.flags = FLAG_N; // N=1, V=0 -> signed less than

    // Program: JLT 0x0500
    vm.romb[0] = OP_JLT;
    vm.romb[1] = 0x00; // addr low
    vm.romb[2] = 0x05; // addr high

    vm_step(&vm);

    ASSERT(vm.ip == 0x0500, "JLT should jump when N!=V, got 0x%04X", vm.ip);

    return 1;
}

int test_jlt_not_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    // Signed greater-or-equal: (N == V)
    vm.flags = FLAG_N | FLAG_V; // N=1, V=1 -> N==V -> not less than

    // Program: JLT 0x0500
    vm.romb[0] = OP_JLT;
    vm.romb[1] = 0x00; // addr low
    vm.romb[2] = 0x05; // addr high

    vm_step(&vm);

    ASSERT(vm.ip == 3, "JLT should not jump when N==V, got 0x%04X", vm.ip);

    return 1;
}

int test_jge_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    // Signed greater-or-equal: (N == V)
    vm.flags = 0; // N=0, V=0 -> N==V -> signed >=

    // Program: JGE 0x0600
    vm.romb[0] = OP_JGE;
    vm.romb[1] = 0x00; // addr low
    vm.romb[2] = 0x06; // addr high

    vm_step(&vm);

    ASSERT(vm.ip == 0x0600, "JGE should jump when N==V, got 0x%04X", vm.ip);

    return 1;
}

int test_jge_not_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    // Signed less-than: (N != V)
    vm.flags = FLAG_V; // N=0, V=1 -> N!=V -> less than

    // Program: JGE 0x0600
    vm.romb[0] = OP_JGE;
    vm.romb[1] = 0x00; // addr low
    vm.romb[2] = 0x06; // addr high

    vm_step(&vm);

    ASSERT(vm.ip == 3, "JGE should not jump when N!=V, got 0x%04X", vm.ip);

    return 1;
}

int test_jmpr(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x0777;

    // Program: JMPR r0
    vm.romb[0] = OP_JMPR;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.ip == 0x0777, "JMPR should jump to address in r0, got 0x%04X", vm.ip);

    return 1;
}

int test_jzr(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = FLAG_Z;
    vm.reg[1] = 0x0888;

    // Program: JZR r1
    vm.romb[0] = OP_JZR;
    vm.romb[1] = 1; // r1

    vm_step(&vm);

    ASSERT(vm.ip == 0x0888, "JZR should jump when Z flag is set, got 0x%04X", vm.ip);

    return 1;
}

int test_jzr_not_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = 0; // Z flag not set
    vm.reg[1] = 0x0888;

    // Program: JZR r1
    vm.romb[0] = OP_JZR;
    vm.romb[1] = 1; // r1

    vm_step(&vm);

    ASSERT(vm.ip == 2, "JZR should not jump when Z flag is clear, got 0x%04X", vm.ip);

    return 1;
}

int test_jnzr_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = 0; // Z flag not set
    vm.reg[2] = 0x0999;

    // Program: JNZR r2
    vm.romb[0] = OP_JNZR;
    vm.romb[1] = 2; // r2

    vm_step(&vm);

    ASSERT(vm.ip == 0x0999, "JNZR should jump when Z flag is clear, got 0x%04X", vm.ip);

    return 1;
}

int test_jnzr_not_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = FLAG_Z; // Z flag set
    vm.reg[2] = 0x0999;

    // Program: JNZR r2
    vm.romb[0] = OP_JNZR;
    vm.romb[1] = 2; // r2

    vm_step(&vm);

    ASSERT(vm.ip == 2, "JNZR should not jump when Z flag is set, got 0x%04X", vm.ip);

    return 1;
}

int test_jcr_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = FLAG_C; // C flag set
    vm.reg[0] = 0x0AAA;

    // Program: JCR r0
    vm.romb[0] = OP_JCR;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.ip == 0x0AAA, "JCR should jump when C flag is set, got 0x%04X", vm.ip);

    return 1;
}

int test_jcr_not_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = 0; // C flag not set
    vm.reg[0] = 0x0AAA;

    // Program: JCR r0
    vm.romb[0] = OP_JCR;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.ip == 2, "JCR should not jump when C flag is clear, got 0x%04X", vm.ip);

    return 1;
}

int test_jncr_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = 0; // C flag not set
    vm.reg[1] = 0x0BBB;

    // Program: JNCR r1
    vm.romb[0] = OP_JNCR;
    vm.romb[1] = 1; // r1

    vm_step(&vm);

    ASSERT(vm.ip == 0x0BBB, "JNCR should jump when C flag is clear, got 0x%04X", vm.ip);

    return 1;
}

int test_jncr_not_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = FLAG_C; // C flag set
    vm.reg[1] = 0x0BBB;

    // Program: JNCR r1
    vm.romb[0] = OP_JNCR;
    vm.romb[1] = 1; // r1

    vm_step(&vm);

    ASSERT(vm.ip == 2, "JNCR should not jump when C flag is set, got 0x%04X", vm.ip);

    return 1;
}

int test_jltr_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = FLAG_N; // N=1, V=0 -> N!=V -> signed less than
    vm.reg[2] = 0x0CCC;

    // Program: JLTR r2
    vm.romb[0] = OP_JLTR;
    vm.romb[1] = 2; // r2

    vm_step(&vm);

    ASSERT(vm.ip == 0x0CCC, "JLTR should jump when N!=V, got 0x%04X", vm.ip);

    return 1;
}

int test_jltr_not_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = 0; // N=0, V=0 -> N==V -> not less than
    vm.reg[2] = 0x0CCC;

    // Program: JLTR r2
    vm.romb[0] = OP_JLTR;
    vm.romb[1] = 2; // r2

    vm_step(&vm);

    ASSERT(vm.ip == 2, "JLTR should not jump when N==V, got 0x%04X", vm.ip);

    return 1;
}

int test_jger_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = FLAG_N | FLAG_V; // N=1, V=1 -> N==V -> signed >=
    vm.reg[3] = 0x0DDD;

    // Program: JGER r3
    vm.romb[0] = OP_JGER;
    vm.romb[1] = 3; // r3

    vm_step(&vm);

    ASSERT(vm.ip == 0x0DDD, "JGER should jump when N==V, got 0x%04X", vm.ip);

    return 1;
}

int test_jger_not_taken(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.flags = FLAG_V; // N=0, V=1 -> N!=V -> less than
    vm.reg[3] = 0x0DDD;

    // Program: JGER r3
    vm.romb[0] = OP_JGER;
    vm.romb[1] = 3; // r3

    vm_step(&vm);

    ASSERT(vm.ip == 2, "JGER should not jump when N!=V, got 0x%04X", vm.ip);

    return 1;
}

// ========================================
// Call/Return Tests
// ========================================

int test_call_ret(void)
{
    VM vm;
    init_test_vm(&vm);

    uint16_t sp_before = vm.sp;

    // Program at 0x0000: CALL 0x0100
    vm.romb[0] = OP_CALL;
    vm.romb[1] = 0x00; // addr low
    vm.romb[2] = 0x01; // addr high

    // Program at 0x0100: RET
    vm.romb[0x0100] = OP_RET;

    vm_step(&vm); // Execute CALL

    ASSERT(vm.ip == 0x0100, "CALL should jump to 0x0100, got 0x%04X", vm.ip);
    ASSERT(vm.sp == sp_before - 2, "CALL should push return address (2 bytes)");

    // Check return address on stack (little-endian)
    uint16_t return_addr = (uint16_t)(vm.ram[sp_before - 1] | (vm.ram[sp_before] << 8));
    ASSERT(return_addr == 3, "Return address should be 3 (after CALL), got 0x%04X", return_addr);

    vm_step(&vm); // Execute RET

    ASSERT(vm.ip == 3, "RET should return to address 3, got 0x%04X", vm.ip);
    ASSERT(vm.sp == sp_before, "RET should restore SP");

    return 1;
}

int test_callr(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[2] = 0x0200;
    uint16_t sp_before = vm.sp;

    // Program: CALLR r2
    vm.romb[0] = OP_CALLR;
    vm.romb[1] = 2; // r2

    // At 0x0200: RET
    vm.romb[0x0200] = OP_RET;

    vm_step(&vm); // Execute CALLR

    ASSERT(vm.ip == 0x0200, "CALLR should jump to address in r2, got 0x%04X", vm.ip);
    ASSERT(vm.sp == sp_before - 2, "CALLR should push return address");

    vm_step(&vm); // Execute RET

    ASSERT(vm.ip == 2, "RET should return after CALLR, got 0x%04X", vm.ip);

    return 1;
}

// ========================================
// Complex Control Flow Test
// ========================================

int test_loop(void)
{
    VM vm;
    init_test_vm(&vm);

    // Simple loop: increment r0 from 0 to 5
    // Program:
    // 0x0000: LDI r0, 0        ; r0 = 0
    // 0x0003: LDI r1, 5        ; r1 = 5 (target)
    // 0x0006: INC r0           ; r0++
    // 0x0008: CMPR r0, r1      ; compare r0 with r1
    // 0x000A: JNZ 0x0006       ; if not equal, loop
    // 0x000D: HLT

    vm.romb[0x0000] = OP_LDI;
    vm.romb[0x0001] = 0; // r0
    vm.romb[0x0002] = 0; // 0

    vm.romb[0x0003] = OP_LDI;
    vm.romb[0x0004] = 1; // r1
    vm.romb[0x0005] = 5; // 5

    vm.romb[0x0006] = OP_INC;
    vm.romb[0x0007] = 0; // r0

    vm.romb[0x0008] = OP_CMPR;
    vm.romb[0x0009] = 0; // r0
    vm.romb[0x000A] = 1; // r1

    vm.romb[0x000B] = OP_JNZ;
    vm.romb[0x000C] = 0x06; // back to INC
    vm.romb[0x000D] = 0x00;

    vm.romb[0x000E] = OP_HLT;

    StepResult res = vm_execute_until_halt(&vm, 100);

    ASSERT(res == STEP_HALT, "Loop should complete with HALT");
    ASSERT(vm.reg[0] == 5, "r0 should be 5 after loop, got 0x%04X", vm.reg[0]);
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set after final compare");

    return 1;
}

// ========================================
// Main test runner
// ========================================

int main(void)
{
    printf("\n========================================\n");
    printf("GINEC VM - Control Flow Tests\n");
    printf("========================================\n\n");

    // Immediate jumps
    run_test("JMP", test_jmp);
    run_test("JZ (taken)", test_jz_taken);
    run_test("JZ (not taken)", test_jz_not_taken);
    run_test("JNZ (taken)", test_jnz_taken);
    run_test("JNZ (not taken)", test_jnz_not_taken);
    run_test("JC (taken)", test_jc_taken);
    run_test("JC (not taken)", test_jc_not_taken);
    run_test("JNC (taken)", test_jnc_taken);
    run_test("JNC (not taken)", test_jnc_not_taken);
    run_test("JLT (taken)", test_jlt_taken);
    run_test("JLT (not taken)", test_jlt_not_taken);
    run_test("JGE (taken)", test_jge_taken);
    run_test("JGE (not taken)", test_jge_not_taken);

    // Register jumps
    run_test("JMPR", test_jmpr);
    run_test("JZR (taken)", test_jzr);
    run_test("JZR (not taken)", test_jzr_not_taken);
    run_test("JNZR (taken)", test_jnzr_taken);
    run_test("JNZR (not taken)", test_jnzr_not_taken);
    run_test("JCR (taken)", test_jcr_taken);
    run_test("JCR (not taken)", test_jcr_not_taken);
    run_test("JNCR (taken)", test_jncr_taken);
    run_test("JNCR (not taken)", test_jncr_not_taken);
    run_test("JLTR (taken)", test_jltr_taken);
    run_test("JLTR (not taken)", test_jltr_not_taken);
    run_test("JGER (taken)", test_jger_taken);
    run_test("JGER (not taken)", test_jger_not_taken);

    // Calls
    run_test("CALL/RET", test_call_ret);
    run_test("CALLR", test_callr);

    // Complex
    run_test("Loop test", test_loop);

    print_summary();

    return tests_failed == 0 ? 0 : 1;
}
