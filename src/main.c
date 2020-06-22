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
        o_int32_tp mem;

        printf("Hello, ginec!\n");
        mem = fs_load_int32_buffer("game.gc");
        if (mem.err != NULL) {
            crash(mem.err);
        }
        printf("Goodbuy, ginec!\n");

        return 0;
    }
#endif
