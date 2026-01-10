#ifndef HOST_DEBUG_H
#define HOST_DEBUG_H

#include <pthread.h>
#include <stdbool.h>

#include "vm.h"

// ==============================
// DEBUG STATE
// ==============================

typedef enum
{
    VM_DEBUG_RUNNING, // VM executing normally
    VM_DEBUG_PAUSED,  // VM paused by debugger
    VM_DEBUG_STEP,    // Execute single step
    VM_DEBUG_STOPPED  // VM stopped (halted or error)
} VMDebugState;

// ==============================
// SHARED DEBUG CONTEXT
// ==============================

typedef struct
{
    VM* vm;                          // Pointer to VM instance
    pthread_mutex_t vm_mutex;        // Mutex for VM access
    volatile VMDebugState state;     // Current debug state
    volatile bool vm_thread_running; // VM thread should keep running
    volatile bool debugger_running;  // Debugger UI should keep running
    pthread_t vm_thread;             // VM thread handle

    // Frame buffer for VM rendering (thread-safe copy)
    uint8_t framebuffer[FB_SIZE]; // Copy of VM framebuffer
    pthread_mutex_t fb_mutex;     // Mutex for framebuffer access
} VMDebugContext;


int host_debug_run(VM* vm);

#endif // HOST_DEBUG_H
