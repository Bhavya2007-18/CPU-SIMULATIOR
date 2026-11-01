# Instruction Set Architecture (ISA)

An 8-bit CPU with 8-bit addresses. Each instruction is 1–3 bytes:

```
[opcode] [operand byte(s)]
```

The first byte is always the **opcode** (values `0x00`–`0x14`). Operand bytes
follow in a fixed, deterministic order depending on the opcode.

## Registers

| Code | Register | Role                     |
|------|----------|--------------------------|
| 0x00 | A        | Accumulator (general)     |
| 0x01 | B        | General purpose           |
| 0x02 | C        | General purpose           |
| 0x03 | D        | General purpose           |

Register codes outside `0x00–0x03` are invalid and rejected by the decoder.

## Flags

| Flag | Name        | Meaning                                   |
|------|-------------|-------------------------------------------|
| Z    | Zero        | Last result was `0`                        |
| C    | Carry       | ADD: unsigned carry-out; SUB/DEC: borrow   |
| S    | Sign        | Bit 7 of the last result                   |
| V    | Overflow    | Signed-arithmetic overflow                 |

Logical operations (`AND OR XOR NOT`) clear C and V and set Z/S from the result.
`LOAD`, `STORE`, `MOV`, jumps, `PUSH`, `POP`, `CALL`, `RET` do **not** change
the flags. Only `ADD SUB INC DEC AND OR XOR NOT CMP` set flags.

## Instruction encoding

| Opcode | Hex  | Mnemonic      | Encoding (bytes)            | Len | Flags | Effect                          |
|--------|------|---------------|-----------------------------|-----|-------|---------------------------------|
| NOP    | 0x00 | `NOP`         | `00`                        | 1   | –     | no operation                     |
| HALT   | 0x01 | `HALT`        | `01`                        | 1   | –     | halt the CPU                     |
| LOAD   | 0x02 | `LOAD Rd,imm` | `02 [Rd] [imm]`            | 3   | –     | `Rd = imm`                       |
| STORE  | 0x03 | `STORE Rd,addr`| `03 [Rd] [addr]`         | 3   | –     | `mem[addr] = Rd`                 |
| MOV    | 0x04 | `MOV Rd,Rs`   | `04 [Rd] [Rs]`             | 3   | –     | `Rd = Rs`                        |
| ADD    | 0x05 | `ADD Rd,Rs`   | `05 [Rd] [Rs]`             | 3   | Z C S V| `Rd = Rd + Rs`                   |
| SUB    | 0x06 | `SUB Rd,Rs`   | `06 [Rd] [Rs]`             | 3   | Z C S V| `Rd = Rd - Rs`                   |
| AND    | 0x07 | `AND Rd,Rs`   | `07 [Rd] [Rs]`             | 3   | Z S   | `Rd = Rd & Rs`                   |
| OR     | 0x08 | `OR  Rd,Rs`   | `08 [Rd] [Rs]`             | 3   | Z S   | `Rd = Rd | Rs`                   |
| XOR    | 0x09 | `XOR Rd,Rs`   | `09 [Rd] [Rs]`             | 3   | Z S   | `Rd = Rd ^ Rs`                   |
| NOT    | 0x0A | `NOT Rd`      | `0A [Rd]`                  | 2   | Z S   | `Rd = ~Rd`                       |
| INC    | 0x0B | `INC Rd`      | `0B [Rd]`                  | 2   | Z C S V| `Rd = Rd + 1`                   |
| DEC    | 0x0C | `DEC Rd`      | `0C [Rd]`                  | 2   | Z C S V| `Rd = Rd - 1`                   |
| CMP    | 0x0D | `CMP Rd,Rs`   | `0D [Rd] [Rs]`             | 3   | Z C S V| flags from `(Rd - Rs)`, no write |
| JMP    | 0x0E | `JMP addr`    | `0E [addr]`                | 2   | –     | `PC = addr`                      |
| JZ     | 0x0F | `JZ  addr`    | `0F [addr]`                | 2   | –     | `if Z  then PC = addr`           |
| JNZ    | 0x10 | `JNZ addr`    | `10 [addr]`                | 2   | –     | `if !Z then PC = addr`           |
| PUSH   | 0x11 | `PUSH Rd`     | `11 [Rd]`                  | 2   | –     | `mem[--SP] = Rd`                 |
| POP    | 0x12 | `POP  Rd`     | `12 [Rd]`                  | 2   | –     | `Rd = mem[SP++]`                 |
| CALL   | 0x13 | `CALL addr`   | `13 [addr]`                | 2   | –     | push `(PC+2); PC = addr`         |
| RET    | 0x14 | `RET`         | `14`                       | 1   | –     | `PC = pop()`                     |

The instruction length is a pure function of the opcode
(`instructionLength()` in `isa.h`), so the assembler, decoder and CPU all agree.

## Worked encoding example

```
LOAD A, 10        ->  02 00 0A      (opcode=LOAD, reg=A=0x00, imm=10=0x0A)
ADD  A, B         ->  05 00 01      (opcode=ADD,  reg=A=0x00, reg=B=0x01)
STORE A, 50       ->  03 00 32      (opcode=STORE,reg=A=0x00, addr=50=0x32)
JMP  20           ->  0E 14         (opcode=JMP,  addr=20=0x14)
HALT              ->  01           (opcode=HALT)
```

## Stack

- `SP` starts at `0xFF`. The stack grows **downward**.
- `PUSH Rd`: pre-decrement `SP`, then `mem[SP] = Rd`.
- `POP Rd`: `Rd = mem[SP]`, then post-increment `SP`.
- `CALL addr`: push the return address (the address right after the 2-byte
  `CALL`), then `PC = addr`. `RET` pops it back into `PC`.
- Overflow/underflow are detected: pushing with `SP == 0x00` raises *stack
  overflow*; popping with `SP == 0xFF` (empty) raises *stack underflow*.

## Notes / design choices

- `LOAD` loads an **immediate**; `STORE` writes a register to memory. The base
  ISA intentionally has no arbitrary memory-to-register load — a memory round
  trip is possible through `PUSH`/`POP` (the stack). This keeps the ISA small and
  matches the acceptance program; an `LDR` could be added later without
  disturbing the encoding.
- Programs are expected to end with `HALT`. If execution runs off into unwritten
  (zero-filled) memory, `0x00` decodes as `NOP`; the `run()` step limit
  (`MAX_EXECUTION_STEPS`, default 100000) prevents an unbounded slide.
