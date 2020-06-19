#include "err.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>
#include <string.h>
#include <stdarg.h>

#include "err_ctx.h"
#include "cli.h"

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

wchar_t* err_to_str(err_t* err) {
    uint32_t errs = 0, str_len = 0, offset = 0, msg_len;
    err_t* cur_err = err;
    wchar_t* str;
    wchar_t* msg;
    do {
        errs++;
        str_len += wcslen(cur_err->msg);
        cur_err = cur_err->base;
    } while (cur_err != NULL);
    str_len += (errs - 1) * 2 + 2;
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

void crash(err_t* err) {
    cli_crash(err);
    exit(1);
}
