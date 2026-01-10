#ifndef HOST_COMMON_H
#define HOST_COMMON_H

#include "vm.h"
#include <stdint.h>

// ==============================
// TIME UTILITIES
// ==============================

double host_now_seconds(void);
void host_sleep_until(double target_time);

// ==============================
// INPUT UTILITIES
// ==============================

uint16_t host_read_input_state(void);
void host_update_input_mmio(VM* vm);
void host_clear_input_latches(VM* vm);

#endif // HOST_COMMON_H
