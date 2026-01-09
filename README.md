# ginec

ginec - is non-existing console

## Building and Running

### Normal Mode (Single Window)

Build and run the VM in normal mode:

```bash
make
./ginec
```

Or use the convenience target:

```bash
make run
```

### Debug Mode (Dual Windows with Debugger)

Build with debug mode enabled:

```bash
make debug
./ginec
```

This opens **two windows**:
- **Main Window** - VM execution and game display
- **Debugger Window** - UI controls and VM state inspection

The debugger runs in a separate thread, so it remains responsive even if the VM hangs.

See [DEBUG.md](DEBUG.md) for detailed documentation.

### Assembling Programs

To assemble a program:

```bash
cd asm
python3 gac.py examples/life.asm ../game.roma ../game.romb
```

or:

```bash
make example/02
```

This creates `game.roma` and `game.romb` files.

### Running the VM

The VM loads `game.roma` and `game.romb` from the current working directory:

```bash
make run
```

## Technical Architecture

### Address Spaces

The virtual machine exposes **three independent 16-bit address spaces**, each ranging from `0x0000` to `0xFFFF`.

| Space | Name         | Purpose                                                      |
| ----- | ------------ | ------------------------------------------------------------ |
| ROMA  | ROM Assets   | Read-only asset storage (audio samples, lookup tables, etc.) |
| ROMB  | ROM Bytecode | Read-only executable bytecode                                |
| RAM   | RAM          | Read-write working memory                                    |

Address spaces are isolated. Instructions explicitly specify the target space.

---

### Registers

#### General Purpose Registers

- `R0`–`R3` — 4 general-purpose 16-bit registers

Registers are untyped.

#### Special Registers

- `IP` — Instruction Pointer (16-bit, ROMB)
- `SP` — Stack Pointer (16-bit, RAM)
- `FLAGS` — Status flags:
  - `Z` — Zero
  - `N` — Negative
  - `C` — Carry
  - `V` — Overflow

---

### Instruction Set Architecture

Described in detail in [ISA.md](ISA.md).

---

### Graphics System

#### Display Parameters

- Fixed resolution: **180 × 136**
- 1 byte per pixel (256 VGA colors)

#### Framebuffer

- Linear, row-major layout
- Total framebuffer size: **24,480 bytes**

---

### Rendering via Memory Mapping

The framebuffer resides entirely in RAM and is modified directly by the VM.

Rendering is deferred until `SYNC` is executed.

---

### Frame Synchronization Instruction

#### `SYNC` instruction

- Finalizes the current VM frame
- Signals the host to render the most recently prepared framebuffer
- Blocks VM execution until the next frame boundary
- Enforces a fixed **24 FPS** virtual frame rate
- All input latch states are updated on `SYNC`

---

### Input System

The VM exposes a **fixed set of 12 digital buttons**.

Input state is represented as a **16-bit bitmask** stored in RAM.
Unused bits must be zero.

#### Button Bit Layout

| Bit | Button |
| --: | ------ |
|   0 | UP     |
|   1 | DOWN   |
|   2 | LEFT   |
|   3 | RIGHT  |
|   4 | A      |
|   5 | B      |
|   6 | X      |
|   7 | Y      |
|   8 | L      |
|   9 | R      |
|  10 | START  |
|  11 | SELECT |

Bits 12–15 are reserved and must be zero.

#### RAM Input Registers (MMIO)

| Offset | Purpose                                  |
| -----: | ---------------------------------------- |
| `0x00` | INPUT_DOWN (current state)               |
| `0x02` | INPUT_PRESSED (latched since last SYNC)  |
| `0x04` | INPUT_RELEASED (latched since last SYNC) |

- INPUT_PRESSED and INPUT_RELEASED are accumulated by the host
- All latched input state is cleared automatically on `SYNC`
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
- `SYNC` defines the canonical frame boundary

The VM runs in a single thread.

---

### Fixed RAM Layout

Total RAM size: **65,536 bytes**

| Address Range   | Size     | Purpose                            |
| --------------- | -------- | ---------------------------------- |
| `0x0000–0x00FF` | 256 B    | MMIO (input, audio)                |
| `0x0100–0x609F` | 24,480 B | Framebuffer (180 × 136)            |
| `0x60A0–0xFDFF` | 40,288 B | General-purpose RAM and data stack |
| `0xFE00–0xFFFF` | 512 B    | Call stack (CALL/RET)              |

Initial stack pointer: SP = 0xFDFF  
Initial call stack pointer: CSP = 0xFFFF

Out-of-bounds memory access is undefined behavior.
