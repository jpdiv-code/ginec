# GINEC Virtual Machine — Instruction Set Architecture

This document describes the instruction set architecture (ISA) of the GINEC virtual machine.

---

## 1. General execution model

- Instructions are fetched sequentially from **ROMB** (bytecode memory).
- All instructions are variable-length.
- Multi-byte operands are encoded **little-endian**.
- The instruction pointer (`IP`) always points to the next opcode byte.
- Invalid or out-of-bounds memory access results in **undefined behavior**.

---

## 2. Registers

- The VM provides 4 **general-purpose registers** (`reg[0..3]`).
- Each register is **16-bit wide**.
- Register operands in instructions (`RD`, `RA`, `RS`, etc.) always refer to a **register index**, not a value.
- The value stored in a register is accessed as `reg[index]`.

---

## 3. Operand width and `.w` suffix

### 3.1 Byte vs Word operations

Instructions follow a strict width convention:

- **Instructions without `.w` suffix** operate on the **low 8 bits** of the destination register.

  - The high 8 bits of the register are **preserved**.

- **Instructions with `.w` suffix** operate on the **full 16-bit register**.

### 3.2 Byte operations (no `.w`)

For byte-sized instructions:

- Input values are taken from `reg[RD] & 0xFF`.
- Results are written back to the low byte only:

  ```
  reg[RD] = (reg[RD] & 0xFF00) | result8
  ```

### 3.3 Word operations (`.w`)

For word-sized instructions:

- Input values use the full 16-bit register.
- Results replace the full register value.

---

## 4. Immediate values

- `imm8` values are **unsigned 8-bit**, unless explicitly stated otherwise.
- `imm16` values are **unsigned 16-bit**, encoded little-endian.
- Some instructions interpret `imm8` as **signed** (`int8_t`), explicitly noted in their description (e.g. offsets, stack adjustment).

---

## 5. Memory model

### 5.1 Address spaces

The VM has three distinct address spaces:

- **RAM** — mutable data memory
- **ROMA** — read-only asset/data memory
- **ROMB** — read-only bytecode memory

Instructions explicitly specify which address space they operate on.

### 5.2 Memory access rules

- All memory addresses are 16-bit.
- Address operands always represent an **offset into the corresponding address space**, not a host pointer.
- Word accesses (`.w`) are **little-endian**:

  ```
  low  byte = mem[addr]
  high byte = mem[addr + 1]
  ```

---

## 6. Stack model

The VM has **two independent hardware stacks** located in RAM:

### 6.1 Data Stack

- The data stack is used by `PUSH`, `POP`, `DUP`, `SWP`, and `ADJSP` instructions.
- The data stack pointer (`SP`) grows **downwards**.
- Push operations decrement `SP`, pop operations increment it.
- Initial `SP` value: `0xFDFF`
- Data stack occupies RAM range `0x60A0–0xFDFF` (grows downwards from `0xFDFF`)

### 6.2 Call Stack

- The call stack is used by `CALL`, `CALLR`, `RET`, `PUSHC.w`, `POPC.w`, and `ADJCSP` instructions.
- The call stack pointer (`CSP`) grows **downwards**.
- Call operations decrement `CSP`, return operations increment it.
- Initial `CSP` value: `0xFFFF`
- Call stack occupies RAM range `0xFE00–0xFFFF` (512 bytes, 256 addresses)
- This separation ensures that function calls do not interfere with data stack operations.
- `PUSHC.w` and `POPC.w` allow manual manipulation of the call stack for advanced use cases.

### 6.3 `ADJSP`

- `ADJSP imm8` adjusts the **data stack pointer** by a **signed** 8-bit value:

  ```
  SP = SP + (int8_t)imm8
  ```

- This instruction is typically used for:

  - allocating local variables,
  - cleaning up function arguments,
  - stack frame management.

### 6.4 `ADJCSP`

- `ADJCSP imm8` adjusts the **call stack pointer** by a **signed** 8-bit value:

  ```
  CSP = CSP + (int8_t)imm8
  ```

