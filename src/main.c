#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <string.h>

#include "roma.h" // Include the palette definition

// ==============================
// VM CONSTANTS
// ==============================

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

#define ROMA_PALETTE_BASE 0x0000 // Base address for palette data in ROMA (32 RGB565 colors, 2 bytes each)
#define ROMA_PALETTE_SIZE 0x0040 // Size of palette data in ROMA (64 bytes)

#define VM_FRAME_DT (1.0f / 24.0f) // Fixed timestep for VM frame updates (24 FPS)

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
    uint16_t sp;             // Stack pointer, targets ram
    uint8_t flags;           // CPU flags: 0000VCNZ (Z=zero, N=negative, C=carry, V=overflow)
    uint16_t reg[4];         // General purpose registers
    uint8_t roma[ROMA_SIZE]; // Assets ROM (contains resources like images, sounds, etc)
    uint8_t romb[ROMB_SIZE]; // Bytecode/binary ROM (contains program code)
    uint8_t ram[RAM_SIZE];   // RAM (data memory)
} VM;

typedef enum
{
    STEP_OK = 0,
    STEP_VSYNC = 1,
    STEP_HALT = 2,
    STEP_ERROR = 3,
} StepResult;

// ==============================
// OPCODES
// ==============================

enum
{
    OP_NOP = 0x00,
    OP_VSYNC = 0x01,
    OP_JMP = 0x02,
};

// ==============================
// TIME UTILITIES
// ==============================

static double now_seconds(void)
{
    static uint64_t freq = 0;
    if (freq == 0)
    {
        freq = (uint64_t)SDL_GetPerformanceFrequency();
    }
    return (double)SDL_GetPerformanceCounter() / (double)freq;
}

static void sleep_until(double target_time)
{
    for (;;)
    {
        double t = now_seconds();
        double dt = target_time - t;
        if (dt <= 0.0)
        {
            return;
        }

        if (dt > 0.002)
        {
            SDL_Delay((uint32_t)((dt - 0.001) * 1000.0));
        }
        else
        {
            // Busy wait
        }
    }
}

// ==============================
// VM
// ==============================

StepResult vm_step(VM* vm)
{
    uint8_t opcode = vm->romb[vm->ip++];
    switch (opcode)
    {
    case OP_NOP:
        break;
    case OP_VSYNC:
        return STEP_VSYNC;
    case OP_JMP:
    {
        uint8_t low = vm->romb[vm->ip];
        vm->ip++;
        uint8_t high = vm->romb[vm->ip];
        uint16_t addr = 0x0000;
        addr |= (uint16_t)low;
        addr |= (uint16_t)(high << 8);
        vm->ip = addr;
        break;
    }
    default:
        return STEP_ERROR;
    }
    return STEP_OK;
}

// ==============================
// Draw sprites
// ==============================

void draw_sprite(
        VM* vm,
        int sprite_start_x, int sprite_start_y,
        int sprite_id, int sprite_w, int sprite_h
)
{
    int sprite_count = (int)(sizeof(sprites) / sizeof(sprites[0]));
    if (sprite_id >= sprite_count) sprite_id = 0; // use empty sprite for invalid id

    for (int sprite_y = 0; sprite_y < sprite_h; sprite_y++)
    {
        for (int sprite_x = 0; sprite_x < sprite_w; sprite_x++)
        {
            uint8_t pixel = sprites[sprite_id][sprite_y][sprite_x];
            if (pixel == 0) continue; // transparent

            int pixel_to_paint_x = sprite_start_x + sprite_x;
            int pixel_to_paint_y = sprite_start_y + sprite_y;

            if (pixel_to_paint_x < 0 || pixel_to_paint_y < 0 || pixel_to_paint_x >= FB_W || pixel_to_paint_y >= FB_H) continue;

            int fb_index = RAM_FB_BASE + pixel_to_paint_y * FB_W + pixel_to_paint_x;

            vm->ram[fb_index] = pixel; // pixel = palette index
        }
    }
}

