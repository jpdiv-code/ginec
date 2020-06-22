#pragma once

#include "err.h"
#include "cartridge.h"

#define OPTIONAL(type) struct { type value; err_t* err; }

typedef OPTIONAL(cartridge_t) o_cartridge_t;
