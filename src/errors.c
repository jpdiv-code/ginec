#include "errors.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>

const uint32_t INDENT_SIZE = 2;

void error_print(error_t* error, uint32_t indent_count) {
    uint32_t indent_length = indent_count * INDENT_SIZE + 1;
    wchar_t indent_string[indent_length];
    uint32_t i;
    for (i = 0; i < indent_length; i++) {
        indent_string[i] = L' ';
    }
    indent_string[indent_length - 1] = L'\0';
    fwprintf(stderr, L"%SError(%d): %S\n", indent_string, error->type, error->message);
    if (error->base != NULL) {
        error_print(error->base, indent_count + 1);
    }
}

void crash(error_t* error) {
    fwprintf(stderr, L"Crash!\n");
    error_print(error, 1);
    exit(1);
}

error_t* error_new_unknown() {
    error_t* error = (error_t*)malloc(sizeof(error_t));
    error->type = ERROR_TYPE_UNKNOWN;
    error->message = L"Unknown error";
    error->base = NULL;
    return error;
}
