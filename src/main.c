#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "defs.h"
#include "errors.h"
#include "cartridge.h"

typedef OPTIONAL(uint32_t*) o_uint32_tp;

o_uint32_tp load(const char* filename) {
    FILE* file;
    long filelen;
    uint32_t* buffer;
    file = fopen(filename, "rb");
    if (file == NULL) {
        return (o_uint32_tp) { 0, error_new_unknown() };
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        return (o_uint32_tp) { 0, error_new_unknown() };
    }
    filelen = ftell(file);
    if (filelen == -1L) {
        return (o_uint32_tp) { 0, error_new_unknown() };
    }
    rewind(file);
    buffer = (uint32_t*)malloc(filelen * sizeof(uint32_t));
    if (buffer == NULL) {
        return (o_uint32_tp) { 0, error_new_unknown() };
    }
    if (fread(buffer, filelen, 1, file) != 1) {
        return (o_uint32_tp) { 0, error_new_unknown() };
    }
    if (fclose(file) != 0) {
        return (o_uint32_tp) { 0, error_new_unknown() };
    }
    return (o_uint32_tp) { buffer, NULL };
}

/*
uint32_t* load(const char* filename) {
    CATCH(file == NULL,
        "Couldn't open \"%s\" file\n", filename);
    CATCH(fseek(file, 0, SEEK_END) != 0,
        "Error of opening \"%s\" file: couldn't go to the end of the file\n", filename);
    filelen = ftell(file);
    CATCH(filelen == -1L,
        "Error working with \"%s\" file: couldn't get the length of the file\n", filename);
    rewind(file);
    buffer = (uint8_t*)malloc(filelen * sizeof(uint8_t));
    CATCH(buffer == NULL,
        "Error working with \"%s\" file: failed to allocate %ld bytes of ram\n", filename, filelen);
    CATCH(fread(buffer, filelen, 1, file) != 1,
        "Error working with \"%s\" file: failed to buffer the data from the file\n", filename);
    CATCH(fclose(file) != 0,
        "Error working with \"%s\" file: failed to close it\n", filename);
    return buffer;
}
*/

int main() {
    cartridge_t c;
    o_uint32_tp memory;
    printf("Hello, ginec!\n");
    memory = load("game.gc");
    memory.error->base = error_new_unknown();
    if (memory.error != NULL) {
        crash(memory.error);
    }
    c = cartridge_new(memory.value);
    cartridge_free(c);
    printf("Goodbuy, ginec!\n");
    return 0;
}
