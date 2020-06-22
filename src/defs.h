#pragma once

#include "err.h"

#define OPTIONAL(type) struct { type value; err_t* err; }

typedef OPTIONAL(uint32_t*) o_uint32_tp;
