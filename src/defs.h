#pragma once

#include "err.h"

#define OPTIONAL(type) struct { type value; err_t* err; }

typedef OPTIONAL(int32_t*) o_int32_tp;