- This instruction is typically used for:

  - advanced call stack manipulation,
  - stack unwinding,
  - custom calling conventions.

---

## 7. Flags register

The VM maintains four condition flags:

| Flag | Name     | Meaning                         |
| ---- | -------- | ------------------------------- |
| Z    | Zero     | Result is zero                  |
| N    | Negative | Sign bit of result is set       |
| C    | Carry    | Carry (ADD) or borrow (SUB/CMP) |
| V    | Overflow | Signed overflow                 |

### 7.1 Flag update policy

- Arithmetic instructions (`ADD`, `SUB`, `CMP`, `MUL`, etc.) update **Z, N, C, V**.
- Logical instructions (`AND`, `OR`, `XOR`, `NOT`) update **Z and N**; **C and V are cleared**.
- Shift instructions update **Z and N**; **C may capture the shifted-out bit**; **V is cleared**.
- `CMP` behaves like `SUB` but discards the result.

---

## 8. Signed vs unsigned comparisons

### 8.1 Unsigned comparisons

After `CMP a, b`:

- `C == 1` → `a < b` (unsigned, borrow occurred)
- `C == 0` → `a >= b` (unsigned)

### 8.2 Signed comparisons

After `CMP a, b`:

- Signed less-than:

  ```
  (N != V)
  ```

- Signed greater-or-equal:

  ```
  (N == V)
  ```

These rules are used by `JLT` / `JGE`.

---

## 9. Sign extension

### 9.1 `SEXT8.w`

- `SEXT8.w RD` sign-extends the low 8 bits of `RD` to a 16-bit signed value:

  ```
  reg[RD] = (int16_t)(int8_t)(reg[RD] & 0xFF)
  ```

- This instruction is required when working with signed byte values.

---

## 10. Control flow

### 10.1 Jumps

- Jump instructions may be:

  - unconditional,
  - conditional (based on flags),
  - immediate, register-based, or stack-based.

### 10.2 Conditional jumps

Conditional jumps evaluate flags **set by the most recent flag-setting instruction** (typically `CMP`).

Examples:

- `JZ` — jump if `Z == 1` (equal / zero)
- `JNZ` — jump if `Z == 0` (not equal)
- `JC` — jump if `C == 1` (unsigned less-than)
- `JNC` — jump if `C == 0` (unsigned greater-or-equal)
- `JLT` — jump if signed less-than
- `JGE` — jump if signed greater-or-equal

---

## 11. Rendering and synchronization

- The framebuffer resides in a predefined RAM region.
- Each pixel is encoded as a single byte.
- The meaning of pixel values (palette or fixed colors) is defined by the host environment.
- `SYNC`:

  - finalizes the current framebuffer contents,
  - triggers rendering,
  - synchronizes execution to the target frame rate.

---

## 12. Determinism and host interaction

- The VM itself is deterministic.

---

## 13. Pseudocode notation

The pseudocode column uses the following conventions:

| Notation                  | Meaning                                                                  |
| ------------------------- | ------------------------------------------------------------------------ |
| `RD`, `RS`, `RA`, etc.    | Value of the register (shorthand for `reg[index]`)                       |
| `lo(R)`                   | Low byte of register R (`reg[R] & 0xFF`)                                 |
| `hi(R)`                   | High byte of register R (`reg[R] >> 8`)                                  |
| `(i8)`, `(i16)`           | Cast to signed 8-bit / 16-bit                                            |
| `read16(mem, addr)`       | Read 16-bit little-endian: `mem[addr] \| (mem[addr+1] << 8)`             |
| `write16(mem, addr, val)` | Write 16-bit little-endian: `mem[addr] = lo(val); mem[addr+1] = hi(val)` |
| `push8(val)`              | `ram[sp] = val; sp -= 1`                                                 |
| `push16(val)`             | `ram[sp] = hi(val); sp -= 1; ram[sp] = lo(val); sp -= 1`                |
| `pop8()`                  | `sp += 1; tmp = ram[sp]; return tmp`                                     |
| `pop16()`                 | `sp += 1; lo = ram[sp]; sp += 1; hi = ram[sp]; return lo \| (hi << 8)`   |
| `call_push16(val)`        | `ram[csp] = hi(val); csp -= 1; ram[csp] = lo(val); csp -= 1`            |
| `call_pop16()`            | `csp += 1; lo = ram[csp]; csp += 1; hi = ram[csp]; return lo \| (hi << 8)` |
| `flags(expr)`             | Update Z, N, C, V flags based on expression result (no store)            |

