#include "err.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>
#include <string.h>
#include <stdarg.h>

#include "err_ctx.h"

const uint32_t INDENT_SIZE = 2;
const uint32_t ERR_MAX_FILENAME_LENGTH = 256;

err_t* err_new_base(ENUM_ERR_T type, wchar_t* msg, err_ctx_t ctx, err_t* base) {
    err_t* err = (err_t*)malloc(sizeof(err_t));
    err->type = type;
    err->msg = msg;
    err->ctx = ctx;
    err->base = base;
    return err;
}

wchar_t* err_msg_new(uint32_t max_msg_len, const wchar_t* msg_base, ...) {
    wchar_t* msg = (wchar_t*)malloc(max_msg_len * sizeof(wchar_t));
    va_list args;
    va_start(args, msg_base);
    vswprintf(msg, max_msg_len, msg_base, args);
    va_end(args);
    msg = (wchar_t*)realloc(msg, (wcslen(msg) + 1) * sizeof(wchar_t));
    return msg;
}

err_t* err_new_unknown() {
    err_ctx_t ctx;
    return err_new_base(ERR_T_UNKNOWN, L"Unknown err", ctx, NULL);
}

err_t* err_new_memory_allocation(size_t size, err_t* base) {
    return err_new_base(
        ERR_T_MEMORY_ALLOCATION,
        err_msg_new(44, L"Error allocating %d bytes of memory", size),
        err_ctx_new_memory_allocation(size), base
    );
}

err_t* err_new_open_file(const char* fname, const char* modes, err_t* base) {
    return err_new_base(
        ERR_T_OPEN_FILE, 
        err_msg_new(41 + ERR_MAX_FILENAME_LENGTH,
            L"Error of opening \"%s\" file with \"%s\" modes", fname, modes),
        err_ctx_new_open_file(fname, modes), base
    );
}

err_t* err_new_working_with_file(const char* fname, const char* modes, const wchar_t* info, err_t* base) {
    return err_new_base(
        ERR_T_WORKING_WITH_FILE,
        err_msg_new(40 + ERR_MAX_FILENAME_LENGTH,
            L"Error of working with the file \"%s\" with modes \"%s\": %S", fname, modes, info),
        err_ctx_new_working_with_file(fname, modes), base
    );
}

err_t* err_new_close_file(const char* fname, const char* modes, err_t* base) {
    return err_new_base(
        ERR_T_CLOSE_FILE,
        err_msg_new(41 + ERR_MAX_FILENAME_LENGTH,
            L"Error of closing \"%s\" file with \"%s\" modes", fname, modes),
        err_ctx_new_close_file(fname, modes), base
    );
}

void err_print(err_t* err, uint32_t indent_count) {
    uint32_t indent_len = indent_count * INDENT_SIZE + 1;
    wchar_t indent_str[indent_len];
    uint32_t i;
    for (i = 0; i < indent_len; i++) {
        indent_str[i] = L' ';
    }
    indent_str[indent_len - 1] = L'\0';
    fwprintf(stderr,
        L"%SError(%d): %S\n", indent_str, err->type, err->msg);
    if (err->base != NULL) {
        err_print(err->base, indent_count + 1);
    }
}

void crash(err_t* err) {
    fwprintf(stderr, L"Crash!\n");
    err_print(err, 1);
    exit(1);
}
