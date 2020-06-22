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
        o_int32_tp memory;

        printf("Hello, ginec!\n");
        memory = fs_load_int32_buffer("game.gc");
        if (memory.err != NULL) {
            crash(memory.err);
        }
        printf("Goodbuy, ginec!\n");

        return 0;
    }
#endif
