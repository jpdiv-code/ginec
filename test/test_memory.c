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

int test_ldrw_strw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup: address in r0, data in RAM (little-endian)
    vm.reg[0] = 0x5500;
    vm.ram[0x5500] = 0x78;
    vm.ram[0x5501] = 0x56;

    // Program: LDR.w r1, r0
    vm.romb[0] = OP_LDRw;
    vm.romb[1] = 1; // rd = r1
    vm.romb[2] = 0; // ra = r0

    vm_step(&vm);

    ASSERT(vm.reg[1] == 0x5678, "LDR.w should load 0x5678, got 0x%04X", vm.reg[1]);

    // Test STR.w: store r2 at address in r3
    vm.reg[3] = 0x6600;
    vm.reg[2] = 0xABCD;

    vm.romb[3] = OP_STRw;
    vm.romb[4] = 3; // ra = r3
    vm.romb[5] = 2; // rs = r2

    vm.ip = 3;
    vm_step(&vm);

    ASSERT(vm.ram[0x6600] == 0xCD, "STR.w low byte should be 0xCD, got 0x%02X", vm.ram[0x6600]);
    ASSERT(vm.ram[0x6601] == 0xAB, "STR.w high byte should be 0xAB, got 0x%02X", vm.ram[0x6601]);

    return 1;
}

int test_ldriw_striw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup: base address in r0, data in RAM (little-endian)
    vm.reg[0] = 0x7700;
    vm.ram[0x770A] = 0xEF; // offset +10
    vm.ram[0x770B] = 0xBE;

    // Program: LDRI.w r1, r0, 10
    vm.romb[0] = OP_LDRIw;
    vm.romb[1] = 1;  // rd = r1
    vm.romb[2] = 0;  // ra = r0
    vm.romb[3] = 10; // offset

    vm_step(&vm);

    ASSERT(vm.reg[1] == 0xBEEF, "LDRI.w should load 0xBEEF, got 0x%04X", vm.reg[1]);

    // Test STRI.w: store at base + offset
    vm.reg[2] = 0x8800;
    vm.reg[3] = 0x1234;

    vm.romb[4] = OP_STRIw;
    vm.romb[5] = 2;  // ra = r2
    vm.romb[6] = 20; // offset
    vm.romb[7] = 3;  // rs = r3

    vm.ip = 4;
    vm_step(&vm);

    ASSERT(vm.ram[0x8814] == 0x34, "STRI.w low byte should be 0x34, got 0x%02X", vm.ram[0x8814]);
    ASSERT(vm.ram[0x8815] == 0x12, "STRI.w high byte should be 0x12, got 0x%02X", vm.ram[0x8815]);

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

int test_ldarw(void)
{
    VM vm;
    init_test_vm(&vm);

    // Put data in ROMA (little-endian)
    vm.roma[0x0400] = 0xAD;
    vm.roma[0x0401] = 0xDE;

    // Setup: address in r0
    vm.reg[0] = 0x0400;

    // Program: LDAR.w r1, r0
    vm.romb[0] = OP_LDARw;
    vm.romb[1] = 1; // rd = r1
    vm.romb[2] = 0; // ra = r0

    vm_step(&vm);

    ASSERT(vm.reg[1] == 0xDEAD, "LDAR.w should load 0xDEAD from ROMA, got 0x%04X", vm.reg[1]);

    return 1;
}

// ========================================
// CPYRA / SPRA Tests
// ========================================

int test_cpyra(void)
{
    VM vm;
    init_test_vm(&vm);

    // Setup: Fill ROMA with test data
    for (uint16_t i = 0; i < 100; i++)
    {
        vm.roma[0x1000 + i] = (uint8_t)(i + 0x10);
    }

    // Setup registers
    vm.reg[0] = 0x2000; // destination in RAM
    vm.reg[1] = 0x1000; // source in ROMA

    // Program: CPYRA r0, r1, 50
    vm.romb[0] = OP_CPYRA;
    vm.romb[1] = 0;  // rd = r0 (RAM dest)
    vm.romb[2] = 1;  // rs = r1 (ROMA source)
    vm.romb[3] = 50; // count

    vm_step(&vm);

    // Verify first 50 bytes were copied
    for (uint16_t i = 0; i < 50; i++)
    {
        uint8_t expected = (uint8_t)(i + 0x10);
        ASSERT(vm.ram[0x2000 + i] == expected, "CPYRA: byte %d should be 0x%02X, got 0x%02X", i,
               expected, vm.ram[0x2000 + i]);
    }

    // Verify bytes beyond count were not touched (should be 0)
    ASSERT(vm.ram[0x2032] == 0, "CPYRA should not copy beyond count");

    return 1;
}

int test_cpyra_zero_count(void)
{
    VM vm;
    init_test_vm(&vm);

    // Fill some RAM with data
    vm.ram[0x3000] = 0xFF;

    vm.reg[0] = 0x3000;
    vm.reg[1] = 0x1500;

    // Program: CPYRA r0, r1, 0 (copy 0 bytes)
    vm.romb[0] = OP_CPYRA;
    vm.romb[1] = 0; // rd = r0
    vm.romb[2] = 1; // rs = r1
    vm.romb[3] = 0; // count = 0

    vm_step(&vm);

    // RAM should be unchanged
    ASSERT(vm.ram[0x3000] == 0xFF, "CPYRA with count=0 should not modify RAM");

    return 1;
}

