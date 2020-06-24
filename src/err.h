#pragma once

#include <stdint.h>
#include <wchar.h>

#include "err_ctx.h"
#include "cartridge.h"
#include "../test/test.h"

typedef enum ENUM_ERR_T {
    ERR_T_GENERAL       =   0,
    ERR_T_MALLOC        =   1,
    ERR_T_FILE          =   2,
    ERR_T_RUNTIME       =   3,
    ERR_T_UNKNOWN_OPC   =   4,
    ERR_T_SEGFAULT      =   5,
    ERR_T_TEST_FAILED   =   6,
} ENUM_ERR_T;

typedef struct err_t {
    ENUM_ERR_T      type;
    wchar_t*        msg;
    err_ctx_t       ctx;
    struct err_t*   base;
} err_t;

err_t* err_new_general(
    const wchar_t*  info,
    err_t*          base
);

err_t* err_new_malloc(
    const wchar_t*  info,
    size_t          size,
    err_t*          base
);

err_t* err_new_file(
    const wchar_t*  info,
    ENUM_ERR_FILE_T type,
    const char*     fname,
    const char*     modes,
    err_t*          base
);

err_t* err_new_runtime(
    const wchar_t*  info,
    cartridge_t     cartridge,
    uint32_t        ip,
    err_t*          base
);

err_t* err_new_unknown_opcode(
    const wchar_t*  info,
    cartridge_t     cartridge,
    int32_t         opc,
    uint32_t        ip,
    err_t*          base
);

err_t* err_new_segfault(
    const wchar_t*  info,
    cartridge_t     cartridge,
    int32_t         opc,
    uint32_t        ip,
    err_t*          base
);

err_t* err_new_test_failed(
    const wchar_t*  info,
    cartridge_t     cartridge,
    test_t          test,
    err_t*          base
);

void err_free(
    err_t*  err
);

wchar_t* err_to_str(
    err_t*  err
);

void crash(
    err_t*  err
);
