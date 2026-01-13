#include "opcodes.h"
#include "test_framework.h"
#include "test_vm.h"

// ========================================
// Arithmetic Instructions Tests
// ========================================

// ADD variants

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

    ASSERT(vm.reg[0] == 0xFF15, "ADD: 0x10 + 0x05 = 0x15, high byte preserved, got 0x%04X",
           vm.reg[0]);
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

    ASSERT(vm.reg[1] == 0xAB01,
           "ADD with carry: 0xFF + 0x02 = 0x01, high byte preserved, got 0x%04X", vm.reg[1]);
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
    ASSERT(NO_FLAG(&vm, FLAG_Z), "Zero flag should not be set");

    return 1;
}

int test_addw_overflow(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFFFF;

    // Program: ADD.w r0, 0x0002
    vm.romb[0] = OP_ADDw;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x02; // imm16 low
    vm.romb[3] = 0x00; // imm16 high

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x0001, "ADD.w with overflow: 0xFFFF + 0x0002 = 0x0001, got 0x%04X",
           vm.reg[0]);
    ASSERT(HAS_FLAG(&vm, FLAG_C), "Carry flag should be set");

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

    ASSERT(vm.reg[0] == 0xFF30, "ADDR: 0x10 + 0x20 = 0x30, high byte preserved, got 0x%04X",
           vm.reg[0]);

    return 1;
}

int test_addrw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x1234;
    vm.reg[1] = 0x5678;

    // Program: ADDR.w r0, r1
    vm.romb[0] = OP_ADDRw;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x68AC, "ADDR.w: 0x1234 + 0x5678 = 0x68AC, got 0x%04X", vm.reg[0]);

    return 1;
}

// SUB variants

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

    ASSERT(vm.reg[0] == 0xFF10, "SUB: 0x20 - 0x10 = 0x10, high byte preserved, got 0x%04X",
           vm.reg[0]);
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

    ASSERT(vm.reg[1] == 0xABF5,
           "SUB with borrow: 0x05 - 0x10 = 0xF5, high byte preserved, got 0x%04X", vm.reg[1]);
    ASSERT(HAS_FLAG(&vm, FLAG_C), "Carry flag should be set (borrow occurred)");
    ASSERT(HAS_FLAG(&vm, FLAG_N), "Negative flag should be set");

    return 1;
}

int test_subw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x5000;

    // Program: SUB.w r0, 0x1234
    vm.romb[0] = OP_SUBw;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x34; // imm16 low
    vm.romb[3] = 0x12; // imm16 high

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x3DCC, "SUB.w: 0x5000 - 0x1234 = 0x3DCC, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_subr(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF50;
    vm.reg[1] = 0xAB20;

    // Program: SUBR r0, r1
    vm.romb[0] = OP_SUBR;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF30, "SUBR: 0x50 - 0x20 = 0x30, high byte preserved, got 0x%04X",
           vm.reg[0]);

    return 1;
}

int test_subrw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x5678;
    vm.reg[1] = 0x1234;

    // Program: SUBR.w r0, r1
    vm.romb[0] = OP_SUBRw;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x4444, "SUBR.w: 0x5678 - 0x1234 = 0x4444, got 0x%04X", vm.reg[0]);

    return 1;
}

// MUL variants

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

    ASSERT(vm.reg[0] == 0xFF0F, "MUL: 0x05 * 0x03 = 0x0F, high byte preserved, got 0x%04X",
           vm.reg[0]);

    return 1;
}

int test_mul_overflow(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xAB80;

    // Program: MUL r0, 0x02  (0x80 * 0x02 = 0x100, wraps to 0x00)
    vm.romb[0] = OP_MUL;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x02; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xAB00,
           "MUL overflow: 0x80 * 0x02 = 0x00 (8-bit), high byte preserved, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_mulw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x0010;

    // Program: MUL.w r0, 0x0100
    vm.romb[0] = OP_MULw;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x00; // imm16 low
    vm.romb[3] = 0x01; // imm16 high

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x1000, "MUL.w: 0x0010 * 0x0100 = 0x1000, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_mulr(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF07;
    vm.reg[1] = 0xAB05;

    // Program: MULR r0, r1
    vm.romb[0] = OP_MULR;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF23, "MULR: 0x07 * 0x05 = 0x23, high byte preserved, got 0x%04X",
           vm.reg[0]);

    return 1;
}

int test_mulrw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x0010;
    vm.reg[1] = 0x0100;

    // Program: MULR.w r0, r1
    vm.romb[0] = OP_MULRw;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x1000, "MULR.w: 0x0010 * 0x0100 = 0x1000, got 0x%04X", vm.reg[0]);

    return 1;
}

// INC/DEC variants

int test_inc(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF42;

    // Program: INC r0
    vm.romb[0] = OP_INC;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF43, "INC: 0x42 + 1 = 0x43, high byte preserved, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_inc_overflow(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xABFF;

    // Program: INC r0
    vm.romb[0] = OP_INC;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xAB00, "INC overflow: 0xFF + 1 = 0x00, high byte preserved, got 0x%04X",
           vm.reg[0]);
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set");
    ASSERT(HAS_FLAG(&vm, FLAG_C), "Carry flag should be set");

    return 1;
}

