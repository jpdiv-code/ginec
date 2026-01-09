#include "host_debug.h"
#include "host_common.h"
#include "vga256.h"
#include "vm.h"

#include <SDL2/SDL.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wnull-pointer-subtraction"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#include "../lib/nuklear.h"
#include "../lib/nuklear_sdl_renderer.h"
#pragma GCC diagnostic pop

// ==============================
// VM THREAD (execution only, no SDL)
// ==============================

static void* vm_thread_main(void* arg)
{
    VMDebugContext* ctx = (VMDebugContext*)arg;

    double next_frame_time = host_now_seconds() + VM_FRAME_DT;

    while (ctx->vm_thread_running)
    {
        pthread_mutex_lock(&ctx->vm_mutex);

        if (ctx->state == VM_DEBUG_RUNNING || ctx->state == VM_DEBUG_STEP)
        {
            host_clear_input_latches(ctx->vm);
            host_update_input_mmio(ctx->vm);

            bool should_continue = true;
            while (should_continue)
            {
                StepResult res = vm_step(ctx->vm);
                if (res == STEP_HALT)
                {
                    ctx->state = VM_DEBUG_STOPPED;
                    ctx->vm_thread_running = false;
                    should_continue = false;
                }
                else if (res == STEP_SYNC)
                {
                    should_continue = false;
                }
                else if (res == STEP_ERROR)
                {
                    fprintf(stderr, "VM Thread: VM encountered an error\n");
                    ctx->state = VM_DEBUG_STOPPED;
                    ctx->vm_thread_running = false;
                    should_continue = false;
                }

                if (ctx->state == VM_DEBUG_STEP)
                {
                    ctx->state = VM_DEBUG_PAUSED;
                    should_continue = false;
                }
            }

            pthread_mutex_lock(&ctx->fb_mutex);
            for (int i = 0; i < FB_SIZE; i++)
            {
                ctx->framebuffer[i] = vm_ram_read8(ctx->vm, (uint16_t)(RAM_FB_BASE + i));
            }
            pthread_mutex_unlock(&ctx->fb_mutex);
        }

        pthread_mutex_unlock(&ctx->vm_mutex);

        host_sleep_until(next_frame_time);
        next_frame_time += VM_FRAME_DT;

        double t = host_now_seconds();
        if (t > next_frame_time + VM_FRAME_DT)
        {
            next_frame_time = t + VM_FRAME_DT;
        }
    }

    return NULL;
}

// ==============================
// DEBUGGER UI (main thread, manages both windows)
// ==============================

