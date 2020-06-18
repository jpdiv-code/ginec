#include "err_ctx.h"

err_ctx_t err_ctx_new_memory_allocation(size_t size) {
    err_ctx_t ctx;
    ctx.memory_allocation.size = size;
    return ctx;
}

err_ctx_t err_ctx_new_open_file(const char* fname, const char* modes) {
    err_ctx_t ctx;
    ctx.open_file.fname = fname;
    ctx.open_file.modes = modes;
    return ctx;
}

err_ctx_t err_ctx_new_working_with_file(const char* fname, const char* modes) {
    err_ctx_t ctx;
    ctx.open_file.fname = fname;
    ctx.open_file.modes = modes;
    return ctx;
}

err_ctx_t err_ctx_new_close_file(const char* fname, const char* modes) {
    err_ctx_t ctx;
    ctx.open_file.fname = fname;
    ctx.open_file.modes = modes;
    return ctx;
}
