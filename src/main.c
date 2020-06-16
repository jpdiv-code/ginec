#include <stdio.h>
#include <stdlib.h>

#include "cartridge.h"

int main() {
    cartridge_t c;
    printf("Hello, ginec!\n");
    c = cartridge_new(NULL);
    cartridge_free(c);
    printf("Goodbuy, ginec!\n");
    return 0;
}
