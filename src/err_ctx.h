#pragma once

#include <stdlib.h>

typedef union err_ctx_t {
    struct {
        size_t size;
    } memory_allocation;
    struct {
        const char* fname;
        const char* modes;
    } open_file;
    struct {
        const char* fname;
        const char* modes;
    } working_with_file;
    struct {
        const char* fname;
        const char* modes;
    } close_file;
} err_ctx_t;

err_ctx_t err_ctx_new_memory_allocation(size_t size);
err_ctx_t err_ctx_new_open_file(const char* fname, const char* modes);
err_ctx_t err_ctx_new_working_with_file(const char* fname, const char* modes);
err_ctx_t err_ctx_new_close_file(const char* fname, const char* modes);
