#pragma once

#include <stdint.h>

#include "string.h"

typedef enum ENUM_ERROR_TYPE {
    ERROR_TYPE_UNKNOWN = 0,
} ENUM_ERROR_TYPE;

/*typedef union error_context_t {

} error_context_t;*/

typedef struct error_t {
    ENUM_ERROR_TYPE type;
    string_t message;
    /*error_context_t context;*/
    struct error_t* base;
} error_t;

void error_print(error_t* error, uint32_t indent_size);

void crash(error_t* error);

error_t* error_new_unknown();
