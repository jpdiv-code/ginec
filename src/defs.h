#pragma once

#include "error.h"

#define OPTIONAL(type) struct { type value; error_t* error }
