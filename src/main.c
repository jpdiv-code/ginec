#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "vm.h"

#ifdef DEBUG_MODE
#include "host_debug.h"
#else
#include "host_normal.h"
#endif

// ==============================
// MAIN - Entry Point
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
        SDL_Quit();
        return 1;
    }

    int result;
#ifdef DEBUG_MODE
    printf("Running in DEBUG mode\n");
    result = host_debug_run(&vm);
#else
    result = host_normal_run(&vm);
#endif

    SDL_Quit();
    return result;
}

