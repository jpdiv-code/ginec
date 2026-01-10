#include "host_normal.h"
#include "host_common.h"
#include "vga256.h"
#include "vm.h"

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// ==============================
// NORMAL MODE HOST
// ==============================

int host_normal_run(VM* vm)
{
    SDL_Window* win = SDL_CreateWindow(
        "Fantasy VM (SDL2)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, FB_W * RENDER_SCALE,
        FB_H * RENDER_SCALE, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!win)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* ren =
        SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren)
    {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        return 1;
    }

    SDL_Texture* tex =
        SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FB_W, FB_H);
    if (!tex)
    {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        return 1;
    }

    uint32_t* scratch_rgba = (uint32_t*)SDL_malloc((size_t)FB_SIZE * sizeof(uint32_t));
    if (!scratch_rgba)
    {
        fprintf(stderr, "Out of memory for scratch_rgba\n");
        SDL_DestroyTexture(tex);
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        return 1;
    }

    bool running = true;
    double next_frame_time = host_now_seconds() + VM_FRAME_DT;

    while (running)
    {
        host_clear_input_latches(vm);
        host_update_input_mmio(vm);

        while (true)
        {
            StepResult res = vm_step(vm);
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

        for (int i = 0; i < FB_SIZE; i++)
        {
            uint8_t color_id = vm_ram_read8(vm, (uint16_t)(RAM_FB_BASE + i));
            Color col = palette[color_id];
            scratch_rgba[i] = (uint32_t)((0xFF << 24) | (col.r << 16) | (col.g << 8) | (col.b));
        }

        SDL_UpdateTexture(tex, NULL, scratch_rgba, FB_W * sizeof(uint32_t));
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);

        host_sleep_until(next_frame_time);
        next_frame_time += VM_FRAME_DT;

        double t = host_now_seconds();
        if (t > next_frame_time + VM_FRAME_DT)
        {
            next_frame_time = t + VM_FRAME_DT;
        }
    }

    SDL_free(scratch_rgba);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    return 0;
}
