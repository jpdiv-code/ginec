#pragma once

#include <stdint.h>
#include <wchar.h>

#include "err_ctx.h"

typedef enum ENUM_ERR_T {
    ERR_T_GENERAL   =   0,
    ERR_T_MALLOC    =   1,
    ERR_T_FILE      =   2,
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

wchar_t* err_to_str(
    err_t*  err
);

void crash(
    err_t*  err
);
