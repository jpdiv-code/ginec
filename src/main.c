#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "defs.h"
#include "err.h"

typedef OPTIONAL(uint32_t*) o_uint32_tp;

o_uint32_tp load(const char* fname) {
    FILE* file;
    long filelen;
    uint32_t* buffer;
    const char* modes = "rb";
    file = fopen(fname, modes);
    if (file == NULL) {
        return (o_uint32_tp) { 0, err_new_open_file(fname, modes, NULL) };
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        return (o_uint32_tp) { 0,
            err_new_working_with_file(fname, modes, L"couldn't go to the end of the file", NULL) };
    }
    filelen = ftell(file);
    if (filelen == -1L) {
        return (o_uint32_tp) { 0,
            err_new_working_with_file(fname, modes, L"couldn't get the length of the file", NULL) };
    }
    rewind(file);
    size_t buffer_size = filelen * sizeof(uint32_t);
    buffer = (uint32_t*)malloc(buffer_size);
    if (buffer == NULL) {
        return (o_uint32_tp) { 0,
            err_new_working_with_file(fname, modes, L"failed to allocate buffer",
                err_new_memory_allocation(buffer_size, NULL)) };
    }
    if (fread(buffer, filelen, 1, file) != 1) {
        return (o_uint32_tp) { 0,
            err_new_working_with_file(fname, modes, L"failed to read the file into the buffer", NULL) };
    }
    if (fclose(file) != 0) {
        return (o_uint32_tp) { 0, err_new_close_file(fname, modes, NULL) };
    }
    return (o_uint32_tp) { buffer, NULL };
}

int main() {
    o_uint32_tp memory;
    printf("Hello, ginec!\n");
    memory = load("game.gc");
    if (memory.err != NULL) {
        crash(memory.err);
    }
    printf("Goodbuy, ginec!\n");
    return 0;
}
