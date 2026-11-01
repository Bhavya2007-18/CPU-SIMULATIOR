# Architecture

An educational 8-bit CPU simulated entirely in software (C++17). The goal is to
make the fundamental parts of a processor — and how they cooperate during the
fetch–decode–execute cycle — explicit and inspectable.

## Datapath overview

```
                ┌─────────────────┐
                │  Control Unit    │   (CPU::step / run)
                └────────┬────────┘
                         │
                         ▼
              ┌─────────────────────┐
              │         CPU         │
              │                     │
              │  PC → IR → Decoder  │   fetch → decode
              │                     │
              │  A B C D Registers  │   RegisterFile
              │                     │
              │      ┌───────┐      │
              │      │  ALU  │──────┼──→ Flags (Z C S V)
              │      └───────┘      │
              │                     │
              └──────────┬──────────┘
                         │
                         ▼
                       RAM (256 bytes)
```

## Data widths

| Quantity      | Width  | Type alias (`types.h`) |
|---------------|--------|------------------------|
| Data word     | 8 bit  | `Byte`   (`uint8_t`)   |
| Address       | 8 bit  | `Address`(`uint8_t`)   |
| Scratch       | 16 bit | `Word`   (`uint16_t`)  |
| Memory size   | 256 B  | `MEMORY_SIZE` (`std::size_t`, since 256 > 255) |

## Components

- **Registers** (`registers.h`) — `RegisterFile` holds the four general-purpose
  registers `A B C D` plus the special registers `PC` (program counter), `SP`
  (stack pointer, initialised to `0xFF`), and `IR` (instruction register, the
  opcode byte of the executing instruction). Access is through `read`/`write`.
- **Flags** (`flags.h`) — `Z` (zero), `C` (carry/borrow), `S` (sign), `V`
  (signed overflow). Set by ALU operations, read by conditional jumps.
- **ALU** (`alu.h`) — a standalone unit. `ADD SUB AND OR XOR NOT INC DEC`.
  Returns an `ALUResult { value, zero, carry, sign, overflow }`. The CPU never
  does arithmetic directly; it always routes operands through the ALU.
- **Memory** (`memory.h`) — 256 bytes with `read`, `write`, `reset`,
  `loadProgram`. Addresses are `uint8_t`, so every address is valid by
  construction; mutation is centralised (no raw array exposure).
- **Decoder** (`decoder.h`) — converts machine-code bytes at a `PC` into an
  `Instruction`. Rejects invalid opcodes, invalid register codes, and
  instructions that cross the end of memory with a `CpuException`.
- **CPU** (`cpu.h`) — the control unit. Owns registers, flags, memory, ALU and
  decoder. Drives `FETCH → DECODE → EXECUTE → WRITEBACK → FLAGS`. Owns the
  execution trace and the breakpoint set.
- **Assembler** (`assembler.h`) — two-pass: pass 1 assigns addresses and labels,
  pass 2 emits machine code and resolves label references. Produces clear,
  line-numbered `AssemblyError`s.
- **Debugger** (`debugger.h`) — wraps the CPU with high-level operations
  (step / run / reset) and renders human-readable reports. The CLI (`main.cpp`)
  drives it through an interactive REPL.

## Module independence

Each module exposes a narrow interface and depends only on lower-level
modules. The ALU knows nothing about registers or memory; the CPU composes them.
The decoder is stateless. This keeps the design easy to explain (and test) in a
viva, and leaves room for a future graphical front end that simply reads
`CPUState` and the `ExecutionTrace`.
