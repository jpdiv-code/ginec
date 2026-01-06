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

int test_fill(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup registers
    vm.reg[0] = 0x1000; // RA - address
    vm.reg[1] = 0xAB42; // RP - pattern (low byte = 0x42)
    vm.reg[2] = 0x0005; // RL - length

    // Program: FILL r0, r1, r2
    vm.romb[0] = OP_FILL;
    vm.romb[1] = 0; // RA
    vm.romb[2] = 1; // RP
    vm.romb[3] = 2; // RL

    vm_step(&vm);

    // Check that 5 bytes are filled with 0x42
    for (int i = 0; i < 5; i++)
    {
        ASSERT(vm.ram[0x1000 + i] == 0x42, "ram[0x%04X] should be 0x42, got 0x%02X", 0x1000 + i,
               vm.ram[0x1000 + i]);
    }

    return 1;
}

int test_fillw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup registers
    vm.reg[0] = 0x2000; // RA - address
    vm.reg[1] = 0x1234; // RP - pattern
    vm.reg[2] = 0x0003; // RL - length (3 words = 6 bytes)

    // Program: FILL.w r0, r1, r2
    vm.romb[0] = OP_FILLw;
    vm.romb[1] = 0; // RA
    vm.romb[2] = 1; // RP
    vm.romb[3] = 2; // RL

    vm_step(&vm);

    // Check that 3 words are filled with 0x1234 (little-endian)
    for (int i = 0; i < 3; i++)
    {
        uint16_t addr = (uint16_t)(0x2000 + i * 2);
        uint16_t value = (uint16_t)(vm.ram[addr] | (vm.ram[addr + 1] << 8));
        ASSERT(value == 0x1234, "Word at 0x%04X should be 0x1234, got 0x%04X", addr, value);
    }

    return 1;
}

int test_seed_rand(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup: seed with a fixed value
    vm.reg[0] = 0x0042; // RS - seed value

    // Program: SEED r0, RAND r1, RAND r2
    vm.romb[0] = OP_SEED;
    vm.romb[1] = 0; // RS
    vm.romb[2] = OP_RAND;
    vm.romb[3] = 1; // RD
    vm.romb[4] = OP_RAND;
    vm.romb[5] = 2; // RD

    vm.reg[1] = 0xFF00;
    vm.reg[2] = 0xFF00;

    vm_execute_steps(&vm, 3);

    // Check that random values were generated (low bytes changed)
    uint8_t rand1 = vm.reg[1] & 0xFF;
    uint8_t rand2 = vm.reg[2] & 0xFF;

    ASSERT((vm.reg[1] & 0xFF00) == 0xFF00, "RAND should preserve high byte");
    ASSERT((vm.reg[2] & 0xFF00) == 0xFF00, "RAND should preserve high byte");

    // With same seed, sequence should be deterministic
    VM vm2;
    init_test_vm(&vm2);
    vm2.reg[0] = 0x0042;
    vm2.romb[0] = OP_SEED;
    vm2.romb[1] = 0;
    vm2.romb[2] = OP_RAND;
    vm2.romb[3] = 1;
    vm2.romb[4] = OP_RAND;
    vm2.romb[5] = 2;
    vm2.reg[1] = 0xFF00;
    vm2.reg[2] = 0xFF00;
    vm_execute_steps(&vm2, 3);

    ASSERT((vm2.reg[1] & 0xFF) == rand1, "Same seed should produce same random sequence");
    ASSERT((vm2.reg[2] & 0xFF) == rand2, "Same seed should produce same random sequence");

    return 1;
}

int test_randw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup: seed with a fixed value
    vm.reg[0] = 0x1234; // RS - seed value

    // Program: SEED r0, RAND.w r1, RAND.w r2
    vm.romb[0] = OP_SEED;
    vm.romb[1] = 0; // RS
    vm.romb[2] = OP_RANDw;
    vm.romb[3] = 1; // RD
    vm.romb[4] = OP_RANDw;
    vm.romb[5] = 2; // RD

    vm_execute_steps(&vm, 3);

    uint16_t rand1 = vm.reg[1];
    uint16_t rand2 = vm.reg[2];

    // With same seed, sequence should be deterministic
    VM vm2;
    init_test_vm(&vm2);
    vm2.reg[0] = 0x1234;
    vm2.romb[0] = OP_SEED;
    vm2.romb[1] = 0;
    vm2.romb[2] = OP_RANDw;
    vm2.romb[3] = 1;
    vm2.romb[4] = OP_RANDw;
    vm2.romb[5] = 2;
    vm_execute_steps(&vm2, 3);

    ASSERT(vm2.reg[1] == rand1, "Same seed should produce same 16-bit random sequence");
    ASSERT(vm2.reg[2] == rand2, "Same seed should produce same 16-bit random sequence");

    return 1;
}

// ========================================
// Main test runner
// ========================================

int main(void)
{
    printf("\n========================================\n");
    printf("GINEC VM - OTHER Instructions Tests\n");
    printf("========================================\n\n");

    run_test("NOP", test_nop);
    run_test("HLT", test_hlt);
    run_test("SYNC", test_sync);
    run_test("FILL", test_fill);
    run_test("FILL.w", test_fillw);
    run_test("SEED/RAND", test_seed_rand);
    run_test("RAND.w", test_randw);

    print_summary();

    return tests_failed == 0 ? 0 : 1;
}
