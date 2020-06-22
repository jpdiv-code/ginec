#pragma once

#include <stdlib.h>

typedef enum ENUM_ERR_FILE_T {
    ERR_FILE_T_GENERAL      =   0,
    ERR_FILE_T_NOT_EXISTS   =   1,
    ERR_FILE_T_CANT_READ    =   2,
    ERR_FILE_T_EMPTY        =   3,
    ERR_FILE_T_CANT_CLOSE   =   4,
} ENUM_ERR_FILE_T;

typedef union err_ctx_t {
    struct {
        size_t  size;
    } malloc;

    struct {
        ENUM_ERR_FILE_T type;
        const char*     fname;
        const char*     modes;
    } file;
} err_ctx_t;

err_ctx_t err_ctx_new_malloc(
    size_t  size
);

err_ctx_t err_ctx_new_file(
    ENUM_ERR_FILE_T type,
    const char*     fname,
    const char*     modes
);
