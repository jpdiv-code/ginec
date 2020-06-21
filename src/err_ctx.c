#include "err_ctx.h"

err_ctx_t err_ctx_new_malloc(
    size_t  size
) {
    err_ctx_t ctx;

    ctx.malloc.size = size;

    return ctx;
}

err_ctx_t err_ctx_new_file(
    ENUM_ERR_FILE_T type,
    const char*     fname,
    const char*     modes
) {
    err_ctx_t ctx;

    ctx.file.type = type;
    ctx.file.fname = fname;
    ctx.file.modes = modes;
    
    return ctx;
}
