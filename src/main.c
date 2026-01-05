#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <string.h>
#include <time.h>

#include "opcodes.h" // Include opcode definitions
#include "roma.h"    // Include test ROMA data
#include "vga256.h"  // Include VGA256 palette
#include "vm.h"      // Include VM definitions

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
// DRAW UTILITIES
// ==============================

void draw_sprite(VM* vm, int sprite_start_x, int sprite_start_y, int sprite_id, int sprite_w,
                 int sprite_h)
{
    int sprite_count = (int)(sizeof(sprites) / sizeof(sprites[0]));
    if (sprite_id >= sprite_count)
        sprite_id = 0; // use empty sprite for invalid id

    for (int sprite_y = 0; sprite_y < sprite_h; sprite_y++)
    {
        for (int sprite_x = 0; sprite_x < sprite_w; sprite_x++)
        {
            uint8_t pixel = sprites[sprite_id][sprite_y][sprite_x];
            if (pixel == 0)
                continue; // transparent

            int pixel_to_paint_x = sprite_start_x + sprite_x;
            int pixel_to_paint_y = sprite_start_y + sprite_y;

            if (pixel_to_paint_x < 0 || pixel_to_paint_y < 0 || pixel_to_paint_x >= FB_W ||
                pixel_to_paint_y >= FB_H)
                continue;

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
        "Fantasy VM (SDL2)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, FB_W * RENDER_SCALE,
        FB_H * RENDER_SCALE, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!win)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* ren =
        SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren)
    {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* tex =
        SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FB_W, FB_H);
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

    // Example program that fills random pixel with random color

    // Seed random generator
    vm.romb[0x0000] = OP_LDIw;
    vm.romb[0x0001] = 0;    // reg 0
    vm.romb[0x0002] = 0xFF; // low byte
    vm.romb[0x0003] = 0x00; // high byte (seed = 0x00FF)
    vm.romb[0x0004] = OP_SEED;
    vm.romb[0x0005] = 0; // seed from reg 0

    // Loop: generate random X coordinate (0-179)
    // reg 0 = X, reg 1 = Y, reg 2 = address, reg 3 = color

    // Generate random X coordinate (0-179)
    vm.romb[0x0006] = OP_RAND;
    vm.romb[0x0007] = 0; // reg 0 = random 8-bit value

    // Generate random Y coordinate (0-135)
    vm.romb[0x000C] = OP_RAND;
    vm.romb[0x000D] = 1; // reg 1 = random 8-bit value

    // Calculate address: base + Y * 180 + X
    // reg 2 = Y * 180
    vm.romb[0x0012] = OP_MOVw;
    vm.romb[0x0013] = 2; // reg 2 = dest
    vm.romb[0x0014] = 1; // reg 1 = src (Y)

    vm.romb[0x0015] = OP_MULw;
    vm.romb[0x0016] = 2;    // reg 2
    vm.romb[0x0017] = 0xB4; // low byte of 180
    vm.romb[0x0018] = 0x00; // high byte of 180

    // reg 2 = reg 2 + X
    vm.romb[0x0019] = OP_ADDRw;
    vm.romb[0x001A] = 2; // reg 2 (dest)
    vm.romb[0x001B] = 0; // reg 0 (X)

    // reg 2 = reg 2 + FB_BASE (0x0100)
    vm.romb[0x001C] = OP_ADDw;
    vm.romb[0x001D] = 2;    // reg 2
    vm.romb[0x001E] = 0x00; // low byte of 0x0100
    vm.romb[0x001F] = 0x01; // high byte of 0x0100

    // Generate random color
    vm.romb[0x0020] = OP_RAND;
    vm.romb[0x0021] = 3; // reg 3 = random 8-bit color

    // Store color at address (but only if within bounds)
    // Check X < 180: compare reg 0 with 180
    vm.romb[0x0022] = OP_CMPw;
    vm.romb[0x0023] = 0;    // reg 0 (X)
    vm.romb[0x0024] = 0xB4; // low byte of 180
    vm.romb[0x0025] = 0x00; // high byte of 180

    // Jump to loop if X >= 180 (carry not set means X >= 180)
    vm.romb[0x0026] = OP_JNC;
    vm.romb[0x0027] = 0x06; // low byte of loop address
    vm.romb[0x0028] = 0x00; // high byte of loop address

    // Check Y < 136: compare reg 1 with 136
    vm.romb[0x0029] = OP_CMPw;
    vm.romb[0x002A] = 1;    // reg 1 (Y)
    vm.romb[0x002B] = 0x88; // low byte of 136
    vm.romb[0x002C] = 0x00; // high byte of 136

    // Jump to loop if Y >= 136
    vm.romb[0x002D] = OP_JNC;
    vm.romb[0x002E] = 0x06; // low byte of loop address
    vm.romb[0x002F] = 0x00; // high byte of loop address

    // Store color at address
    vm.romb[0x0030] = OP_STR;
    vm.romb[0x0031] = 2; // address in reg 2
    vm.romb[0x0032] = 3; // color in reg 3

    // Sync and loop
    vm.romb[0x0033] = OP_SYNC;
    vm.romb[0x0034] = OP_JMP;
    vm.romb[0x0035] = 0x06; // low byte of loop address
    vm.romb[0x0036] = 0x00; // high byte of loop address

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
            if (res == STEP_SYNC)
            {
                break;
            }
            if (res == STEP_ERROR)
            {
                fprintf(stderr, "VM encountered an error during execution\n");
                running = false;
                break;
            }
        }
        if (!running)
        {
            break;
        }

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = false;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
            {
                running = false;
            }
        }
        if (!running)
        {
            break;
        }

        // TODO: Update input states in MIMO region of RAM

        // ==============================
        // drawing sprite RU_I demo code
        // ==============================

        // memset(&vm.ram[RAM_FB_BASE], 0, FB_SIZE); // Clear framebuffer for demonstration

        // uint16_t offset = 1;

        // draw_sprite(&vm, offset, 1, 1, 8, 8); // Draw sprite RU_I
        // offset += 8;
        // draw_sprite(&vm, offset, 1, 2, 8, 8); // Draw sprite RU_L
        // offset += 8;
        // draw_sprite(&vm, offset, 1, 3, 8, 8); // Draw sprite RU_U
        // offset += 8;
        // draw_sprite(&vm, offset, 1, 4, 8, 8); // Draw sprite RU_SH
        // offset += 8;
        // draw_sprite(&vm, offset, 1, 5, 8, 8); // Draw sprite RU_A
        // offset += 16;

        // draw_sprite(&vm, offset, 1, 4, 8, 8); // Draw sprite RU_SH
        // offset += 8;
        // draw_sprite(&vm, offset, 1, 2, 8, 8); // Draw sprite RU_L
        // offset += 8;
        // draw_sprite(&vm, offset, 1, 3, 8, 8); // Draw sprite RU_U
        // offset += 8;
        // draw_sprite(&vm, offset, 1, 4, 8, 8); // Draw sprite RU_SH
        // offset += 8;
        // draw_sprite(&vm, offset, 1, 5, 8, 8); // Draw sprite RU_A

        for (int i = 0; i < FB_SIZE; i++)
        {
            uint8_t color_id = vm.ram[RAM_FB_BASE + i];

            Color col = palette[color_id];
            scratch_rgba[i] =
                (uint32_t)((0xFF << 24) | (col.r << 16) | (col.g << 8) | (col.b)); // ARGB format
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

