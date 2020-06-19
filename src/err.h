#pragma once

#include <stdint.h>
#include <wchar.h>

#include "err_ctx.h"

typedef enum ENUM_ERR_T {
    ERR_T_UNKNOWN = 0,
    ERR_T_MEMORY_ALLOCATION = 1,
    ERR_T_OPEN_FILE = 2,
    ERR_T_WORKING_WITH_FILE = 3,
    ERR_T_CLOSE_FILE = 4,
} ENUM_ERR_T;

typedef struct err_t {
    ENUM_ERR_T type;
    wchar_t* msg;
    err_ctx_t ctx;
    struct err_t* base;
} err_t;

err_t* err_new_unknown();
err_t* err_new_memory_allocation(size_t size, err_t* base);
err_t* err_new_open_file(const char* fname, const char* modes, err_t* base);
err_t* err_new_working_with_file(const char* fname, const char* modes, const wchar_t* info, err_t* base);
err_t* err_new_close_file(const char* fname, const char* modes, err_t* base);

wchar_t* err_to_str(err_t* err);
void crash(err_t* err);