---

## 14. Opcode table

The following table lists all instructions supported by the VM.

| hex | mnemonic | operands | bytes | pseudocode | description | section |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | NOP |  | 1 |  | no operation | OTHER |
| 1 |  |  |  |  |  |  |
| 2 | HLT |  | 1 | exit(0) | exit |  |
| 3 | SYNC |  | 1 |  | update io, render, wait for next 24 FPS frame |  |
| 4 | FILL | RA RP RL | 4 | for i in 0..RL-1: ram[RA+i] = lo(RP) | 8-bit memset |  |
| 5 | FILL.w | RA RP RL | 4 | for i in 0..RL-1: write16(ram, RA+2*i, RP) | 16-bit memset |  |
| 6 | SEED | RS | 2 | srand(RS == 0 ? time() : RS) | set random seed as RS reg value (if RS reg value is 0, time(NULL) would be used instead) |  |
| 7 | RAND | RD | 2 | lo(RD) = rand8() | store next random 8-bit value in RD reg low byte |  |
| 8 | RAND.w | RD | 2 | RD = rand16() | store next random 16-bit value in RD reg |  |
| 9 |  |  |  |  |  |  |
| A | MOV | RD RS | 3 | lo(RD) = lo(RS) | copy 8-bit value from RS reg low byte to RD reg low byte | REG |
| B | MOV.w | RD RS | 3 | RD = RS | copy 16-bit value from RS reg to RD reg |  |
| C | LDI | RD imm8 | 3 | lo(RD) = imm8 | load an 8-bit immediate value into RD reg low byte |  |
| D | LDI.w | RD imm16 | 4 | RD = imm16 | load a 16-bit immediate value into RD reg |  |
| E | SRB | RD | 2 | swap(lo(RD), hi(RD)) | swap low and high bytes inside RD reg |  |
| F |  |  |  |  |  |  |
| 10 | PUSHI | imm8 | 2 | push8(imm8) | put an 8-bit immediate value on top of the stack | STACK |
| 11 | PUSHI.w | imm16 | 3 | push16(imm16) | put a 16-bit immediate value on top of the stack |  |
| 12 | PUSH | RS | 2 | push8(lo(RS)) | put an 8-bit value from the RS reg low byte on top of the stack |  |
| 13 | PUSH.w | RS | 2 | push16(RS) | put a 16-bit value from the RS reg on top of the stack |  |
| 14 |  |  |  |  |  |  |
| 15 | POP | RD | 2 | lo(RD) = pop8() | pop an 8-bit value from the stack and put it into the RD reg low byte |  |
| 16 | POP.w | RD | 2 | RD = pop16() | pop a 16-bit value from the stack and put it into the RD reg |  |
| 17 |  |  |  |  |  |  |
| 18 | SWP |  | 1 | a = pop8(); b = pop8(); push8(a); push8(b) | swap top 2 8-bit stack values |  |
| 19 | SWP.w |  | 1 | a = pop16(); b = pop16(); push16(a); push16(b) | swap top 2 16-bit stack values |  |
| 1A |  |  |  |  |  |  |
| 1B | DUP |  | 1 | a = pop8(); push8(a); push8(a) | duplicate top 8-bit stack value |  |
| 1C | DUP.w |  | 1 | a = pop16(); push16(a); push16(a) | duplicate top 16-bit stack value |  |
| 1D |  |  |  |  |  |  |
| 1E | ADJSP | imm8 | 2 | sp += (i8)imm8 | add signed 8-bit immediate value to the SP |  |
| 1F |  |  |  |  |  |  |
| 20 | LD | RD addr | 4 | lo(RD) = ram[addr] | load an 8-bit value from the specified ram address into the RD reg low byte | RAM |
| 21 | LD.w | RD addr | 4 | RD = read16(ram, addr) | load a 16-bit value from the specified ram address into the RD reg |  |
| 22 | ST | addr RS | 4 | ram[addr] = lo(RS) | save an 8-bit value from the RS reg low byte to ram at the specified address |  |
| 23 | ST.w | addr RS | 4 | write16(ram, addr, RS) | save the 16-bit value from the RS reg to ram at the specified address |  |
| 24 |  |  |  |  |  |  |
| 25 | LDR | RD RA | 3 | lo(RD) = ram[RA] | same as LD/ST[.w], but addr lies in the reg[RA] |  |
| 26 | LDR.w | RD RA | 3 | RD = read16(ram, RA) | same |  |
| 27 | STR | RA RS | 3 | ram[RA] = lo(RS) | same |  |
| 28 | STR.w | RA RS | 3 | write16(ram, RA, RS) | same |  |
| 29 |  |  |  |  |  |  |
| 2A | LDRI | RD RA imm8 | 4 | lo(RD) = ram[RA+imm8] | same as LDR/STR[.w], but imm8 added to the addr |  |
| 2B | LDRI.w | RD RA imm8 | 4 | RD = read16(ram, RA+imm8) | same |  |
| 2C | STRI | RA imm8 RS | 4 | ram[RA+imm8] = lo(RS) | same |  |
| 2D | STRI.w | RA imm8 RS | 4 | write16(ram, RA+imm8, RS) | same |  |
| 2E |  |  |  |  |  |  |
| 2F |  |  |  |  |  |  |
| 30 | LDA | RD addr | 4 | lo(RD) = roma[addr] | load an 8-bit value from specified roma address into the RD reg low byte | ROMA |
| 31 | LDA.w | RD addr | 4 | RD = read16(roma, addr) | load a 16-bit value from the specified roma address into the RD reg |  |
| 32 |  |  |  |  |  |  |
| 33 | LDAR | RD RA | 3 | lo(RD) = roma[RA] | same as LDA[.w], but addr lies in the reg[RA] |  |
| 34 | LDAR.w | RD RA | 3 | RD = read16(roma, RA) | same |  |
| 35 | CPYRA | RD RS cnt | 4 | for i in 0..cnt-1: ram[RD+i] = roma[RS+i] | copy cnt bytes from roma[RS] to ram[RD] |  |
| 36 | SPRA | RC RA w h | 5 | render sprite from roma[RA] (w×h pixels) to framebuffer at coords in RC (x=lo, y=hi) | render sprite from ROMA to framebuffer |  |
| 37 |  |  |  |  |  |  |
| 38 |  |  |  |  |  |  |
| 39 |  |  |  |  |  |  |
| 3A |  |  |  |  |  |  |
| 3B |  |  |  |  |  |  |
| 3C |  |  |  |  |  |  |
| 3D |  |  |  |  |  |  |
| 3E |  |  |  |  |  |  |
| 3F |  |  |  |  |  |  |
| 40 | ADD | RD imm8 | 3 | lo(RD) += imm8 | add an 8-bit immediate value to RD reg low byte; update flags | ARITH. & LOGIC |
| 41 | SUB | RD imm8 | 3 | lo(RD) -= imm8 | subtract an 8-bit immediate value from the RD reg low byte; update flags |  |
| 42 | MUL | RD imm8 | 3 | lo(RD) *= imm8 | multiply RD reg low byte by an 8-bit immediate value; update flags |  |
| 43 | AND | RD imm8 | 3 | lo(RD) &= imm8 | store result of the bitwise AND between RD reg low byte and 8-bit immediate value in the RD reg low byte; update flags |  |
| 44 | OR | RD imm8 | 3 | lo(RD) \|= imm8 | store result of the bitwise OR between RD reg low byte and 8-bit immediate value in the RD reg low byte; update flags |  |
| 45 | XOR | RD imm8 | 3 | lo(RD) ^= imm8 | store result of the bitwise XOR between RD reg low byte and 8-bit immediate value in the RD reg low byte; update flags |  |
| 46 | CMP | RD imm8 | 3 | flags(lo(RD) - imm8) | update flags as if it was SUB instruction execution (basically SUB without RD reg update) |  |
| 47 |  |  |  |  |  |  |
| 48 | ADD.w | RD imm16 | 4 | RD += imm16 | same as ADD/SUB/MUL/AND/OR/XOR/CMP, but operate full RD reg and imm16 |  |
| 49 | SUB.w | RD imm16 | 4 | RD -= imm16 | same |  |
| 4A | MUL.w | RD imm16 | 4 | RD *= imm16 | same |  |
| 4B | AND.w | RD imm16 | 4 | RD &= imm16 | same |  |
| 4C | OR.w | RD imm16 | 4 | RD \|= imm16 | same |  |
| 4D | XOR.w | RD imm16 | 4 | RD ^= imm16 | same |  |
| 4E | CMP.w | RD imm16 | 4 | flags(RD - imm16) | same |  |
| 4F |  |  |  |  |  |  |
| 50 | ADDR | RD RS | 3 | lo(RD) += lo(RS) | same as ADD/SUB/MUL/AND/OR/XOR/CMP, but with RS reg low byte instead of imm8 |  |
| 51 | SUBR | RD RS | 3 | lo(RD) -= lo(RS) | same |  |
| 52 | MULR | RD RS | 3 | lo(RD) *= lo(RS) | same |  |
| 53 | ANDR | RD RS | 3 | lo(RD) &= lo(RS) | same |  |
| 54 | ORR | RD RS | 3 | lo(RD) \|= lo(RS) | same |  |
| 55 | XORR | RD RS | 3 | lo(RD) ^= lo(RS) | same |  |
| 56 | CMPR | RD RS | 3 | flags(lo(RD) - lo(RS)) | same |  |
| 57 |  |  |  |  |  |  |
| 58 | ADDR.w | RD RS | 3 | RD += RS | same as ADD/SUB/MUL/AND/OR/XOR/CMP, but operate full RD reg and with RS reg value instead of imm16 |  |
| 59 | SUBR.w | RD RS | 3 | RD -= RS | same |  |
| 5A | MULR.w | RD RS | 3 | RD *= RS | same |  |
| 5B | ANDR.w | RD RS | 3 | RD &= RS | same |  |
| 5C | ORR.w | RD RS | 3 | RD \|= RS | same |  |
| 5D | XORR.w | RD RS | 3 | RD ^= RS | same |  |
| 5E | CMPR.w | RD RS | 3 | flags(RD - RS) | same |  |
| 5F |  |  |  |  |  |  |
| 60 | INC | RD | 2 | lo(RD) += 1 | add 1 to RD reg low byte; update flags |  |
| 61 | INC.w | RD | 2 | RD += 1 | add 1 to RD reg; update flags |  |
| 62 | DEC | RD | 2 | lo(RD) -= 1 | subtract 1 from RD reg low byte; update flags |  |
| 63 | DEC.w | RD | 2 | RD -= 1 | subtract 1 from RD reg; update flags |  |
| 64 |  |  |  |  |  |  |
| 65 | NOT | RD | 2 | lo(RD) = ~lo(RD) | bitwise not RD reg low byte; update flags |  |
| 66 | NOT.w | RD | 2 | RD = ~RD | bitwise not RD reg; update flags |  |
| 67 | SHL | RD imm8 | 3 | lo(RD) <<= imm8 | perform left shift by imm8 bits on RD reg low byte; update flags |  |
| 68 | SHL.w | RD imm8 | 3 | RD <<= imm8 | perform left shift by imm8 bits on RD reg; update flags |  |
| 69 | SHR | RD imm8 | 3 | lo(RD) >>= imm8 | perform right shift by imm8 bits on RD reg low byte; update flags |  |
| 6A | SHR.w | RD imm8 | 3 | RD >>= imm8 | perform right shift by imm8 bits on RD reg; update flags |  |
| 6B | ASR | RD imm8 | 3 | lo(RD) = (i8)lo(RD) >> imm8 | perform arithmetic right shift by imm8 bits on RD reg low byte; update flags |  |
| 6C | ASR.w | RD imm8 | 3 | RD = (i16)RD >> imm8 | perform arithmetic right shift by imm8 bits on RD reg; update flags |  |
| 6D |  |  |  |  |  |  |
| 6E | SEXT8.w | RD | 2 | RD = (i16)(i8)lo(RD) | extend signed 8-bit value from RD reg low byte to the full RD reg |  |
| 6F |  |  |  |  |  |  |
| 70 | JMP | addr | 3 | ip = addr | jump to addr address | JMPS |
| 71 | JZ | addr | 3 | if (Z) ip = addr | jump to addr address if Z is 1 (==) |  |
| 72 | JNZ | addr | 3 | if (!Z) ip = addr | jump to addr address if Z is 0 (!=) |  |
| 73 | JC | addr | 3 | if (C) ip = addr | jump to addr address if C is 1 (unsigned <) |  |
| 74 | JNC | addr | 3 | if (!C) ip = addr | jump to addr address if C is 0 (unsigned >=) |  |
| 75 | JLT | addr | 3 | if (N != V) ip = addr | jump to addr address if N != V (signed <) |  |
| 76 | JGE | addr | 3 | if (N == V) ip = addr | jump to addr address if N == V (signed >=) |  |
| 77 |  |  |  |  |  |  |
| 78 | JMPR | RA | 2 | ip = RA | same as JMP/JZ/JNZ/JC/JNC/JLT/JGE, but addr lies in the reg[RA] |  |
| 79 | JZR | RA | 2 | if (Z) ip = RA | same |  |
| 7A | JNZR | RA | 2 | if (!Z) ip = RA | same |  |
| 7B | JCR | RA | 2 | if (C) ip = RA | same |  |
| 7C | JNCR | RA | 2 | if (!C) ip = RA | same |  |
| 7D | JLTR | RA | 2 | if (N != V) ip = RA | same |  |
| 7E | JGER | RA | 2 | if (N == V) ip = RA | same |  |
| 7F |  |  |  |  |  |  |
| 80 | PUSHC.w | RS | 2 | call_push16(RS) | put a 16-bit value from the RS reg on top of the call stack | CSTACK |
| 81 | POPC.w | RD | 2 | RD = call_pop16() | pop a 16-bit value from the call stack and put it into the RD reg |  |
| 82 | ADJCSP | imm8 | 2 | csp += (i8)imm8 | add signed 8-bit immediate value to the CSP |  |
| 83 |  |  |  |  |  |  |
| 84 |  |  |  |  |  |  |
| 85 |  |  |  |  |  |  |
| 86 |  |  |  |  |  |  |
| 87 |  |  |  |  |  |  |
| 88 |  |  |  |  |  |  |
| 89 |  |  |  |  |  |  |
| 8A |  |  |  |  |  |  |
| 8B |  |  |  |  |  |  |
| 8C |  |  |  |  |  |  |
| 8D |  |  |  |  |  |  |
| 8E |  |  |  |  |  |  |
| 8F |  |  |  |  |  |  |
| 90 | CALL | addr | 3 | call_push16(ip + 3); ip = addr | push next instruction address on call stack, jump to addr address | CALLS |
| 91 | CALLR | RA | 2 | call_push16(ip + 2); ip = RA | same as CALL, but addr lies in the reg[RA] |  |
| 92 | RET |  | 1 | ip = call_pop16() | pop return address from call stack and jump to it |  |
