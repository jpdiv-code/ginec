#include "cartridge.h"

#include <stdint.h>
#include <stdlib.h>

cartridge_t cartridge_new(
    uint32_t    size,
    int32_t*    mem
) {
    cartridge_t cartridge;
    cartridge.size = size;
    cartridge.mem = mem;
    return cartridge;
}

void cartridge_free(
    cartridge_t cartridge
) {
    free(cartridge.mem);
}
