#ifndef VM_H
#define VM_H

#include <stdint.h>

// ==============================
// VM CONSTANTS
// ==============================

#define REG_COUNT 4 // Number of general purpose registers

#define ROMA_SIZE 65536 // Size of assets ROM
#define ROMB_SIZE 65536 // Size of bytecode/binary ROM
#define RAM_SIZE 65536  // Size of RAM

#define MIMO_BASE 0x0000 // Base address for MIMO memory-mapped I/O
#define MIMO_SIZE 0x0100 // Size of MIMO region

#define MIMO_INPUT_DOWN 0x0000     // MIMO address for input down state
#define MIMO_INPUT_PRESSED 0x0002  // MIMO address for input pressed state
#define MIMO_INPUT_RELEASED 0x0004 // MIMO address for input released state

#define RAM_FB_BASE 0x0100    // Base address for framebuffer in RAM
#define FB_W 180              // Framebuffer width
#define FB_H 136              // Framebuffer height
#define FB_SIZE (FB_W * FB_H) // Framebuffer size

#define RENDER_SCALE 4 // Scale factor for rendering

#define VM_FRAME_DT (1.0f / 24.0f) // Fixed timestep for VM frame updates (24 FPS)

#define CALL_STACK_BASE 0xFE00 // Base address for call stack in RAM
#define CALL_STACK_SIZE 0x0200 // Size of call stack (512 bytes)

// ==============================
// INPUT BIT LAYOUT (12 buttons)
// ==============================

typedef enum
{
    BTN_UP = 0,
    BTN_DOWN = 1,
    BTN_LEFT = 2,
    BTN_RIGHT = 3,
    BTN_A = 4,
    BTN_B = 5,
    BTN_X = 6,
    BTN_Y = 7,
    BTN_L = 8,
    BTN_R = 9,
    BTN_START = 10,
    BTN_SELECT = 11,
} VmButtonBit;

// ==============================
// VM STRUCT
// ==============================

typedef struct VM
{
    uint16_t ip;             // Instruction pointer, targets romb
    uint16_t sp;             // Stack pointer (data stack), targets ram
    uint16_t csp;            // Call stack pointer, targets ram (0xFE00-0xFFFF)
    uint8_t flags;           // CPU flags: 0000VCNZ (Z=zero, N=negative, C=carry, V=overflow)
    uint16_t reg[REG_COUNT]; // General purpose registers
    uint8_t roma[ROMA_SIZE]; // Assets ROM (contains resources like images, sounds, etc)
    uint8_t romb[ROMB_SIZE]; // Bytecode/binary ROM (contains program code)
    uint8_t ram[RAM_SIZE];   // RAM (data memory)
} VM;

typedef enum
{
    STEP_OK = 0,
    STEP_SYNC = 1,
    STEP_HALT = 2,
    STEP_ERROR = 3,
} StepResult;

// ==============================
// MEMORY ACCESS HELPERS
// ==============================

static inline uint8_t vm_ram_read8(VM* vm, uint16_t addr) { return vm->ram[addr]; }

static inline uint16_t vm_ram_read16(VM* vm, uint16_t addr)
{
    uint8_t low = vm->ram[addr];
    uint8_t high = vm->ram[addr + 1];
    return (uint16_t)((uint16_t)low | ((uint16_t)high << 8));
}

static inline void vm_ram_write8(VM* vm, uint16_t addr, uint8_t val) { vm->ram[addr] = val; }

static inline void vm_ram_write16(VM* vm, uint16_t addr, uint16_t val)
{
    vm->ram[addr] = (uint8_t)(val & 0xFF);
    vm->ram[addr + 1] = (uint8_t)((val >> 8) & 0xFF);
}

static inline uint8_t vm_roma_read8(VM* vm, uint16_t addr) { return vm->roma[addr]; }

static inline uint16_t vm_roma_read16(VM* vm, uint16_t addr)
{
    uint8_t low = vm->roma[addr];
    uint8_t high = vm->roma[addr + 1];
    return (uint16_t)((uint16_t)low | ((uint16_t)high << 8));
}

// ==============================
// VM FUNCTIONS
// ==============================

StepResult vm_step(VM* vm);

#endif // VM_H