// ==============================
// MAIN
// ==============================

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow(
        "Fantasy VM (SDL2)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        FB_W * RENDER_SCALE, FB_H * RENDER_SCALE,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!win)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* ren = SDL_CreateRenderer(
            win, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!ren)
    {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* tex = SDL_CreateTexture(
            ren,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            FB_W, FB_H
    );
    if (!tex)
    {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    uint32_t* scratch_rgba = (uint32_t*)SDL_malloc((size_t)FB_SIZE * sizeof(uint32_t));
    if (!scratch_rgba)
    {
        fprintf(stderr, "Out of memory for scratch_rgba\n");
        SDL_DestroyTexture(tex);
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    VM vm;
    memset(&vm, 0, sizeof(VM));
    vm.ip = 0x0000;
    vm.sp = 0xFFFF;
    // TODO: Load ROMA and ROMB from some source
    // For now, just fill ROMB with a simple program that does nothing
    vm.romb[0x0000] = OP_VSYNC;
    vm.romb[0x0001] = OP_JMP;
    vm.romb[0x0002] = 0x00;
    vm.romb[0x0003] = 0x00;

    bool running = true;
    double next_frame_time = now_seconds() + VM_FRAME_DT;

    while (running)
    {
        while (true)
        {
            StepResult res = vm_step(&vm);
            if (res == STEP_HALT)
            {
                running = false;
                break;
            }
            if (res == STEP_VSYNC) { break; }
            if (res == STEP_ERROR)
            {
                fprintf(stderr, "VM encountered an error during execution\n");
                running = false;
                break;
            }
        }
        if (!running) { break; }

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) { running = false; }
            if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) { running = false; }
        }
        if (!running) { break; }

        // TODO: Update input states in MIMO region of RAM
        // TODO: Render framebuffer from RAM to scratch_rgba
        // TODO: Render scratch_rgba to texture and present

        // ==============================
        // buffer rendering demo code
        // ==============================

        // memset(&vm.ram[RAM_FB_BASE], 0, FB_SIZE); // Clear framebuffer for demonstration

        // for (int i = 0; i < FB_SIZE; i++)
        // {
        //     vm.ram[RAM_FB_BASE + i] = 1;
        // }

        // for (int i = 0; i < FB_SIZE; i++)
        // {
        //     uint8_t color_id = vm.ram[RAM_FB_BASE + i];

        //     color_id &= 31; // Ensure color_id is within palette range

        //     Color col = palette[color_id];
        //     scratch_rgba[i] = (0xFF << 24) | (col.r << 16) | (col.g << 8) | (col.b); // ARGB
        //     format
        // }

        // ==============================
        // drawing sprite RU_I demo code
        // ==============================

        memset(&vm.ram[RAM_FB_BASE], 0, FB_SIZE); // Clear framebuffer for demonstration

        uint16_t offset = 1;

        draw_sprite(&vm, offset, 1, 1, 8, 8); // Draw sprite RU_I
        offset += 8;
        draw_sprite(&vm, offset, 1, 2, 8, 8); // Draw sprite RU_L
        offset += 8;
        draw_sprite(&vm, offset, 1, 3, 8, 8); // Draw sprite RU_U
        offset += 8;
        draw_sprite(&vm, offset, 1, 4, 8, 8); // Draw sprite RU_SH
        offset += 8;
        draw_sprite(&vm, offset, 1, 5, 8, 8); // Draw sprite RU_A
        offset += 16;

        draw_sprite(&vm, offset, 1, 4, 8, 8); // Draw sprite RU_SH
        offset += 8;
        draw_sprite(&vm, offset, 1, 2, 8, 8); // Draw sprite RU_L
        offset += 8;
        draw_sprite(&vm, offset, 1, 3, 8, 8); // Draw sprite RU_U
        offset += 8;
        draw_sprite(&vm, offset, 1, 4, 8, 8); // Draw sprite RU_SH
        offset += 8;
        draw_sprite(&vm, offset, 1, 5, 8, 8); // Draw sprite RU_A

        for (int i = 0; i < FB_SIZE; i++)
        {
            uint8_t color_id = vm.ram[RAM_FB_BASE + i];

            color_id &= 31; // Ensure color_id is within palette range

            Color col = palette[color_id];
            scratch_rgba[i] = (uint32_t)((0xFF << 24) | (col.r << 16) | (col.g << 8) | (col.b)); // ARGB format
        }

        SDL_UpdateTexture(tex, NULL, scratch_rgba, FB_W * sizeof(uint32_t));
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);

        sleep_until(next_frame_time);
        next_frame_time += VM_FRAME_DT;

        double t = now_seconds();
        if (t > next_frame_time + VM_FRAME_DT)
        {
            next_frame_time = t + VM_FRAME_DT;
        }
    }

    SDL_free(scratch_rgba);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