int test_incw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x1234;

    // Program: INC.w r0
    vm.romb[0] = OP_INCw;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x1235, "INC.w: 0x1234 + 1 = 0x1235, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_incw_overflow(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFFFF;

    // Program: INC.w r0
    vm.romb[0] = OP_INCw;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x0000, "INC.w overflow: 0xFFFF + 1 = 0x0000, got 0x%04X", vm.reg[0]);
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set");
    ASSERT(HAS_FLAG(&vm, FLAG_C), "Carry flag should be set");

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

    ASSERT(vm.reg[1] == 0xAB0F, "DEC: 0x10 - 1 = 0x0F, high byte preserved, got 0x%04X", vm.reg[1]);

    return 1;
}

int test_dec_underflow(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[1] = 0xCD00;

    // Program: DEC r1
    vm.romb[0] = OP_DEC;
    vm.romb[1] = 1; // r1

    vm_step(&vm);

    ASSERT(vm.reg[1] == 0xCDFF, "DEC underflow: 0x00 - 1 = 0xFF, high byte preserved, got 0x%04X",
           vm.reg[1]);
    ASSERT(HAS_FLAG(&vm, FLAG_C), "Carry flag should be set (borrow)");
    ASSERT(HAS_FLAG(&vm, FLAG_N), "Negative flag should be set");

    return 1;
}

int test_decw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x1234;

    // Program: DEC.w r0
    vm.romb[0] = OP_DECw;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x1233, "DEC.w: 0x1234 - 1 = 0x1233, got 0x%04X", vm.reg[0]);

    return 1;
}

int test_decw_underflow(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x0000;

    // Program: DEC.w r0
    vm.romb[0] = OP_DECw;
    vm.romb[1] = 0; // r0

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFFFF, "DEC.w underflow: 0x0000 - 1 = 0xFFFF, got 0x%04X", vm.reg[0]);
    ASSERT(HAS_FLAG(&vm, FLAG_C), "Carry flag should be set (borrow)");
    ASSERT(HAS_FLAG(&vm, FLAG_N), "Negative flag should be set");

    return 1;
}

// CMP variants

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

int test_cmp_less(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xAB10;

    // Program: CMP r0, 0x20  (0x10 < 0x20)
    vm.romb[0] = OP_CMP;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x20; // imm8

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xAB10, "CMP should not modify register");
    ASSERT(HAS_FLAG(&vm, FLAG_C), "Carry flag should be set (less than)");
    ASSERT(NO_FLAG(&vm, FLAG_Z), "Zero flag should not be set");

    return 1;
}

int test_cmpw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x1234;

    // Program: CMP.w r0, 0x1234
    vm.romb[0] = OP_CMPw;
    vm.romb[1] = 0;    // r0
    vm.romb[2] = 0x34; // imm16 low
    vm.romb[3] = 0x12; // imm16 high

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x1234, "CMP.w should not modify register");
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set (equal)");

    return 1;
}

int test_cmpr(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0xFF42;
    vm.reg[1] = 0xAB42;

    // Program: CMPR r0, r1
    vm.romb[0] = OP_CMPR;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0xFF42, "CMPR should not modify register");
    ASSERT(HAS_FLAG(&vm, FLAG_Z), "Zero flag should be set (equal low bytes)");

    return 1;
}

int test_cmprw(void)
{
    VM vm;
    init_test_vm(&vm);

    vm.reg[0] = 0x5678;
    vm.reg[1] = 0x1234;

    // Program: CMPR.w r0, r1  (0x5678 > 0x1234)
    vm.romb[0] = OP_CMPRw;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1

    vm_step(&vm);

    ASSERT(vm.reg[0] == 0x5678, "CMPR.w should not modify register");
    ASSERT(NO_FLAG(&vm, FLAG_C), "Carry flag should not be set (greater than)");
    ASSERT(NO_FLAG(&vm, FLAG_Z), "Zero flag should not be set");

    return 1;
}

// ========================================
// Main test runner
// ========================================

int main(void)
{
    printf("\n========================================\n");
    printf("GINEC VM - Arithmetic Instructions Tests\n");
    printf("========================================\n\n");

    // ADD variants
    run_test("ADD", test_add);
    run_test("ADD with carry", test_add_with_carry);
    run_test("ADD zero flag", test_add_zero);
    run_test("ADD.w", test_addw);
    run_test("ADD.w overflow", test_addw_overflow);
    run_test("ADDR", test_addr);
    run_test("ADDR.w", test_addrw);

    // SUB variants
    run_test("SUB", test_sub);
    run_test("SUB with borrow", test_sub_with_borrow);
    run_test("SUB.w", test_subw);
    run_test("SUBR", test_subr);
    run_test("SUBR.w", test_subrw);

    // MUL variants
    run_test("MUL", test_mul);
    run_test("MUL overflow", test_mul_overflow);
    run_test("MUL.w", test_mulw);
    run_test("MULR", test_mulr);
    run_test("MULR.w", test_mulrw);

    // INC/DEC variants
    run_test("INC", test_inc);
    run_test("INC overflow", test_inc_overflow);
    run_test("INC.w", test_incw);
    run_test("INC.w overflow", test_incw_overflow);
    run_test("DEC", test_dec);
    run_test("DEC underflow", test_dec_underflow);
    run_test("DEC.w", test_decw);
    run_test("DEC.w underflow", test_decw_underflow);

    // CMP variants
    run_test("CMP", test_cmp);
    run_test("CMP less than", test_cmp_less);
    run_test("CMP.w", test_cmpw);
    run_test("CMPR", test_cmpr);
    run_test("CMPR.w", test_cmprw);

    print_summary();

    return tests_failed == 0 ? 0 : 1;
}
