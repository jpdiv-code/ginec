#include "err_ctx.h"

#include <stdint.h>

#include "cartridge.h"

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

err_ctx_t err_ctx_new_runtime(
    cartridge_t cartridge,
    uint32_t    ip
) {
    err_ctx_t ctx;

    ctx.runtime.cartridge = cartridge;
    ctx.runtime.ip = ip;
    
    return ctx;
}

err_ctx_t err_ctx_new_unknown_opc(
    cartridge_t cartridge,
    int32_t     opc,
    uint32_t    ip
) {
    err_ctx_t ctx;

    ctx.unknown_opc.cartridge = cartridge;
    ctx.unknown_opc.opc = opc;
    ctx.unknown_opc.ip = ip;
    
    return ctx;
}

err_ctx_t err_ctx_new_segfault(
    cartridge_t cartridge,
    int32_t     opc,
    uint32_t    ip
) {
    err_ctx_t ctx;

    ctx.segfault.cartridge = cartridge;
    ctx.segfault.opc = opc;
    ctx.segfault.ip = ip;
    
    return ctx;
}
