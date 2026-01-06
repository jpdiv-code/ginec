#include "opcodes.h"
#include "test_framework.h"
#include "test_vm.h"

// ========================================
// RAM Instructions Tests
// ========================================

int test_ld_st(void)
{
    VM vm;
    init_test_vm(&vm);

    // Store 0x77 at RAM address 0x1000
    vm.ram[0x1000] = 0x77;

    // Program: LD r0, 0x1000
    vm.romb[0] = OP_LD;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x00; // addr low
    vm.romb[3] = 0x10; // addr high

    vm.reg[0] = 0xFFFF;
    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF77, "LD should load 0x77 into low byte, got 0x%04X", vm.reg[0]);

    // Now test ST: Store low byte of r1 at 0x2000
    vm.reg[1] = 0xAB42;

    vm.romb[4] = OP_ST;
    vm.romb[5] = 0x00; // addr low
    vm.romb[6] = 0x20; // addr high
    vm.romb[7] = 1;    // r1

    vm.ip = 4;
    vm_step(&vm);

    ASSERT(vm.ram[0x2000] == 0x42, "ST should store 0x42 at 0x2000, got 0x%02X", vm.ram[0x2000]);

    return 1;
}

int test_ldw_stw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Store 0x1234 at RAM address 0x3000 (little-endian)
    vm.ram[0x3000] = 0x34;
    vm.ram[0x3001] = 0x12;

    // Program: LD.w r0, 0x3000
    vm.romb[0] = OP_LDw;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x00; // addr low
    vm.romb[3] = 0x30; // addr high

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x1234, "LD.w should load 0x1234, got 0x%04X", vm.reg[0]);

    // Now test ST.w: Store r2 at 0x4000
    vm.reg[2] = 0xABCD;

    vm.romb[4] = OP_STw;
    vm.romb[5] = 0x00; // addr low
    vm.romb[6] = 0x40; // addr high
    vm.romb[7] = 2;    // r2

    vm.ip = 4;
    vm_step(&vm);

    ASSERT(vm.ram[0x4000] == 0xCD, "ST.w low byte should be 0xCD, got 0x%02X", vm.ram[0x4000]);
    ASSERT(vm.ram[0x4001] == 0xAB, "ST.w high byte should be 0xAB, got 0x%02X", vm.ram[0x4001]);

    return 1;
}

int test_ldr_str(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup: address in r0
    vm.reg[0] = 0x5000;
    vm.ram[0x5000] = 0x88;

    // Program: LDR r1, r0
    vm.romb[0] = OP_LDR;
    vm.romb[1] = 1; // rd = r1
    vm.romb[2] = 0; // ra = r0

    vm.reg[1] = 0xFFFF;
    vm_step(&vm);

    ASSERT(vm.reg[1] == 0xFF88, "LDR should load 0x88, got 0x%04X", vm.reg[1]);

    // Test STR: store r2 at address in r3
    vm.reg[3] = 0x6000;
    vm.reg[2] = 0xAB99;

    vm.romb[3] = OP_STR;
    vm.romb[4] = 3; // ra = r3
    vm.romb[5] = 2; // rs = r2

    vm.ip = 3;
    vm_step(&vm);

    ASSERT(vm.ram[0x6000] == 0x99, "STR should store 0x99, got 0x%02X", vm.ram[0x6000]);

    return 1;
}

int test_ldri_stri(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup: base address in r0
    vm.reg[0] = 0x7000;
    vm.ram[0x7005] = 0xCC; // offset +5

    // Program: LDRI r1, r0, 5
    vm.romb[0] = OP_LDRI;
    vm.romb[1] = 1; // rd = r1
    vm.romb[2] = 0; // ra = r0
    vm.romb[3] = 5; // offset

    vm.reg[1] = 0xFFFF;
    vm_step(&vm);

    ASSERT(vm.reg[1] == 0xFFCC, "LDRI should load 0xCC, got 0x%04X", vm.reg[1]);

    // Test STRI: store at base + offset
    vm.reg[2] = 0x8000;
    vm.reg[3] = 0xAB55;

    vm.romb[4] = OP_STRI;
    vm.romb[5] = 2;  // ra = r2
    vm.romb[6] = 10; // offset
    vm.romb[7] = 3;  // rs = r3

    vm.ip = 4;
    vm_step(&vm);

    ASSERT(vm.ram[0x800A] == 0x55, "STRI should store 0x55 at 0x800A, got 0x%02X", vm.ram[0x800A]);

    return 1;
}

// ========================================
// ROMA Instructions Tests
// ========================================

int test_lda(void)
{
    VM vm;
    init_test_vm(&vm);

    // Put data in ROMA
    vm.roma[0x0100] = 0xAA;

    // Program: LDA r0, 0x0100
    vm.romb[0] = OP_LDA;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x00; // addr low
    vm.romb[3] = 0x01; // addr high

    vm.reg[0] = 0xFFFF;
    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFFAA, "LDA should load 0xAA from ROMA, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_ldaw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Put data in ROMA (little-endian)
    vm.roma[0x0200] = 0x34;
    vm.roma[0x0201] = 0x12;

    // Program: LDA.w r1, 0x0200
    vm.romb[0] = OP_LDAw;
    vm.romb[1] = 1;    // r1
    vm.romb[2] = 0x00; // addr low
    vm.romb[3] = 0x02; // addr high

    vm_step(&vm);

    ASSERT(vm.reg[1] == 0x1234, "LDA.w should load 0x1234 from ROMA, got 0x%04X", vm.reg[1]);

    return 1;
}

int test_ldar(void)
{
    VM vm;
    init_test_vm(&vm);

    // Put data in ROMA
    vm.roma[0x0300] = 0xBB;

    // Setup: address in r2
    vm.reg[2] = 0x0300;

    // Program: LDAR r3, r2
    vm.romb[0] = OP_LDAR;
    vm.romb[1] = 3; // rd = r3
    vm.romb[2] = 2; // ra = r2

    vm.reg[3] = 0xFFFF;
    vm_step(&vm);

    ASSERT(vm.reg[3] == 0xFFBB, "LDAR should load 0xBB, got 0x%04X", vm.reg[3]);

    return 1;
}

// ========================================
// Main test runner
// ========================================

int main(void)
{
    printf("\n========================================\n");
    printf("GINEC VM - Memory Instruction Tests\n");
    printf("========================================\n\n");

    // RAM
    run_test("LD/ST", test_ld_st);
    run_test("LD.w/ST.w", test_ldw_stw);
    run_test("LDR/STR", test_ldr_str);
    run_test("LDRI/STRI", test_ldri_stri);

    // ROMA
    run_test("LDA", test_lda);
    run_test("LDA.w", test_ldaw);
    run_test("LDAR", test_ldar);

    print_summary();

    return tests_failed == 0 ? 0 : 1;
}