static void debugger_ui_run(VMDebugContext* ctx)
{
    SDL_Window* vm_win = SDL_CreateWindow("Fantasy VM - Main Window", 100, 100, FB_W * RENDER_SCALE,
                                          FB_H * RENDER_SCALE, SDL_WINDOW_SHOWN);
    if (!vm_win)
    {
        fprintf(stderr, "Debugger: SDL_CreateWindow (VM) failed: %s\n", SDL_GetError());
        return;
    }

    SDL_Renderer* vm_ren =
        SDL_CreateRenderer(vm_win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!vm_ren)
    {
        fprintf(stderr, "Debugger: SDL_CreateRenderer (VM) failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(vm_win);
        return;
    }

    SDL_Texture* vm_tex = SDL_CreateTexture(vm_ren, SDL_PIXELFORMAT_ARGB8888,
                                            SDL_TEXTUREACCESS_STREAMING, FB_W, FB_H);
    if (!vm_tex)
    {
        fprintf(stderr, "Debugger: SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(vm_ren);
        SDL_DestroyWindow(vm_win);
        return;
    }

    uint32_t* scratch_rgba = (uint32_t*)SDL_malloc((size_t)FB_SIZE * sizeof(uint32_t));
    if (!scratch_rgba)
    {
        fprintf(stderr, "Debugger: Out of memory for scratch_rgba\n");
        SDL_DestroyTexture(vm_tex);
        SDL_DestroyRenderer(vm_ren);
        SDL_DestroyWindow(vm_win);
        return;
    }

    SDL_Window* dbg_win = SDL_CreateWindow("GINEC Debugger", 800, 100, 800, 600,
                                           SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!dbg_win)
    {
        fprintf(stderr, "Debugger: SDL_CreateWindow (Debugger) failed: %s\n", SDL_GetError());
        SDL_free(scratch_rgba);
        SDL_DestroyTexture(vm_tex);
        SDL_DestroyRenderer(vm_ren);
        SDL_DestroyWindow(vm_win);
        return;
    }

    SDL_Renderer* dbg_ren =
        SDL_CreateRenderer(dbg_win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!dbg_ren)
    {
        fprintf(stderr, "Debugger: SDL_CreateRenderer (Debugger) failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(dbg_win);
        SDL_free(scratch_rgba);
        SDL_DestroyTexture(vm_tex);
        SDL_DestroyRenderer(vm_ren);
        SDL_DestroyWindow(vm_win);
        return;
    }

    struct nk_context* nk_ctx = nk_sdl_init(dbg_win, dbg_ren);
    struct nk_font_atlas* atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();

    struct nk_color background = nk_rgb(28, 48, 62);

    while (ctx->debugger_running)
    {
        SDL_Event evt;
        nk_input_begin(nk_ctx);
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_QUIT)
            {
                ctx->debugger_running = false;
                ctx->vm_thread_running = false;
            }
            if (evt.type == SDL_KEYDOWN && evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
            {
                ctx->debugger_running = false;
                ctx->vm_thread_running = false;
            }
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(nk_ctx);

        if (nk_begin(nk_ctx, "VM Debugger", nk_rect(50, 50, 700, 500),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(nk_ctx, 30, 1);
            nk_label(nk_ctx, "GINEC VM Debugger", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(nk_ctx, 20, 1);
            nk_label(nk_ctx, "(Debug UI will be implemented here)", NK_TEXT_LEFT);
        }
        nk_end(nk_ctx);

        SDL_SetRenderDrawColor(dbg_ren, background.r, background.g, background.b, 255);
        SDL_RenderClear(dbg_ren);
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_RenderPresent(dbg_ren);

        pthread_mutex_lock(&ctx->fb_mutex);
        for (int i = 0; i < FB_SIZE; i++)
        {
            uint8_t color_id = ctx->framebuffer[i];
            Color col = palette[color_id];
            scratch_rgba[i] = (uint32_t)((0xFF << 24) | (col.r << 16) | (col.g << 8) | (col.b));
        }
        pthread_mutex_unlock(&ctx->fb_mutex);

        SDL_UpdateTexture(vm_tex, NULL, scratch_rgba, FB_W * sizeof(uint32_t));
        SDL_RenderClear(vm_ren);
        SDL_RenderCopy(vm_ren, vm_tex, NULL, NULL);
        SDL_RenderPresent(vm_ren);
    }

    nk_sdl_shutdown();
    SDL_DestroyRenderer(dbg_ren);
    SDL_DestroyWindow(dbg_win);
    SDL_free(scratch_rgba);
    SDL_DestroyTexture(vm_tex);
    SDL_DestroyRenderer(vm_ren);
    SDL_DestroyWindow(vm_win);
}

// ==============================
// DEBUG HOST ENTRY POINT
// ==============================

int host_debug_run(VM* vm)
{
    VMDebugContext debug_ctx;
    debug_ctx.vm = vm;
    pthread_mutex_init(&debug_ctx.vm_mutex, NULL);
    pthread_mutex_init(&debug_ctx.fb_mutex, NULL);
    debug_ctx.state = VM_DEBUG_RUNNING;
    debug_ctx.vm_thread_running = true;
    debug_ctx.debugger_running = true;
    memset(debug_ctx.framebuffer, 0, FB_SIZE);

    if (pthread_create(&debug_ctx.vm_thread, NULL, vm_thread_main, &debug_ctx) != 0)
    {
        fprintf(stderr, "Failed to create VM thread\n");
        pthread_mutex_destroy(&debug_ctx.vm_mutex);
        pthread_mutex_destroy(&debug_ctx.fb_mutex);
        return 1;
    }

    debugger_ui_run(&debug_ctx);

    pthread_join(debug_ctx.vm_thread, NULL);

    pthread_mutex_destroy(&debug_ctx.vm_mutex);
    pthread_mutex_destroy(&debug_ctx.fb_mutex);

    return 0;
}
