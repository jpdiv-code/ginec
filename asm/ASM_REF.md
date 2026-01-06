# GINEC Assembler Reference

Complete reference documentation for the GINEC virtual machine assembler.

---

## Table of Contents

1. [Overview](#overview)
2. [Usage](#usage)
3. [Source File Syntax](#source-file-syntax)
4. [Labels](#labels)
5. [Section Directives](#section-directives)
6. [Data Directives](#data-directives)
7. [Instructions](#instructions)
8. [Constants and Expressions](#constants-and-expressions)
9. [Macros](#macros)
10. [Assembly Process](#assembly-process)
11. [Instruction Encoding](#instruction-encoding)
12. [Output File Format](#output-file-format)
13. [Complete Example](#complete-example)

---

## Overview

The GINEC assembler is a two-pass assembler that converts assembly language source files into binary bytecode for the GINEC virtual machine. It is implemented as a single Python file (`gac.py`) with no external dependencies beyond the Python standard library.

### Key Features

- **Complete ISA support**: All 93 opcodes from the GINEC instruction set
- **Two-pass compilation**: Forward and backward label references
- **Memory sections**: ROMA (read-only assets) and ROMB (bytecode)
- **Expressions**: Arithmetic and bitwise operations in operands
- **Macros**: Text substitution with parameters and local labels
- **Data directives**: Binary data, strings, sprites (BMP), audio (WAV)
- **File inclusion**: `.inc` directive for modular source files
- **Error reporting**: Clear error messages with file and line numbers

### System Requirements

- Python 3.6 or later
- No external dependencies

---

## Usage

### Command Line Syntax

```bash
python3 gac.py <input.asm> [output_roma] [output_romb]
```

### Parameters

- `input.asm` - Input assembly source file
- `output_roma` - Output file for ROMA section (default: `output.roma`)
- `output_romb` - Output file for ROMB section (default: `output.romb`)

### Examples

```bash
# Basic usage (creates output.roma and output.romb)
python3 gac.py program.asm

# Specify custom output files
python3 gac.py program.asm game.roma game.romb

# Make executable and run
chmod +x gac.py
./gac.py program.asm
```

### Exit Codes

- **0** - Successful compilation
- **1** - Compilation error

---

## Source File Syntax

### Line Structure

```
[label:] [instruction|directive] [operands] [; comment]
```

All elements in square brackets are optional. Whitespace (spaces and tabs) is used as a delimiter.

### Comments

```asm
; This is a line comment
NOP              ; Comment after instruction
```

Comments start with `;` and continue to the end of the line.

### Case Sensitivity

- **Instructions and directives**: Case-insensitive (`MOV`, `mov`, `MoV` are equivalent)
- **Labels**: Case-sensitive (`Label` and `label` are different)
- **Macro names**: Case-insensitive

### Blank Lines

Empty lines and lines containing only whitespace are ignored.

---

## Labels

Labels mark specific addresses in memory and can be used in place of numeric addresses.

### Label Declaration

```asm
label_name:
    NOP

another_label:  MOV R0, R1
```

**Rules:**

- Must start with a letter or underscore
- Can contain letters, digits, and underscores
- End with a colon `:`
- Bind to the address of the next instruction or data
- Case-sensitive

### Label Usage

```asm
    JMP start         ; Forward reference

start:
    LDI R0, 10
    JMP start         ; Backward reference
```

Both forward and backward references are supported thanks to two-pass compilation.

### Local Labels in Macros

Local labels start with a dot and can include the `\@` suffix for uniqueness:

```asm
.local_label:
    NOP

.skip\@:          ; Unique per macro expansion
    NOP
```

---

## Section Directives

GINEC has two memory sections that can be assembled separately.

### `.section` - Select Active Section

```asm
.section romb     ; Switch to ROMB (bytecode)
.section roma     ; Switch to ROMA (assets)
```

**Details:**

- Default section is `romb`
- Can switch between sections multiple times
- Address counter advances automatically within each section
- RAM is not supported as a section (initialized to zeros by VM)

### `.org` - Set Address Origin

```asm
.section romb
.org 0x0100       ; Next data/code starts at address 0x0100 in ROMB
```

Sets the current address within the active section.

---

## Data Directives

### `.db` - Define Bytes

```asm
.db 0x10, 0x20, 0x30        ; Three bytes
.db 'H', 'e', 'l', 'l', 'o' ; ASCII characters
.db 0                        ; Single byte
```

Declares a sequence of 8-bit bytes.

### `.dw` - Define Words

```asm
.dw 0x1234        ; Stored as 0x34, 0x12 (little-endian)
.dw 0xABCD, 0x5678
```

Declares 16-bit words stored in little-endian format.

### `.string` - Define String

```asm
.string "Hello, World!"  ; ASCII string + null terminator
```

Declares a null-terminated ASCII string.

### `.reserve` - Reserve Bytes

```asm
.reserve 100      ; Reserve 100 bytes (filled with zeros)
```

Reserves a specified number of bytes initialized to zero.

### `.incbin` - Include Binary File

```asm
.incbin "data.bin"        ; Include entire file
.incbin "data.bin", 0, 100  ; Include 100 bytes starting from offset 0
```

Includes raw binary data from an external file. Optional offset and size parameters control which portion of the file to include.

### `.incbmp` - Include BMP Image

```asm
.incbmp "sprite.bmp"      ; Include pixel data from BMP
```

**Requirements:**

- 8-bit BMP files only (256 colors)
- Includes pixel data only (54-byte header + 1024-byte palette are stripped)
- Reads all remaining data after header and palette
- Image dimensions must be tracked separately by the program

### `.incwav` - Include WAV Audio

```asm
.incwav "sound.wav"       ; Include PCM data from WAV
```

**Requirements:**

- 8-bit mono PCM WAV files only
- Includes audio samples only (44-byte header is stripped)
- No resampling or conversion performed
- WAV file must be properly formatted

### `.inc` - Include Assembly File

```asm
.inc "constants.asm"      ; Include constants
.inc "macros.asm"         ; Include macro definitions
```

Includes another assembly file as if its contents were inserted at the directive location.

**Features:**

- Path can be relative or absolute (relative to the including file)
- Recursive inclusion supported during second pass (avoid circular dependencies)
- Current section preserved, but file context changes
- Note: First pass does not recursively process .inc files for label collection

---

## Instructions

The GINEC assembler supports all 93 opcodes in the ISA.

### Instruction Format

```asm
MNEMONIC [operands]
```

### Operand Types

#### Registers

Four general-purpose 16-bit registers: `R0`, `R1`, `R2`, `R3`

```asm
MOV R0, R1
ADD R2, 10
```

#### Immediate Values

Decimal, hexadecimal, or binary literals:

```asm
LDI R0, 42        ; Decimal
LDI R1, 0xFF      ; Hexadecimal (0x prefix)
LDI R2, 0b1010    ; Binary (0b prefix)
```

#### Addresses

Direct addresses or labels:

```asm
LD R0, 0x1000     ; Direct address
LD R0, data_label ; Label reference
JMP loop_start    ; Jump to label
```

### Instruction Categories

#### Basic Operations

```asm
NOP               ; No operation
HLT               ; Halt execution
SYNC              ; Synchronize frame (24 FPS)
```

#### Register Operations

```asm
MOV R0, R1        ; Copy 8-bit (low byte)
MOV.w R0, R1      ; Copy 16-bit (full register)
LDI R0, 100       ; Load immediate 8-bit
LDI.w R0, 1000    ; Load immediate 16-bit
SRB R0            ; Swap bytes (low ↔ high)
```

#### Stack Operations

```asm
PUSH R0           ; Push 8-bit
PUSH.w R0         ; Push 16-bit
POP R0            ; Pop 8-bit
POP.w R0          ; Pop 16-bit
PUSHI 42          ; Push immediate 8-bit
PUSHI.w 1000      ; Push immediate 16-bit
SWP               ; Swap top 2 values (8-bit)
SWP.w             ; Swap top 2 values (16-bit)
DUP               ; Duplicate top (8-bit)
DUP.w             ; Duplicate top (16-bit)
ADJSP -10         ; Adjust stack pointer (signed)
```

#### RAM Memory Operations

```asm
LD R0, 0x1000     ; Load 8-bit from RAM
LD.w R0, 0x1000   ; Load 16-bit from RAM
ST 0x1000, R0     ; Store 8-bit to RAM
ST.w 0x1000, R0   ; Store 16-bit to RAM
LDR R0, R1        ; Load via register address
LDR.w R0, R1      ; Load word via register
STR R1, R0        ; Store via register
STR.w R1, R0      ; Store word via register
LDRI R0, R1, 10   ; Load with offset
LDRI.w R0, R1, 10 ; Load word with offset
STRI R1, 10, R0   ; Store with offset
STRI.w R1, 10, R0 ; Store word with offset
```

#### ROMA Memory Operations

```asm
LDA R0, 0x2000    ; Load 8-bit from ROMA
LDA.w R0, 0x2000  ; Load 16-bit from ROMA
LDAR R0, R1       ; Load via register
LDAR.w R0, R1     ; Load word via register
```

#### Arithmetic and Logic (Immediate)

```asm
ADD R0, 10        ; R0 += 10 (8-bit)
ADD.w R0, 1000    ; R0 += 1000 (16-bit)
SUB R0, 5         ; R0 -= 5
MUL R0, 3         ; R0 *= 3
AND R0, 0x0F      ; R0 &= 0x0F
OR R0, 0x80       ; R0 |= 0x80
XOR R0, 0xFF      ; R0 ^= 0xFF
CMP R0, 42        ; Compare (update flags only)
```

All operations available in `.w` (16-bit) variant.

#### Arithmetic and Logic (Register)

```asm
ADDR R0, R1       ; R0 += R1 (8-bit)
ADDR.w R0, R1     ; R0 += R1 (16-bit)
SUBR R0, R1       ; R0 -= R1
MULR R0, R1       ; R0 *= R1
ANDR R0, R1       ; R0 &= R1
ORR R0, R1        ; R0 |= R1
XORR R0, R1       ; R0 ^= R1
CMPR R0, R1       ; Compare registers
```

All operations available in `.w` (16-bit) variant.

#### Unary Operations

```asm
INC R0            ; R0++ (8-bit)
INC.w R0          ; R0++ (16-bit)
DEC R0            ; R0-- (8-bit)
DEC.w R0          ; R0-- (16-bit)
NOT R0            ; R0 = ~R0 (8-bit)
NOT.w R0          ; R0 = ~R0 (16-bit)
SEXT8.w R0        ; Sign-extend 8-bit to 16-bit
```

#### Shift Operations

```asm
SHL R0, 1         ; Logical shift left (8-bit)
SHL.w R0, 1       ; Logical shift left (16-bit)
SHR R0, 1         ; Logical shift right (8-bit)
SHR.w R0, 1       ; Logical shift right (16-bit)
ASR R0, 1         ; Arithmetic shift right (8-bit)
ASR.w R0, 1       ; Arithmetic shift right (16-bit)
```

#### Jump Instructions

```asm
JMP label         ; Unconditional jump
JZ label          ; Jump if zero
JNZ label         ; Jump if not zero
JC label          ; Jump if carry (unsigned <)
JNC label         ; Jump if not carry (unsigned >=)
JLT label         ; Jump if signed <
JGE label         ; Jump if signed >=
JMPR R0           ; Jump to address in register
JZR R0            ; Conditional jump via register
JNZR R0
JCR R0
JNCR R0
JLTR R0
JGER R0
```

#### Function Calls

```asm
CALL function     ; Call function at address
CALLR R0          ; Call function at address in register
RET               ; Return from function
```

#### Special Operations

```asm
FILL R0, R1, R2   ; Memory fill (8-bit)
FILL.w R0, R1, R2 ; Memory fill (16-bit)
SEED R0           ; Set random seed
RAND R0           ; Get random 8-bit value
RAND.w R0         ; Get random 16-bit value
```

### Width Suffix `.w`

Instructions without the `.w` suffix operate on the **low 8 bits** of registers, preserving the high 8 bits. Instructions with `.w` operate on the **full 16-bit register**.

**Example:**

```asm
LDI R0, 0xFF      ; R0 = 0x??FF (high byte unchanged)
LDI.w R0, 0x1234  ; R0 = 0x1234 (full register)
```

---

## Constants and Expressions

### Numeric Literals

```asm
42          ; Decimal
0x2A        ; Hexadecimal
0b00101010  ; Binary
```

### Character Literals

```asm
'A'         ; ASCII code (0x41)
'\n'        ; Escape sequences: \n \r \t \\ \'
```

### Constants Definition

```asm
.equ SCREEN_WIDTH, 180
.equ SCREEN_HEIGHT, 136
.equ FRAMEBUFFER, 0x0100

LDI R0, SCREEN_WIDTH    ; Use constant
```

Constants are defined with `.equ` and substituted at compile time. They don't occupy memory.

### Expression Evaluation

Expressions can be used in instruction operands and data directives:

```asm
; Arithmetic operations
LDI.w R0, SPRITE_SIZE * 8         ; Multiplication
LDI.w R1, FRAMEBUFFER + 100       ; Addition
LD R0, BASE_ADDR - 4              ; Subtraction

; Address arithmetic
JMP loop_start + 10               ; Jump with offset
LDA R0, sprite_data + 16          ; Load with offset

; In data directives
.dw sprite_end - sprite_start     ; Data size
.org FRAMEBUFFER + (SCREEN_WIDTH * 10)  ; Computed address
```

### Supported Operators

**Arithmetic:**

- `+` Addition
- `-` Subtraction
- `*` Multiplication
- `/` Integer division
- `%` Modulo

**Bitwise:**

- `&` Bitwise AND
- `|` Bitwise OR
- `^` Bitwise XOR
- `~` Bitwise NOT (unary)
- `<<` Left shift
- `>>` Right shift

### Operator Precedence

1. `~` (unary)
2. `*`, `/`, `%`
3. `+`, `-`
4. `<<`, `>>`
5. `&`
6. `^`
7. `|`

Parentheses `()` can be used to override precedence.

**Implementation note**: Expressions are evaluated using Python's `eval()` with restricted builtins for security. This means operator precedence follows Python's rules.

---

## Macros

Macros provide text substitution with parameters, enabling code reuse and abstraction.

### Macro Definition

```asm
.macro SET_PIXEL, x, y, color
    LDI.w R0, FRAMEBUFFER
    LDI.w R1, \y
    LDI.w R2, SCREEN_WIDTH
    MULR.w R1, R2
    ADDR.w R0, R1
    LDI.w R1, \x
    ADDR.w R0, R1
    LDI R1, \color
    STR R0, R1
.endmacro
```

**Rules:**

- Name must be unique (case-insensitive)
- Parameters listed after name, comma-separated
- Parameters referenced with `\` prefix
- Body between `.macro` and `.endmacro`
- Expanded during compilation (text substitution)

### Macro Invocation

```asm
SET_PIXEL 10, 20, 0xFF    ; Set white pixel
SET_PIXEL 50, 60, 0x00    ; Set black pixel
```

### Local Labels

To avoid label conflicts when macros are used multiple times:

```asm
.macro CLAMP, reg, min, max
    CMP.w \reg, \min
    JNC .skip_min\@       ; \@ expands to unique number
    LDI.w \reg, \min
.skip_min\@:
    CMP.w \reg, \max
    JC .skip_max\@
    LDI.w \reg, \max
.skip_max\@:
.endmacro
```

The `\@` suffix is replaced with a unique number for each macro expansion.

### Useful Macro Examples

```asm
; Save all registers
.macro PUSH_ALL
    PUSH.w R0
    PUSH.w R1
    PUSH.w R2
    PUSH.w R3
.endmacro

; Restore all registers
.macro POP_ALL
    POP.w R3
    POP.w R2
    POP.w R1
    POP.w R0
.endmacro

; Load effective address
.macro LEA, reg, label
    LDI.w \reg, \label
.endmacro

; Test register for zero
.macro TEST, reg
    CMPR.w \reg, \reg
.endmacro
```

### Macro Limitations

- Cannot define macros inside other macros (no nesting)
- No recursive macro expansion
- Recommended maximum of 8 parameters
- Pure text substitution (no conditional expansion)

---

## Assembly Process

The assembler uses a two-pass compilation model to resolve forward references and generate correct addresses.

### Pass 1: Symbol Collection

**Goals:**

- Collect all labels and their addresses
- Define constants
- Store macro definitions
- Expand macros to calculate instruction sizes
- Compute final addresses

**Activities:**

- Read source file line by line
- Process directives (`.equ`, `.macro`, `.section`, `.org`)
- Record label positions
- Calculate instruction and data sizes
- Track address counters for each section

### Pass 2: Code Generation

**Goals:**

- Generate machine code
- Resolve label references
- Write binary output files

**Activities:**

- Re-read source file
- Expand macros with parameter substitution
- Encode instructions with resolved addresses
- Process data directives
- Emit bytes to section buffers
- Write output files

### Internal Data Structures

- **labels**: Symbol table (name → address)
- **constants**: Constant table (name → value)
- **macros**: Macro table (name → (parameters, body))
- **sections**: Binary buffers for ROMA and ROMB (65536 bytes each)
- **section_addresses**: Current write address in each section

### Error Handling

The assembler reports errors with filename and line number:

```
file.asm:15: error: undefined label 'unknown_label'
```

**Common errors:**

- Unknown instruction mnemonic
- Undefined label reference (evaluates to 0 - may not be caught as error)
- Incorrect operand count or type
- Immediate value out of range (values are masked to fit)
- Duplicate label definition
- Invalid register name
- File not found (for .inc, .incbin, .incbmp, .incwav)
- Invalid file format (for .incbmp, .incwav)

---

## Instruction Encoding

Instructions are encoded as a sequence of bytes starting with the opcode.

### Encoding Format

- **Opcode byte**: 1 byte
- **Operands**: Variable size based on instruction
  - Registers: 1 byte (0=R0, 1=R1, 2=R2, 3=R3)
  - Immediate 8-bit: 1 byte
  - Immediate 16-bit: 2 bytes (little-endian)
  - Address 16-bit: 2 bytes (little-endian)

### Encoding Examples

```
Instruction          Bytes                       Hex
-----------------    -------------------------   ---------------
NOP                  [0x00]                      00
HLT                  [0x02]                      02
LDI R0, 42           [0x0C, 0x00, 0x2A]          0C 00 2A
LDI.w R1, 300        [0x0D, 0x01, 0x2C, 0x01]    0D 01 2C 01
MOV R2, R3           [0x0A, 0x02, 0x03]          0A 02 03
JMP 0x0100           [0x70, 0x00, 0x01]          70 00 01
CALL 0x0200          [0x90, 0x00, 0x02]          90 00 02
RET                  [0x92]                      92
```

### Little-Endian Encoding

16-bit values are stored with the least significant byte first:

```
Value: 0x1234
Bytes: [0x34, 0x12]  (low byte first, then high byte)
```

---

## Output File Format

### Binary Files

The assembler generates two raw binary files:

- **output.roma** - ROMA section data (read-only assets)
- **output.romb** - ROMB section data (bytecode)

### File Structure

- No headers or metadata
- Raw binary data
- Uninitialized areas filled with zeros
- Maximum size: 65536 bytes (64 KB) per file
- 16-bit values encoded little-endian

### Typical Structure

```
output.romb:
  - Program bytecode starting at address 0x0000
  - Executable instructions
  - May include inline data

output.roma:
  - Sprite data from .incbmp directives
  - Audio samples from .incwav directives
  - Lookup tables and constants
  - Read-only data
```

---

## Complete Example

### Full Program with Multiple Features

```asm
; Sprite animation demo for GINEC
; Demonstrates most assembler features

; ===== Constants =====
.equ SCREEN_WIDTH, 180
.equ SCREEN_HEIGHT, 136
.equ FRAMEBUFFER, 0x0100
.equ SPRITE_SIZE, 16
.equ FB_SIZE, SCREEN_WIDTH * SCREEN_HEIGHT

; ===== Macros =====

.macro PUSH_ALL
    PUSH.w R0
    PUSH.w R1
    PUSH.w R2
    PUSH.w R3
.endmacro

.macro POP_ALL
    POP.w R3
    POP.w R2
    POP.w R1
    POP.w R0
.endmacro

; ===== Code Section =====

.section romb
.org 0x0000

start:
    ; Initialize sprite position
    LDI.w R0, 80            ; X position (screen center)
    LDI.w R1, 60            ; Y position (screen center)
    LDI.w R2, 1             ; DX (X velocity)
    LDI.w R3, 1             ; DY (Y velocity)

main_loop:
    ; Clear screen
    CALL clear_screen

    ; Draw sprite
    PUSH.w R0               ; Push X
    PUSH.w R1               ; Push Y
    CALL draw_sprite
    ADJSP 4                 ; Clean up parameters

    ; Update position
    ADDR.w R0, R2           ; X += DX
    ADDR.w R1, R3           ; Y += DY

    ; Check X boundaries
    CMPR.w R0, R2
    JC bounce_left
    LDI.w R2, SCREEN_WIDTH
    SUBR.w R2, R0
    CMPR.w R2, R3
    JC bounce_right
    JMP check_y

bounce_left:
    LDI.w R2, 1             ; DX = 1
    JMP check_y

bounce_right:
    LDI.w R2, -1            ; DX = -1
    SEXT8.w R2

check_y:
    ; Check Y boundaries
    CMPR.w R1, R3
    JC bounce_top
    LDI.w R3, SCREEN_HEIGHT
    SUBR.w R3, R1
    LDI R2, SPRITE_SIZE
    CMPR R3, R2
    JC bounce_bottom
    JMP frame_end

bounce_top:
    LDI.w R3, 1             ; DY = 1
    JMP frame_end

bounce_bottom:
    LDI.w R3, -1            ; DY = -1
    SEXT8.w R3

frame_end:
    SYNC                    ; Synchronize to 24 FPS
    JMP main_loop

; ===== Functions =====

; Clear screen to black
clear_screen:
    LDI.w R0, FRAMEBUFFER
    LDI R1, 0x00            ; Black color
    LDI.w R2, FB_SIZE
    FILL R0, R1, R2
    RET

; Draw 16x16 sprite
; Parameters: [SP+2] = X, [SP+0] = Y
draw_sprite:
    ; Get parameters
    LDR.w R0, SP            ; Y position
    LDRI.w R1, SP, 2        ; X position

    ; Calculate base address: FB + Y * 180 + X
    PUSH.w R1               ; Save X
    LDI.w R2, SCREEN_WIDTH
    MULR.w R0, R2           ; R0 = Y * 180
    POP.w R1                ; Restore X
    ADDR.w R0, R1           ; R0 = Y * 180 + X
    LDI.w R1, FRAMEBUFFER
    ADDR.w R0, R1           ; R0 = pixel address

    ; Copy sprite data
    LDI.w R1, sprite_data   ; Sprite address in ROMA
    LDI R2, SPRITE_SIZE     ; Row counter

draw_row:
    PUSH.w R0               ; Save row address
    PUSH.w R1               ; Save data address
    PUSH R2                 ; Save counter

    ; Copy 16 pixels
    LDI R3, SPRITE_SIZE
draw_pixel:
    LDAR R2, R1             ; Load sprite byte
    STR R0, R2              ; Write to framebuffer
    INC.w R0                ; Next pixel
    INC.w R1                ; Next sprite byte
    DEC R3
    JNZ draw_pixel

    ; Restore and advance to next row
    POP R2
    POP.w R1
    POP.w R0
    LDI.w R3, SCREEN_WIDTH
    ADDR.w R0, R3           ; Next row
    LDI R3, SPRITE_SIZE
    ADDR.w R1, R3           ; Next sprite row
    DEC R2
    JNZ draw_row

    RET

; ===== Data Section =====

.section roma
.org 0x0000

sprite_data:
    .incbmp "player.bmp"    ; 16x16 sprite (256 bytes)

sound_jump:
    .incwav "jump.wav"      ; Sound effect
```

### Compiling This Example

```bash
python3 gac.py demo.asm demo.roma demo.romb
```

Expected output:

```
First pass: demo.asm
Second pass: demo.asm
Writing demo.roma...
Writing demo.romb...
Assembly successful!
  ROMA size: XXXX bytes
  ROMB size: XXXX bytes
  Labels: XX
  Constants: 5
  Macros: 2
```

---

## Limitations and Notes

### Size Limits

- Maximum section size: 64 KB (0x0000-0xFFFF per section)
- No size limit on number of labels or constants
- Recommended maximum 8 parameters per macro

### File Format Support

- **BMP**: 8-bit indexed color only (256 colors)
- **WAV**: 8-bit mono PCM only
- File paths relative to source file location

### Expression Evaluation

- Basic arithmetic and bitwise operations using Python's `eval()`
- No floating-point support (results converted to integers)
- Division by zero results in Python exception during evaluation
- Evaluated at compile time
- **Important**: Undefined labels evaluate to 0 (may not produce errors)
- Values automatically masked to fit operand size (8-bit or 16-bit)

### Macro System

- No nested macro definitions
- No recursive expansion
- Pure text substitution
- Local labels with `\@` for uniqueness
- Parameters must use `\` prefix
- Macro names are case-insensitive

### Memory Model

- RAM section not supported for assembly (initialized to zeros by VM)
- ROMA and ROMB sections are independent 64KB address spaces
- Uninitialized areas in output files are filled with zeros
- Section address counter can be set with `.org` directive

### Character Encoding

- Source files must be UTF-8 encoded
- Character literals support escape sequences: `\n`, `\r`, `\t`, `\\`, `\'`, `\"`
- String literals also use UTF-8, but output as raw bytes
- ROMA is read-only at runtime
- ROMB is read-only at runtime

---

## Opcode Summary

The assembler supports all 93 opcodes:

| Category        | Instructions                                                            |
| --------------- | ----------------------------------------------------------------------- |
| **OTHER**       | NOP, HLT, SYNC, FILL, FILL.w, SEED, RAND, RAND.w                        |
| **REG**         | MOV, MOV.w, LDI, LDI.w, SRB                                             |
| **STACK**       | PUSHI, PUSHI.w, PUSH, PUSH.w, POP, POP.w, SWP, SWP.w, DUP, DUP.w, ADJSP |
| **RAM**         | LD, LD.w, ST, ST.w, LDR, LDR.w, STR, STR.w, LDRI, LDRI.w, STRI, STRI.w  |
| **ROMA**        | LDA, LDA.w, LDAR, LDAR.w                                                |
| **ARITH/LOGIC** | ADD, SUB, MUL, AND, OR, XOR, CMP (+ .w variants)                        |
|                 | ADDR, SUBR, MULR, ANDR, ORR, XORR, CMPR (+ .w variants)                 |
|                 | INC, INC.w, DEC, DEC.w, NOT, NOT.w                                      |
|                 | SHL, SHL.w, SHR, SHR.w, ASR, ASR.w                                      |
|                 | SEXT8.w                                                                 |
| **JUMPS**       | JMP, JZ, JNZ, JC, JNC, JLT, JGE                                         |
|                 | JMPR, JZR, JNZR, JCR, JNCR, JLTR, JGER                                  |
| **CALLS**       | CALL, CALLR, RET                                                        |

**Total: 93 opcodes**

---

## Implementation Notes

### Known Behaviors and Quirks

**Label Resolution:**

- Undefined labels evaluate to 0 instead of producing errors
- This is due to the expression evaluator returning 0 for failed evaluations
- Always check your label names carefully

**Expression Evaluation:**

- Uses Python's `eval()` with restricted builtins
- Whitespace in expressions is stripped before evaluation
- Complex expressions may fail silently, evaluating to 0

**Immediate Value Handling:**

- Values are masked to fit operand size (& 0xFF for 8-bit, & 0xFFFF for 16-bit)
- Overflow is silent - no warnings or errors
- Negative immediates work via two's complement when masked

**File Inclusion:**

- `.inc` directive only processes recursively during second pass
- This means labels in included files may not be available if they depend on first pass discovery
- Best practice: use `.inc` at the start of files

**BMP/WAV Files:**

- No validation of file format beyond header size check
- Assumes standard format (8-bit BMP with 256 colors, 8-bit mono PCM WAV)
- Malformed files may produce incorrect output or errors

**Macro Expansion:**

- Macros expand during both passes to ensure correct address calculation
- Local label uniqueness (`\@`) uses a global counter
- Macro parameters are simple text substitution (no type checking)

**Case Sensitivity:**

- Instructions: case-insensitive (`MOV` = `mov` = `MoV`)
- Directives: case-insensitive (`.DB` = `.db`)
- Labels: case-sensitive (`Label` ≠ `label`)
- Macro names: case-insensitive (stored in lowercase)
- Macro parameters: case-sensitive in usage

**Error Recovery:**

- No error recovery - first error stops assembly
- Error messages show file name and line number
- Some errors may be cryptic due to Python exceptions bubbling up

---

## Additional Resources

- **ISA.md** - Complete instruction set architecture specification
- **README.md** - GINEC virtual machine overview
- **gac.py** - "GINEC assembler compiler" source code