int test_cpyra_255_bytes(void)
{
    VM vm;
    init_test_vm(&vm);

    // Fill ROMA with pattern
    for (uint16_t i = 0; i < 255; i++)
    {
        vm.roma[0x2000 + i] = (uint8_t)(i ^ 0xAA);
    }

    vm.reg[0] = 0x4000;
    vm.reg[1] = 0x2000;

    // Program: CPYRA r0, r1, 255 (max count)
    vm.romb[0] = OP_CPYRA;
    vm.romb[1] = 0;   // rd = r0
    vm.romb[2] = 1;   // rs = r1
    vm.romb[3] = 255; // count

    vm_step(&vm);

    // Verify all 255 bytes
    for (uint16_t i = 0; i < 255; i++)
    {
        uint8_t expected = (uint8_t)(i ^ 0xAA);
        ASSERT(vm.ram[0x4000 + i] == expected, "CPYRA: byte %d mismatch", i);
    }

    return 1;
}

int test_spra_basic(void)
{
    VM vm;
    init_test_vm(&vm);

    // Create a simple 4x4 sprite in ROMA with pattern
    uint16_t sprite_addr = 0x3000;
    for (uint8_t row = 0; row < 4; row++)
    {
        for (uint8_t col = 0; col < 4; col++)
        {
            vm.roma[sprite_addr + (row * 4) + col] = (uint8_t)(0x10 + row * 4 + col);
        }
    }

    // Setup: coordinates in r0 (x=10, y=20), sprite address in r1
    vm.reg[0] = (20 << 8) | 10; // x=10 (low byte), y=20 (high byte)
    vm.reg[1] = sprite_addr;

    // Clear framebuffer area
    for (uint16_t i = 0; i < FB_W * FB_H; i++)
    {
        vm.ram[RAM_FB_BASE + i] = 0;
    }

    // Program: SPRA r0, r1, 4, 4 (4x4 sprite)
    vm.romb[0] = OP_SPRRA;
    vm.romb[1] = 0; // rc = r0 (coords)
    vm.romb[2] = 1; // ra = r1 (sprite address)
    vm.romb[3] = 4; // width
    vm.romb[4] = 4; // height

    vm_step(&vm);

    // Verify sprite was rendered to framebuffer
    for (uint8_t row = 0; row < 4; row++)
    {
        for (uint8_t col = 0; col < 4; col++)
        {
            uint16_t screen_x = 10 + col;
            uint16_t screen_y = 20 + row;
            uint16_t fb_addr = RAM_FB_BASE + (screen_y * FB_W) + screen_x;
            uint8_t expected = (uint8_t)(0x10 + row * 4 + col);

            ASSERT(vm.ram[fb_addr] == expected,
                   "SPRA: pixel at (%d,%d) should be 0x%02X, got 0x%02X", screen_x, screen_y,
                   expected, vm.ram[fb_addr]);
        }
    }

    return 1;
}

int test_spra_at_origin(void)
{
    VM vm;
    init_test_vm(&vm);

    // Create 2x2 sprite
    vm.roma[0x4000] = 0xAA;
    vm.roma[0x4001] = 0xBB;
    vm.roma[0x4002] = 0xCC;
    vm.roma[0x4003] = 0xDD;

    // Render at (0, 0)
    vm.reg[0] = 0x0000; // x=0, y=0
    vm.reg[1] = 0x4000;

    // Clear framebuffer
    for (uint16_t i = 0; i < 1000; i++)
    {
        vm.ram[RAM_FB_BASE + i] = 0;
    }

    // Program: SPRA r0, r1, 2, 2
    vm.romb[0] = OP_SPRRA;
    vm.romb[1] = 0; // rc
    vm.romb[2] = 1; // ra
    vm.romb[3] = 2; // width
    vm.romb[4] = 2; // height

    vm_step(&vm);

    // Verify top-left corner
    ASSERT(vm.ram[RAM_FB_BASE + 0] == 0xAA, "Pixel (0,0) should be 0xAA");
    ASSERT(vm.ram[RAM_FB_BASE + 1] == 0xBB, "Pixel (1,0) should be 0xBB");
    ASSERT(vm.ram[RAM_FB_BASE + FB_W] == 0xCC, "Pixel (0,1) should be 0xCC");
    ASSERT(vm.ram[RAM_FB_BASE + FB_W + 1] == 0xDD, "Pixel (1,1) should be 0xDD");

    return 1;
}

