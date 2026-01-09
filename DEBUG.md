# GINEC Debug Mode

## Quick Start

```bash
# Normal mode - single window
make
./ginec

# Debug mode - dual windows + debugger
make debug
./ginec
```

## Architecture

GINEC uses a modular host-layer architecture:

```
┌──────────────────────────────────────────────┐
│ main.c (Entry Point)                         │
│  - SDL initialization                        │
│  - ROM loading                               │
│  - Mode selection (#ifdef DEBUG_MODE)        │
└──────────────────┬───────────────────────────┘
                   │
        ┌──────────┴──────────┐
        │                     │
┌───────▼────────┐   ┌────────▼────────┐
│ host_normal.c  │   │ host_debug.c    │
│ Single window  │   │ Dual windows    │
│ Synchronous    │   │ + Threading     │
└───────┬────────┘   └────────┬────────┘
        │                     │
        └──────────┬──────────┘
                   │
        ┌──────────▼───────────┐
        │ host_common.c        │
        │ - Input handling     │
        │ - Timing utilities   │
        │ - MMIO updates       │
        └──────────┬───────────┘
                   │
        ┌──────────▼───────────┐
        │ vm.c                 │
        │ - Opcode execution   │
        │ - Register/Memory    │
        └──────────────────────┘
```

### Debug Mode Threading

```
Main Thread (UI)              Worker Thread (VM)
┌─────────────────────┐       ┌─────────────────────┐
│ debugger_ui_run()   │       │ vm_thread_main()    │
├─────────────────────┤       ├─────────────────────┤
│ • Creates windows   │       │ • Executes VM       │
│ • Nuklear UI        │       │ • Updates input     │
│ • Renders FB        │       │ • Copies FB         │
│ • Event handling    │       │ • Frame timing      │
└─────────────────────┘       └─────────────────────┘
         ↕                             ↕
    ┌────────────────────────────────────┐
    │  VMDebugContext (shared state)     │
    │  • pthread_mutex_t vm_mutex        │
    │  • pthread_mutex_t fb_mutex        │
    │  • VMDebugState state              │
    │  • uint8_t framebuffer[FB_SIZE]    │
    └────────────────────────────────────┘
```

## File Structure

### Headers
| File | Purpose |
|------|---------|
| `host_common.h` | Common utilities API (timing, input) |
| `host_normal.h` | Normal mode entry point |
| `host_debug.h` | Debug mode entry point + VMDebugContext |
| `vm.h` | VM core API |

### Implementation
| File | Purpose | Dependencies |
|------|---------|--------------|
| `main.c` | Entry point only | `vm.h`, `host_*.h` |
| `host_common.c` | Input & timing | SDL2 |
| `host_normal.c` | Single window loop | `host_common.h`, `vm.h` |
| `host_debug.c` | Threading + Nuklear UI | `host_common.h`, `vm.h`, Nuklear |
| `vm.c` | VM execution | None (pure VM) |

## Key Components

### VMDebugContext (host_debug.h)
```c
typedef struct {
    VM* vm;                          // VM instance
    pthread_mutex_t vm_mutex;        // Protects VM state
    pthread_mutex_t fb_mutex;        // Protects framebuffer
    volatile VMDebugState state;     // RUNNING|PAUSED|STEP|STOPPED
    volatile bool vm_thread_running; // Worker thread flag
    volatile bool debugger_running;  // UI thread flag
    pthread_t vm_thread;             // Worker thread handle
    uint8_t framebuffer[FB_SIZE];    // Thread-safe FB copy
} VMDebugContext;
```

### Thread Synchronization

**Worker Thread (vm_thread_main):**
```c
pthread_mutex_lock(&vm_mutex);
  vm_step(vm);                    // Execute instructions
pthread_mutex_unlock(&vm_mutex);

pthread_mutex_lock(&fb_mutex);
  memcpy(framebuffer, ...);       // Copy FB for rendering
pthread_mutex_unlock(&fb_mutex);
```

**Main Thread (debugger_ui_run):**
```c
pthread_mutex_lock(&fb_mutex);
  render_framebuffer(...);        // Read FB copy
pthread_mutex_unlock(&fb_mutex);
```

## Controls

| Window | Key | Action |
|--------|-----|--------|
| Any | ESC | Exit program |
| Any | Close | Exit program |
| VM Window | Arrows | Movement |
| VM Window | Z/X/A/S | Buttons A/B/X/Y |
| VM Window | Q/W | Shoulder L/R |
| VM Window | Enter | Start |
| VM Window | Shift | Select |

## Technical Notes

- **macOS compatibility:** All SDL windows created in main thread
- **Thread safety:** Dual mutex strategy (VM state + framebuffer)
- **Zero overhead:** Debug code compiled out in normal mode
- **Performance:** Worker thread runs VM at full speed, UI capped at ~60 FPS
- **Nuklear warnings:** Suppressed via `#pragma GCC diagnostic`

## Common Issues

**Q: VM window freezes but debugger responsive?**  
A: Working as designed! VM likely in infinite loop without SYNC. Use debugger to pause/inspect.

**Q: Build error "NSWindow main thread"?**  
A: Ensure `host_debug.c` creates windows in `debugger_ui_run()`, not in worker thread.

**Q: Race condition crashes?**  
A: Always lock appropriate mutex before accessing VM or framebuffer.

## Architecture Benefits

✅ **Separation of concerns** - clear layer boundaries  
✅ **No code duplication** - shared host_common functions  
✅ **VM portability** - vm.c has zero SDL dependency  
✅ **Testability** - each layer independently testable  
✅ **Maintainability** - changes isolated to appropriate file
