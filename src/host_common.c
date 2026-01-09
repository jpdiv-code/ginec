#include "host_common.h"
#include "vm.h"

#include <SDL2/SDL.h>
#include <stdint.h>

// ==============================
// TIME UTILITIES
// ==============================

double host_now_seconds(void)
{
    static uint64_t freq = 0;
    if (freq == 0)
    {
        freq = (uint64_t)SDL_GetPerformanceFrequency();
    }
    return (double)SDL_GetPerformanceCounter() / (double)freq;
}

void host_sleep_until(double target_time)
{
    for (;;)
    {
        double t = host_now_seconds();
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

uint16_t host_read_input_state(void)
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

    return input;
}

void host_update_input_mmio(VM* vm)
{
    uint16_t current_input = host_read_input_state();
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

void host_clear_input_latches(VM* vm)
{
    vm_ram_write16(vm, MIMO_INPUT_PRESSED, 0);
    vm_ram_write16(vm, MIMO_INPUT_RELEASED, 0);
}