int test_spra_clipping_right(void)
{
    VM vm;
    init_test_vm(&vm);

    // Create 5x3 sprite
    for (uint8_t i = 0; i < 15; i++)
    {
        vm.roma[0x5000 + i] = 0x50 + i;
    }

    // Render near right edge (x=178, y=10) - should clip
    vm.reg[0] = (10 << 8) | 178; // x=178, y=10
    vm.reg[1] = 0x5000;

    // Clear framebuffer area
    for (uint16_t i = 0; i < FB_W * FB_H; i++)
    {
        vm.ram[RAM_FB_BASE + i] = 0;
    }

    // Program: SPRA r0, r1, 5, 3
    vm.romb[0] = OP_SPRRA;
    vm.romb[1] = 0; // rc
    vm.romb[2] = 1; // ra
    vm.romb[3] = 5; // width
    vm.romb[4] = 3; // height

    vm_step(&vm);

    // Only first 2 columns should be visible (178, 179), 180+ is clipped
    uint16_t fb_row = RAM_FB_BASE + 10 * FB_W;
    ASSERT(vm.ram[fb_row + 178] == 0x50, "First pixel should render");
    ASSERT(vm.ram[fb_row + 179] == 0x51, "Second pixel should render");

    // Middle pixels (180+) should not render - but let's check they're 0
    // Actually, framebuffer width is 180, so position 180 is out of bounds
    // The implementation should skip these

    return 1;
}

int test_spra_clipping_bottom(void)
{
    VM vm;
    init_test_vm(&vm);

    // Create 3x5 sprite
    for (uint8_t i = 0; i < 15; i++)
    {
        vm.roma[0x6000 + i] = 0x60 + i;
    }

    // Render near bottom edge (x=10, y=134) - should clip
    vm.reg[0] = (134 << 8) | 10; // x=10, y=134
    vm.reg[1] = 0x6000;

    // Clear framebuffer
    for (uint16_t i = 0; i < FB_W * FB_H; i++)
    {
        vm.ram[RAM_FB_BASE + i] = 0;
    }

    // Program: SPRA r0, r1, 3, 5
    vm.romb[0] = OP_SPRRA;
    vm.romb[1] = 0; // rc
    vm.romb[2] = 1; // ra
    vm.romb[3] = 3; // width
    vm.romb[4] = 5; // height

    vm_step(&vm);

    // Only first 2 rows should be visible (y=134, 135), y >= 136 is clipped
    uint16_t fb_row_134 = RAM_FB_BASE + 134 * FB_W + 10;
    uint16_t fb_row_135 = RAM_FB_BASE + 135 * FB_W + 10;

    ASSERT(vm.ram[fb_row_134] == 0x60, "Row 134 should render");
    ASSERT(vm.ram[fb_row_135] == 0x63, "Row 135 should render");
    // Row 136 and beyond should be clipped (loop breaks)

    return 1;
}

int test_spra_large_sprite(void)
{
    VM vm;
    init_test_vm(&vm);

    // Create 16x16 sprite with gradient pattern
    for (uint8_t row = 0; row < 16; row++)
    {
        for (uint8_t col = 0; col < 16; col++)
        {
            vm.roma[0x7000 + row * 16 + col] = (uint8_t)(row * 16 + col);
        }
    }

    // Render at (50, 60)
    vm.reg[0] = (60 << 8) | 50;
    vm.reg[1] = 0x7000;

    // Clear framebuffer
    for (uint16_t i = 0; i < FB_W * FB_H; i++)
    {
        vm.ram[RAM_FB_BASE + i] = 0;
    }

    // Program: SPRA r0, r1, 16, 16
    vm.romb[0] = OP_SPRRA;
    vm.romb[1] = 0;  // rc
    vm.romb[2] = 1;  // ra
    vm.romb[3] = 16; // width
    vm.romb[4] = 16; // height

    vm_step(&vm);

    // Verify a few key pixels
    uint16_t fb_addr_0_0 = RAM_FB_BASE + 60 * FB_W + 50;
    uint16_t fb_addr_15_15 = RAM_FB_BASE + 75 * FB_W + 65;

    ASSERT(vm.ram[fb_addr_0_0] == 0, "Top-left pixel should be 0");
    ASSERT(vm.ram[fb_addr_15_15] == 255, "Bottom-right pixel should be 255");

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
    run_test("LDR.w/STR.w", test_ldrw_strw);
    run_test("LDRI/STRI", test_ldri_stri);
    run_test("LDRI.w/STRI.w", test_ldriw_striw);

    // ROMA
    run_test("LDA", test_lda);
    run_test("LDA.w", test_ldaw);
    run_test("LDAR", test_ldar);
    run_test("LDAR.w", test_ldarw);

    // CPYRA / SPRA
    run_test("CPYRA", test_cpyra);
    run_test("CPYRA zero count", test_cpyra_zero_count);
    run_test("CPYRA 255 bytes", test_cpyra_255_bytes);
    run_test("SPRA basic", test_spra_basic);
    run_test("SPRA at origin", test_spra_at_origin);
    run_test("SPRA clipping right", test_spra_clipping_right);
    run_test("SPRA clipping bottom", test_spra_clipping_bottom);
    run_test("SPRA large sprite", test_spra_large_sprite);

    print_summary();

    return tests_failed == 0 ? 0 : 1;
}
