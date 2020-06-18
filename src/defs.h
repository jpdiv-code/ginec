#pragma once

#include "err.h"

#define OPTIONAL(type) struct { type value; err_t* err }
