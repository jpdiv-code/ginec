# ginec
ginec - is non-existing console

## Technical Architecture

### Address Spaces

The virtual machine exposes **three independent 16-bit address spaces**, each ranging from `0x0000` to `0xFFFF`.

| Space | Name  | Purpose |
|------|-------|---------|
| ROMA | ROM Assets | Read-only asset storage (palette, audio samples, lookup tables, etc.) |
| ROMB | ROM Bytecode | Read-only executable bytecode |
| RAM  | RAM | Read-write working memory |

Address spaces are isolated. Instructions explicitly specify the target space.

---

### Registers

#### General Purpose Registers
- `R0`–`R3` — 4 general-purpose 16-bit registers

Registers are untyped.

#### Special Registers
- `PC` — Program Counter (16-bit, ROMB)
- `SP` — Stack Pointer (16-bit, RAM)
- `FLAGS` — Status flags:
  - `Z` — Zero
  - `N` — Negative
  - `C` — Carry
  - `V` — Overflow

---

### Instruction Encoding

- Variable-length instructions
- Encoding format:
  - 1 byte opcode
  - Followed by N operand bytes
- No instruction alignment
- All multi-byte values are little-endian

---

### Instruction Set Overview (ISA)

This section describes instruction categories only.

#### Data Movement
- Register-to-register moves
- Immediate loads (8-bit and 16-bit)
- Load/store between registers and RAM
- Load from ROMA into registers
- Push/pop to/from stack in RAM (immediate and register forms)

#### Integer Arithmetic (16-bit)
- Addition, subtraction
- Bitwise logic
- Shifts
- Comparison (flag-setting only)

#### Control Flow
- Absolute and relative jumps, unconditional and conditional (on FLAGS)
- Call/return using the stack

---

### Graphics System

#### Display Parameters
- Fixed resolution: **180 × 136**
- 1 byte per pixel
- Each pixel stores a palette index
- Palette size: **32 colors**
- Only lower 5 bits of each pixel are used (`index & 0x1F`)

#### Framebuffer
- Linear, row-major layout
- Total framebuffer size: **24,480 bytes**

---

### Rendering via Memory Mapping

The framebuffer resides entirely in RAM and is modified directly by the VM.

Rendering is deferred until `VSYNC` is executed.

---

### Frame Synchronization Instruction

#### `VSYNC`

- Finalizes the current VM frame
- Signals the host to render the most recently prepared framebuffer
- Blocks VM execution until the next frame boundary
- Enforces a fixed **24 FPS** virtual frame rate

All input latch states are cleared on `VSYNC`.

#### `VSYNC` instruction
- Finalizes the current VM frame
- Signals the host to render the most recently prepared framebuffer
- Blocks VM execution until the next frame boundary
- Enforces a fixed **24 FPS** virtual frame rate
- All input latch states are updated on `VSYNC`

---

### Input System

The VM exposes a **fixed set of 12 digital buttons**.

Input state is represented as a **16-bit bitmask** stored in RAM.
Unused bits must be zero.

#### Button Bit Layout

| Bit | Button |
|----:|--------|
| 0 | UP |
| 1 | DOWN |
| 2 | LEFT |
| 3 | RIGHT |
| 4 | A |
| 5 | B |
| 6 | X |
| 7 | Y |
| 8 | L |
| 9 | R |
| 10 | START |
| 11 | SELECT |

Bits 12–15 are reserved and must be zero.

#### RAM Input Registers (MMIO)

| Offset | Purpose |
|------:|---------|
| `0x00` | INPUT_DOWN (current state) |
| `0x02` | INPUT_PRESSED (latched since last VSYNC) |
| `0x04` | INPUT_RELEASED (latched since last VSYNC) |

- INPUT_PRESSED and INPUT_RELEASED are accumulated by the host
- All latched input state is cleared automatically on `VSYNC`
- Host implementations may map these buttons to any physical input devices

---

### Audio System

#### Audio Format (Fixed)

All audio samples must use the following format:

- **Signed 8-bit PCM**
- **Mono**
- **Sample rate: 22050 Hz**

Host implementations must resample audio if required by the underlying audio system.

---

#### Sample Playback Instruction

A dedicated instruction starts playback of a PCM buffer stored in ROMA.

Parameters:
- ROMA address of sample data
- Sample length in bytes

---

### Audio Channels

#### Sample Channels
- **4 independent sample channels**
- Each channel supports:
  - Play
  - Stop
  - Optional looping

#### PSG Channels
- **2 PSG channels**
- Supported waveforms:
  - Square
  - Noise
- Each channel exposes:
  - Frequency
  - Volume
  - Gate (on/off)

All channels are mixed by the host.

---

### VM Execution Model

- Bytecode is executed sequentially from ROMB
- `VSYNC` defines the canonical frame boundary

The VM runs in a single thread.

---

### Fixed RAM Layout

Total RAM size: **65,536 bytes**

| Address Range | Size | Purpose |
|--------------|------|---------|
| `0x0000–0x00FF` | 256 B | MMIO (input, audio) |
| `0x0100–0x60AF` | 24,480 B | Framebuffer (180 × 136) |
| `0x60B0–0xDFFF` | 32,608 B | General-purpose RAM |
| `0xE000–0xFFFF` | 8,192 B | Stack (grows downward) |

Initial stack pointer: SP = 0xFFFF

Out-of-bounds memory access is undefined behavior.

---

### Fixed ROMA Layout

| Address Range | Purpose |
|--------------|---------|
| `0x0000–0x003F` | Palette (32 × RGB565) |
| `0x0040–0xFFFF` | Audio samples and other assets |

ROMA is strictly read-only from the VM perspective.

