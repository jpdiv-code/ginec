#pragma once

#include <stdint.h>

typedef struct cartridge_t {
    uint32_t    size;
    int32_t*    mem;
} cartridge_t;

cartridge_t cartridge_new(
    uint32_t    size,
    int32_t*    mem
);

void cartridge_free(
    cartridge_t cartridge
);
