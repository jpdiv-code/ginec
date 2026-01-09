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
// INPUT UTILITIES
// ==============================

static SDL_GameController* gamepad = NULL;

static uint16_t read_gamepad_state(void)
{
    if (!gamepad)
        return 0;

    uint16_t input = 0;

    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_UP))
        input |= (1 << BTN_UP);
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
        input |= (1 << BTN_DOWN);
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
        input |= (1 << BTN_LEFT);
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
        input |= (1 << BTN_RIGHT);
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_A))
        input |= (1 << BTN_A);
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_B))
        input |= (1 << BTN_B);
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_X))
        input |= (1 << BTN_X);
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_Y))
        input |= (1 << BTN_Y);
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
        input |= (1 << BTN_L);
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))
        input |= (1 << BTN_R);
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_START))
        input |= (1 << BTN_START);
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_BACK))
        input |= (1 << BTN_SELECT);

    return input;
}

static uint16_t read_input_state(void)
{
    const uint8_t* keys = SDL_GetKeyboardState(NULL);
    uint16_t input = 0;

    if (keys[SDL_SCANCODE_UP])
        input |= (1 << BTN_UP);
    if (keys[SDL_SCANCODE_DOWN])
        input |= (1 << BTN_DOWN);
    if (keys[SDL_SCANCODE_LEFT])
        input |= (1 << BTN_LEFT);
    if (keys[SDL_SCANCODE_RIGHT])
        input |= (1 << BTN_RIGHT);
    if (keys[SDL_SCANCODE_Z])
        input |= (1 << BTN_A);
    if (keys[SDL_SCANCODE_X])
        input |= (1 << BTN_B);
    if (keys[SDL_SCANCODE_A])
        input |= (1 << BTN_X);
    if (keys[SDL_SCANCODE_S])
        input |= (1 << BTN_Y);
    if (keys[SDL_SCANCODE_Q])
        input |= (1 << BTN_L);
    if (keys[SDL_SCANCODE_W])
        input |= (1 << BTN_R);
    if (keys[SDL_SCANCODE_RETURN])
        input |= (1 << BTN_START);
    if (keys[SDL_SCANCODE_RSHIFT] || keys[SDL_SCANCODE_LSHIFT])
        input |= (1 << BTN_SELECT);

    // Combine keyboard and gamepad input
    input |= read_gamepad_state();

    return input;
}

static void update_input_mmio(VM* vm)
{
    uint16_t current_input = read_input_state();

    uint16_t previous_input = vm_ram_read16(vm, MIMO_INPUT_DOWN);

    uint16_t pressed = current_input & ~previous_input;
    uint16_t released = previous_input & ~current_input;

    uint16_t existing_pressed = vm_ram_read16(vm, MIMO_INPUT_PRESSED);
    uint16_t existing_released = vm_ram_read16(vm, MIMO_INPUT_RELEASED);

    existing_pressed |= pressed;
    existing_released |= released;

    vm_ram_write16(vm, MIMO_INPUT_DOWN, current_input);

    vm_ram_write16(vm, MIMO_INPUT_PRESSED, existing_pressed);

    vm_ram_write16(vm, MIMO_INPUT_RELEASED, existing_released);
}

static void clear_input_latches(VM* vm)
{
    vm_ram_write16(vm, MIMO_INPUT_PRESSED, 0);

    vm_ram_write16(vm, MIMO_INPUT_RELEASED, 0);
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

            vm_ram_write8(vm, (uint16_t)fb_index, pixel); // pixel = palette index
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

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER |
                 SDL_INIT_GAMECONTROLLER) != 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        if (SDL_IsGameController(i))
        {
            gamepad = SDL_GameControllerOpen(i);
            if (gamepad)
            {
                printf("Gamepad connected: %s\n", SDL_GameControllerName(gamepad));
                break;
            }
            else
            {
                fprintf(stderr,
                        "Warning: Failed to open game controller at index %d: %s\n",
                        i, SDL_GetError());
            }
        }
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
    vm.sp = 0xFDFF;
    vm.csp = 0xFFFF;

    FILE* roma_file = fopen("game.roma", "rb");
    if (roma_file)
    {
        size_t roma_size = fread(vm.roma, 1, sizeof(vm.roma), roma_file);
        fclose(roma_file);
        printf("Loaded game.roma: %zu bytes\n", roma_size);
    }
    else
    {
        fprintf(stderr, "Warning: Could not open game.roma\n");
    }

    FILE* romb_file = fopen("game.romb", "rb");
    if (romb_file)
    {
        size_t romb_size = fread(vm.romb, 1, sizeof(vm.romb), romb_file);
        fclose(romb_file);
        printf("Loaded game.romb: %zu bytes\n", romb_size);
    }
    else
    {
        fprintf(stderr, "Error: Could not open game.romb\n");
        SDL_free(scratch_rgba);
        SDL_DestroyTexture(tex);
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    double next_frame_time = now_seconds() + VM_FRAME_DT;

    while (running)
    {
        clear_input_latches(&vm);

        update_input_mmio(&vm);

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
            if (e.type == SDL_CONTROLLERDEVICEADDED)
            {
                if (!gamepad)
                {
                    gamepad = SDL_GameControllerOpen(e.cdevice.which);
                    if (gamepad)
                    {
                        printf("Gamepad connected: %s\n", SDL_GameControllerName(gamepad));
                    }
                    else
                    {
                        fprintf(stderr, "Failed to open game controller (index %d): %s\n",
                                e.cdevice.which, SDL_GetError());
                    }
                }
            }
            if (e.type == SDL_CONTROLLERDEVICEREMOVED)
            {
                if (gamepad && e.cdevice.which ==
                                   SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad)))
                {
                    printf("Gamepad disconnected\n");
                    SDL_GameControllerClose(gamepad);
                    gamepad = NULL;
                }
            }
        }
        if (!running)
        {
            break;
        }

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
            uint8_t color_id = vm_ram_read8(&vm, (uint16_t)(RAM_FB_BASE + i));

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

    if (gamepad)
    {
        SDL_GameControllerClose(gamepad);
    }

    SDL_free(scratch_rgba);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

