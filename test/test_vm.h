#ifndef TEST_VM_H
#define TEST_VM_H

#include "vm.h"
#include <string.h>

static inline void init_test_vm(VM* vm)
{
    memset(vm, 0, sizeof(VM));
    vm->ip = 0x0000;
    vm->sp = 0xFDFF;
    vm->csp = 0xFFFF;
    vm->flags = 0;
}

static inline StepResult vm_execute_steps(VM* vm, int steps)
{
    for (int i = 0; i < steps; i++)
    {
        StepResult res = vm_step(vm);
        if (res != STEP_OK)
        {
            return res;
        }
    }
    return STEP_OK;
}

static inline StepResult vm_execute_until_halt(VM* vm, int max_steps)
{
    for (int i = 0; i < max_steps; i++)
    {
        StepResult res = vm_step(vm);
        if (res == STEP_HALT)
        {
            return STEP_HALT;
        }
        if (res == STEP_ERROR)
        {
            return STEP_ERROR;
        }
        if (res == STEP_SYNC)
        {
            continue; // Skip SYNC in tests
        }
    }
    return STEP_ERROR; // Max steps exceeded
}

#define FLAG_Z 0x01
#define FLAG_N 0x02
#define FLAG_C 0x04
#define FLAG_V 0x08

#define HAS_FLAG(vm, flag) ((vm)->flags & (flag))
#define NO_FLAG(vm, flag) (!((vm)->flags & (flag)))

#endif // TEST_VM_H
