#include "test.h"

#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>

#include "../src/err.h"

err_t* test_dummy() {
    return NULL;
}

test_t tests[] = {
    { L"dummy", L"always succeeds", test_dummy }
};

void test() {
    uint32_t tests_number, i, passed, failed;
    test_t test;
    err_t* err;
    tests_number = sizeof(tests) / sizeof(test_t);
    passed = 0;
    failed = 0;
    for (i = 0; i < tests_number; i++) {
        test = tests[i];
        wprintf(L"[%zu]\n\tname: %S\n\tdesc: %S\n", i, test.name, test.desc);
        err = test.fn();
        if (err == NULL) {
            wprintf(L"\tsuccess: yes\n\n");
            passed++;
        } else {
            wprintf(L"\tsuccess: no\n\terror:\n\t%S\n\n", err_to_str(err));
            failed++;
        }
    }
    wprintf(L"passed: %zu\nfailed: %zu\n", passed, failed);
}
