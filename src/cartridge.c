#include "cartridge.h"

cartridge_t cartridge_new(uint32_t* memory) {
    cartridge_t cartridge = { NULL };
    cartridge.memory = memory;
    return cartridge;
}

void cartridge_free(cartridge_t cartridge) {
    free(cartridge.memory);
}
