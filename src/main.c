#include <stdio.h>

#include "defs.h"
#include "err.h"
#include "fs.h"

#if TEST
    #include "../test/test.h"
#endif

#if TEST
    int main() {
        test();
    }
#else
    int main() {
        o_cartridge_t cartridge;

        cartridge = fs_load_cartridge("game.gc");
        if (cartridge.err != NULL) {
            crash(cartridge.err);
        }

        return 0;
    }
#endif
