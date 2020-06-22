#pragma once

#include <wchar.h>

#include "../src/err.h"

typedef err_t* (*test_ft)();

typedef struct test_t {
    const wchar_t*  name;
    const wchar_t*  desc;
    test_ft         fn;
} test_t;

void test();
