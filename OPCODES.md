| ƒ | mnemonic | operands | bytes | pseudocode | description | section |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | NOP |  | 1 |  | no operation | OTHER |
| 1 |  |  |  |  |  |  |
| 2 | HLT |  | 1 | exit(0) | exit |  |
| 3 | SYNC |  | 1 |  | update io, render, wait for next 24 FPS frame |  |
| 4 | FILL | RA RP RL | 4 | for i in 0..reg[RL)-1: ram[reg[RA]+i] = (reg[RP] & 0xFF) | 8-bit memset |  |
| 5 | FILL.w | RA RP RL | 4 | for i in 0..reg[RL)-1: write16(ram, reg[RA]+2*i, reg[RP]) | 16-bit memset |  |
| 6 | SEED | RS | 2 | if (reg[RS] == 0) srand((uint16_t)time(NULL)) else srand(reg[RS]) | set random seed as RS reg value (if RS reg value is 0, time(NULL) would be used instead) |  |
| 7 | RAND | RD | 2 | ((uint8_t*)&reg[RD])[0] = (uint8_t)(rand() & 0xFF) | store next random 8-bit value in RD reg low byte |  |
| 8 | RAND.w | RD | 2 | reg[RD] = (uint16_t)((rand() & 0xFF) \| ((rand() & 0xFF) << 8)) | store next random 16-bit value in RD reg |  |
| 9 |  |  |  |  |  |  |
| A | MOV | RD RS | 3 | ((uint8_t*)&reg[RD])[0] = ((uint8_t*)&reg[RS])[0] | copy 8-bit value from RS reg low byte to RD reg low byte | REG |
| B | MOV.w | RD RS | 3 | reg[RD] = reg[RS] | copy 16-bit value from RS reg to RD reg |  |
| C | LDI | RD imm8 | 3 | ((uint8_t*)&reg[RD])[0] = imm8 | load an 8-bit immediate value into RD reg low byte |  |
| D | LDI.w | RD imm16 | 4 | reg[RD] = imm16 | load a 16-bit immediate value into RD reg |  |
| E | SRB | RD | 2 | uint8_t* bytes = (uint8_t*)&reg[RD]; uint8_t tmp = bytes[0]; bytes[0] = bytes[1]; bytes[1] = tmp | swap low and high bytes inside RD reg |  |
| F |  |  |  |  |  |  |
| 10 | PUSHI | imm8 | 2 | sp -= 1; ram[sp] = imm8 | put an 8-bit immediate value on top of the stack | STACK |
| 11 | PUSHI.w | imm16 | 3 | sp -= 1; ram[sp] = imm16 & 0xFF00; sp -= 1; ram[sp] = imm16 & 0xFF; | put a 16-bit immediate value on top of the stack |  |
| 12 | PUSH | RS | 2 | sp -= 1; ram[sp] = reg[RS] & 0xFF | put an 8-bit value from the RS reg low byte on top of the stack |  |
| 13 | PUSH.w | RS | 2 | sp -= 1; ram[sp] = reg[RS] & 0xFF00; sp -= 1; ram[sp] = reg[RS] & 0xFF; | put a 16-bit value from the RS reg on top of the stack |  |
| 14 |  |  |  |  |  |  |
| 15 | POP | RD | 2 | ((uint8_t*)&reg[RD])[0] = ram[sp]; sp += 1 | pop an 8-bit value from the stack and put it into the RD reg low byte |  |
| 16 | POP.w | RD | 2 | reg[RD] = ram[sp]; sp += 1; reg[RD] &= ram[sp] << 8; sp += 1 | pop a 16-bit value from the stack and put it into the RD reg |  |
| 17 |  |  |  |  |  |  |
| 18 | SWP |  | 1 | a = pop(); b = pop(); push(a); push(b) | swap top 2 8-bit stack values |  |
| 19 | SWP.w |  | 1 | a = popw(); b = popw(); push(a); push(b) | swap top 2 16-bit stack values |  |
| 1A |  |  |  |  |  |  |
| 1B | DUP |  | 1 | a = pop(); push(a); push(a) | duplicate top 8-bit stack value |  |
| 1C | DUP.w |  | 1 | a = popw(); pushw(a); pushw(a) | duplicate top 16-bit stack value |  |
| 1D |  |  |  |  |  |  |
| 1E | ADJSP | imm8 | 2 | SP = SP + (int8_t)imm8; | add signed 8-bit immediate value to the SP |  |
| 1F |  |  |  |  |  |  |
| 20 | LD | RD addr | 4 | ((uint8_t*)&reg[RD])[0] = ram[addr] | load an 8-bit value from the specified ram address into the RD reg low byte | RAM |
| 21 | LD.w | RD addr | 4 | reg[RD] = ram[addr] \| (ram[addr+1] << 8) | load a 16-bit value from the specified ram address into the RD reg |  |
| 22 | ST | addr RS | 4 | ram[addr] = reg[RS] & 0xFF | save an 8-bit value from the RS reg low byte to ram at the specified address |  |
| 23 | ST.w | addr RS | 4 | ram[addr] = reg[RS] & 0xFF; ram[addr+1] = (reg[RS] >> 8) & 0xFF | save the 16-bit value from the RS reg to ram at the specified address |  |
| 24 |  |  |  |  |  |  |
| 25 | LDR | RD RA | 3 | ((uint8_t*)&reg[RD])[0] = ram[reg[RA]] | same as LD/ST[.w], but addr lies in the reg[RA] |  |
| 26 | LDR.w | RD RA | 3 | reg[RD] = ram[reg[RA]] \| (ram[reg[RA]+1] << 8) | same |  |
| 27 | STR | RA RS | 3 | ram[reg[RA]] = reg[RS] & 0xFF | same |  |
| 28 | STR.w | RA RS | 3 | ram[reg[RA]] = reg[RS] & 0xFF; ram[reg[RA]+1] = (reg[RS] >> 8) & 0xFF | same |  |
| 29 |  |  |  |  |  |  |
| 2A | LDRI | RD RA imm8 | 4 | ((uint8_t*)&reg[RD])[0] = ram[reg[RA]+imm8] | same as LDR/STR[.w], but imm8 added to the addr |  |
| 2B | LDRI.w | RD RA imm8 | 4 | reg[RD] = ram[reg[RA]+imm8] \| (ram[reg[RA]+imm8+1] << 8) | same |  |
| 2C | STRI | RA imm8 RS | 4 | ram[reg[RA]+imm8] = reg[RS] & 0xFF | same |  |
| 2D | STRI.w | RA imm8 RS | 4 | ram[reg[RA]+imm8] = reg[RS] & 0xFF; ram[reg[RA]+imm8+1] = (reg[RS] >> 8) & 0xFF | same |  |
| 2E |  |  |  |  |  |  |
| 2F |  |  |  |  |  |  |
| 30 | LDA | RD addr | 4 | ((uint8_t*)&reg[RD])[0] = roma[addr] | load an 8-bit value from specified roma address into the RD reg low byte | ROMA |
| 31 | LDA.w | RD addr | 4 | reg[RD] = roma[addr] \| (roma[addr+1] << 8) | load a 16-bit value from the specified roma address into the RD reg |  |
| 32 |  |  |  |  |  |  |
| 33 | LDAR | RD RA | 3 | ((uint8_t*)&reg[RD])[0] = roma[reg[RA]] | same as LDA[.w], but addr lies in the reg[RA] |  |
| 34 | LDAR.w | RD RA | 3 | reg[RD] = roma[reg[RA]] \| (roma[reg[RA]+1] << 8) | same |  |
| 35 |  |  |  |  |  |  |
| 36 |  |  |  |  |  |  |
| 37 |  |  |  |  |  |  |
| 38 |  |  |  |  |  |  |
| 39 |  |  |  |  |  |  |
| 3A |  |  |  |  |  |  |
| 3B |  |  |  |  |  |  |
| 3C |  |  |  |  |  |  |
| 3D |  |  |  |  |  |  |
| 3E |  |  |  |  |  |  |
| 3F |  |  |  |  |  |  |
| 40 | ADD | RD imm8 | 3 | ((uint8_t*)&reg[RD])[0] += imm8 | add an 8-bit immediate value to RD reg low byte; update flags | ARITH. & LOGIC |
| 41 | SUB | RD imm8 | 3 | ((uint8_t*)&reg[RD])[0] -= imm8 | subtract an 8-bit immediate value from the RD reg low byte; update flags |  |
| 42 | MUL | RD imm8 | 3 | ((uint8_t*)&reg[RD])[0] *= imm8 | multiply RD reg low byte by an 8-bit immediate value; update flags |  |
| 43 | AND | RD imm8 | 3 | ((uint8_t*)&reg[RD])[0] &= imm8 | store result of the bitwise AND between RD reg low byte and 8-bit immediate value in the RD reg low byte; update flags |  |
| 44 | OR | RD imm8 | 3 | ((uint8_t*)&reg[RD])[0] \|= imm8 | store result of the bitwise OR between RD reg low byte and 8-bit immediate value in the RD reg low byte; update flags |  |
| 45 | XOR | RD imm8 | 3 | ((uint8_t*)&reg[RD])[0] ^= imm8 | store result of the bitwise XOR between RD reg low byte and 8-bit immediate value in the RD reg low byte; update flags |  |
| 46 | CMP | RD imm8 | 3 |  | update flags as if it was SUB instruction execution (basically SUB without RD reg update) |  |
| 47 |  |  |  |  |  |  |
| 48 | ADD.w | RD imm16 | 4 | reg[RD] += imm16 | same as ADD/SUB/MUL/AND/OR/XOR/CMP, but operate full RD reg and imm16 |  |
| 49 | SUB.w | RD imm16 | 4 | reg[RD] -= imm16 | same |  |
| 4A | MUL.w | RD imm16 | 4 | reg[RD] *= imm16 | same |  |
| 4B | AND.w | RD imm16 | 4 | reg[RD] &= imm16 | same |  |
| 4C | OR.w | RD imm16 | 4 | reg[RD] \|= imm16 | same |  |
| 4D | XOR.w | RD imm16 | 4 | reg[RD] ^= imm16 | same |  |
| 4E | CMP.w | RD imm16 | 4 |  | same |  |
| 4F |  |  |  |  |  |  |
| 50 | ADDR | RD RS | 3 | ((uint8_t*)&reg[RD])[0] += reg[RS] & 0xFF | same as ADD/SUB/MUL/AND/OR/XOR/CMP, but with RS reg low byte instead of imm8 |  |
| 51 | SUBR | RD RS | 3 | ((uint8_t*)&reg[RD])[0] -= reg[RS] & 0xFF | same |  |
| 52 | MULR | RD RS | 3 | ((uint8_t*)&reg[RD])[0] *= reg[RS] & 0xFF | same |  |
| 53 | ANDR | RD RS | 3 | ((uint8_t*)&reg[RD])[0] &= reg[RS] & 0xFF | same |  |
| 54 | ORR | RD RS | 3 | ((uint8_t*)&reg[RD])[0] \|= reg[RS] & 0xFF | same |  |
| 55 | XORR | RD RS | 3 | ((uint8_t*)&reg[RD])[0] ^= reg[RS] & 0xFF | same |  |
| 56 | CMPR | RD RS | 3 |  | same |  |
| 57 |  |  |  |  |  |  |
| 58 | ADDR.w | RD RS | 3 | reg[RD] += reg[RS] | same as ADD/SUB/MUL/AND/OR/XOR/CMP, but operate full RD reg and with RS reg value instead of imm16 |  |
| 59 | SUBR.w | RD RS | 3 | reg[RD] -= reg[RS] | same |  |
| 5A | MULR.w | RD RS | 3 | reg[RD] *= reg[RS] | same |  |
| 5B | ANDR.w | RD RS | 3 | reg[RD] &= reg[RS] | same |  |
| 5C | ORR.w | RD RS | 3 | reg[RD] \|= reg[RS] | same |  |
| 5D | XORR.w | RD RS | 3 | reg[RD] ^= reg[RS] | same |  |
| 5E | CMPR.w | RD RS | 3 |  | same |  |
| 5F |  |  |  |  |  |  |
| 60 | INC | RD | 2 | ((uint8_t*)&reg[RD])[0] += 1 | add 1 to RD reg low byte; update flags |  |
| 61 | INC.w | RD | 2 | reg[RD] += 1 | add 1 to RD reg; update flags |  |
| 62 | DEC | RD | 2 | ((uint8_t*)&reg[RD])[0] -= 1 | subtract 1 from RD reg low byte; update flags |  |
| 63 | DEC.w | RD | 2 | reg[RD] -= 1 | subtract 1 from RD reg; update flags |  |
| 64 |  |  |  |  |  |  |
| 65 | NOT | RD | 2 | ((uint8_t*)&reg[RD])[0] = ~((uint8_t*)&reg[RD])[0] | bitwise not RD reg low byte; update flags |  |
| 66 | NOT.w | RD | 2 | reg[RD] = ~reg[RD] | bitwise not RD reg; update flags |  |
| 67 | SHL | RD imm8 | 3 | ((uint8_t*)&reg[RD])[0] <<= imm8 | perform left shift by imm8 bits on RD reg low byte; update flags |  |
| 68 | SHL.w | RD imm8 | 3 | reg[RD] <<= imm8 | perform left shift by imm8 bits on RD reg; update flags |  |
| 69 | SHR | RD imm8 | 3 | ((uint8_t*)&reg[RD])[0] >>= imm8 | perform right shift by imm8 bits on RD reg low byte; update flags |  |
| 6A | SHR.w | RD imm8 | 3 | reg[RD] >>= imm8 | perform right shift by imm8 bits on RD reg; update flags |  |
| 6B | ASR | RD imm8 | 3 | int8_t low = (uint8_t)(reg[RD] & 0xFF); low >>= imm8; reg[RD] = (reg[RD] & 0xFF00) \| (uint8_t)low | perform arithmetic right shift by imm8 bits on RD reg low byte; update flags |  |
| 6C | ASR.w | RD imm8 | 3 | ((int16_t*)&reg[RD])[0] >>= imm8 | perform arithmetic right shift by imm8 bits on RD reg; update flags |  |
| 6D |  |  |  |  |  |  |
| 6E | SEXT8.w | RD | 2 | int8_t  ext = (int8_t)(reg[RD] & 0xFF); reg[RD] = (int16_t)ext | extend signed 8-bit value from RD reg low byte to the full RD reg |  |
| 6F |  |  |  |  |  |  |
| 70 | JMP | addr | 3 | ip = addr | jump to addr address | JMPS |
| 71 | JZ | addr | 3 | if (Z == 1) ip = addr | jump to addr address if Z is 1 (==) |  |
| 72 | JNZ | addr | 3 | if (Z == 0) ip = addr | jump to addr address if Z is 0 (!=) |  |
| 73 | JC | addr | 3 | if (C == 1) ip = addr | jump to addr address if C is 1 (unsigned <) |  |
| 74 | JNC | addr | 3 | if (C == 0) ip = addr | jump to addr address if C is 0 (unsigned >=) |  |
| 75 | JLT | addr | 3 | if (N != V) ip = addr | jump to addr address if N != V (signed <) |  |
| 76 | JGE | addr | 3 | if (N == V) ip = addr | jump to addr address if N == V (signed >=) |  |
| 77 |  |  |  |  |  |  |
| 78 | JMPR | RA | 2 | ip = reg[RA] | same as JMP/JZ/JNZ/JC/JNC/JLT/JGE, but addr lies in the reg[RA] |  |
| 79 | JZR | RA | 2 | if (Z == 1) ip = reg[RA] | same |  |
| 7A | JNZR | RA | 2 | if (Z == 0) ip = reg[RA] | same |  |
| 7B | JCR | RA | 2 | if (C == 1) ip = reg[RA] | same |  |
| 7C | JNCR | RA | 2 | if (C == 0) ip = reg[RA] | same |  |
| 7D | JLTR | RA | 2 | if (N != V) ip = reg[RA] | same |  |
| 7E | JGER | RA | 2 | if (N == V) ip = reg[RA] | same |  |
| 7F |  |  |  |  |  |  |
| 80 |  |  |  |  |  |  |
| 81 |  |  |  |  |  |  |
| 82 |  |  |  |  |  |  |
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
| 90 | CALL | addr | 3 | pushw(ip + 3); ip = addr | push next instruction address on stack, jump to addr address | CALLS |
| 91 | CALLR | RA | 2 | pushw(ip + 3); ip = reg[RA] | same as CALL, but addr lies in the reg[RA] |  |
