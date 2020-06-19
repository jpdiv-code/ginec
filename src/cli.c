#include "cli.h"

#include <stdio.h>

#include "err.h"

void cli_err(err_t* err) {
    fwprintf(stderr, L"Error:\n%S", err_to_str(err));
}

void cli_crash(err_t* err) {
    fwprintf(stderr, L"Crash:\n%S", err_to_str(err));
}
