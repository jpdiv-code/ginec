#include "err.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>
#include <string.h>
#include <stdarg.h>

#include "err_ctx.h"
#include "cli.h"
#include "cartridge.h"

const uint32_t ERR_MAX_MSG_LENGTH = 512;
const uint32_t ERR_MAX_FILENAME_LENGTH = 256;

err_t* err_new_base(
    ENUM_ERR_T  type,
    wchar_t*    msg,
    err_ctx_t   ctx,
    err_t*      base
) {
    err_t* err;

    err         = (err_t*)malloc(sizeof(err_t));
    err->type   = type;
    err->msg    = msg;
    err->ctx    = ctx;
    err->base   = base;

    return err;
}

wchar_t* err_msg_new(
    uint32_t        max_msg_len,
    const wchar_t*  msg_base,
    ...
) {
    wchar_t* msg;
    va_list args;

    msg = (wchar_t*)malloc(max_msg_len * sizeof(wchar_t));
    va_start(args, msg_base);
    vswprintf(msg, max_msg_len, msg_base, args);
    va_end(args);
    msg = (wchar_t*)realloc(msg, (wcslen(msg) + 1) * sizeof(wchar_t));

    return msg;
}

err_t* err_new_general(
    const wchar_t*  info,
    err_t*          base
) {
    err_ctx_t ctx;
    err_t* err;

    err = err_new_base(
        ERR_T_GENERAL,
        err_msg_new(ERR_MAX_MSG_LENGTH, L"General error: %S", info),
        ctx, base
    );

    return err;
}

err_t* err_new_malloc(
    const wchar_t*  info,
    size_t          size,
    err_t*          base
) {
    wchar_t* msg;
    err_t* err;

    if (info == NULL) {
        msg = err_msg_new(ERR_MAX_MSG_LENGTH,
            L"Error allocating %d bytes of memory", size);
    } else {
        msg = err_msg_new(ERR_MAX_MSG_LENGTH,
            L"Error allocating %d bytes of memory, info: %S", size, info);
    }
    err = err_new_base(
        ERR_T_MALLOC, msg,
        err_ctx_new_malloc(size), base
    );

    return err;
}

err_t* err_new_file(
    const wchar_t*  info,
    ENUM_ERR_FILE_T type,
    const char*     fname,
    const char*     modes,
    err_t*          base
) {
    wchar_t* msg;
    err_t* err;

    if (info == NULL) {
        msg = err_msg_new(ERR_MAX_MSG_LENGTH,
            L"Error of working with the file \"%s\" with modes \"%s\"", fname, modes
        );
    } else {
        msg = err_msg_new(ERR_MAX_MSG_LENGTH,
            L"Error of working with the file \"%s\" with modes \"%s\": %S", fname, modes, info
        );
    }
    err = err_new_base(
        ERR_T_FILE, msg,
        err_ctx_new_file(type, fname, modes),
        base
    );

    return err;
}

err_t* err_new_runtime(
    const wchar_t*  info,
    cartridge_t     cartridge,
    uint32_t        ip,
    err_t*          base
) {
    wchar_t* msg;
    err_t* err;

    if (info == NULL) {
        msg = err_msg_new(ERR_MAX_MSG_LENGTH,
            L"Runtime error at %zu", ip
        );
    } else {
        msg = err_msg_new(ERR_MAX_MSG_LENGTH,
            L"Runtime error at %zu: %S", ip, info
        );
    }
    err = err_new_base(
        ERR_T_RUNTIME, msg,
        err_ctx_new_runtime(cartridge, ip),
        base
    );

    return err;
}

err_t* err_new_unknown_opcode(
    const wchar_t*  info,
    cartridge_t     cartridge,
    int32_t         opc,
    uint32_t        ip,
    err_t*          base
) {
    wchar_t* msg;
    err_t* err;

    if (info == NULL) {
        msg = err_msg_new(ERR_MAX_MSG_LENGTH,
            L"Unknown opcode %d at %zu", opc, ip
        );
    } else {
        msg = err_msg_new(ERR_MAX_MSG_LENGTH,
            L"Unknown opcode %d at %zu: %S", opc, ip, info
        );
    }
    err = err_new_base(
        ERR_T_UNKNOWN_OPC, msg,
        err_ctx_new_unknown_opc(cartridge, opc, ip),
        base
    );

    return err;
}

err_t* err_new_segfault(
    const wchar_t*  info,
    cartridge_t     cartridge,
    int32_t         opc,
    uint32_t        ip,
    err_t*          base
) {
    wchar_t* msg;
    err_t* err;

    if (info == NULL) {
        msg = err_msg_new(ERR_MAX_MSG_LENGTH,
            L"Segmentation fault with opcode %d at %zu", opc, ip
        );
    } else {
        msg = err_msg_new(ERR_MAX_MSG_LENGTH,
            L"Segmentation fault with opcode %d at %zu: %S", opc, ip, info
        );
    }
    err = err_new_base(
        ERR_T_SEGFAULT, msg,
        err_ctx_new_segfault(cartridge, opc, ip),
        base
    );

    return err;
}

void err_free(
    err_t*  err
) {
    free(err->msg);
    if (err->base != NULL) {
        err_free(err->base);
    }
    free(err);
}

wchar_t* err_to_str(
    err_t*  err
) {
    uint32_t errs, str_len, offset, msg_len;
    err_t* cur_err;
    wchar_t* str; wchar_t* msg;

    errs = 0;
    str_len = 0;
    offset = 0;
    cur_err = err;
    do {
        errs++;
        str_len += wcslen(cur_err->msg);
        cur_err = cur_err->base;
    } while (cur_err != NULL);
    str_len += (errs - 1) * 2 + 3;
    str = (wchar_t*)malloc(str_len * sizeof(wchar_t));
    cur_err = err;
    do {
        str[offset++] = L'\t';
        msg = cur_err->msg;
        msg_len = wcslen(msg);
        memcpy(&str[offset], msg, msg_len * sizeof(wchar_t));
        offset += msg_len;
        cur_err = cur_err->base;
        str[offset++] = L'\n';
    } while (cur_err != NULL);
    str[offset] = L'\0';
    
    return str;
}

void crash(
    err_t*  err
) {
    cli_crash(err);
    exit(1);
}
