#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct cartridge_t {
    uint8_t* memory;
} cartridge_t;

cartridge_t cartridge_new(uint8_t* memory);

void cartridge_free(cartridge_t cartridge);
