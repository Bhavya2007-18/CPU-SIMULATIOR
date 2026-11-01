# CPU SIMULATOR

A software-only **8-bit CPU simulator** built from scratch in C++17 — ISA,
registers, ALU, memory, instruction decoder, assembler, fetch-decode-execute
engine, debugger, execution trace, and tests. A **Digital Logic & Computer
Design (DLCD) PBL** project.

> "We designed an 8-bit CPU architecture, defined our own instruction set,
> created an assembler, simulated memory and registers, implemented an ALU, and
> built a fetch-decode-execute engine capable of executing our machine code."

## Pipeline

```
Assembly ─► Assembler ─► Machine code ─► Memory ─► CPU
                                                      │
   ┌──────────────────────────────────────────────────┘
   ▼
 Fetch ─► Decode ─► Execute ─► ALU / Registers / Memory ─► CPU State ─► Debugger
```

## Build & run

You need a C++17 compiler (`g++`, `clang++`, or MSVC).

**GNU Make** (Linux / macOS / MinGW-make):

```bash
make            # build the simulator (./build/cpu-simulator)
make test       # build + run all unit tests
make run         # run the acceptance program
make clean
```

**Windows without Make** (this repo includes `build.ps1`):

```powershell
.\build.ps1            # build
.\build.ps1 test       # build + run all unit tests
.\build.ps1 run        # run the acceptance program
.\build.ps1 clean
```

The project compiles cleanly under `-Wall -Wextra -Wpedantic` (and `-Werror`).

## Usage

```bash
# Run a program to completion and print the final state:
./build/cpu-simulator --run examples/acceptance.asm

# Interactive debugger (step, breakpoints, memory, trace, ...):
./build/cpu-simulator examples/acceptance.asm
```

In the interactive REPL: `step`, `run`, `reset`, `state`, `regs`, `flags`,
`mem [start] [count]`, `trace [n]`, `break <addr>`, `unbreak <addr>`, `breaks`,
`help`, `quit`. See [`docs/debugger.md`](docs/debugger.md).

## Architecture

- **8-bit** data, **8-bit** addresses, **256 bytes** of RAM.
- Registers: `A B C D` (general purpose), `PC`, `IR`, `SP`.
- Flags: `Z` (zero), `C` (carry/borrow), `S` (sign), `V` (overflow).
- 21 instructions, 1–3 bytes each. Full encoding in [`docs/isa.md`](docs/isa.md).

## Instruction set

```
NOP HALT
LOAD  STORE  MOV
ADD SUB AND OR XOR NOT INC DEC CMP
JMP JZ JNZ
PUSH POP CALL RET
```

Example encoding: `ADD A, B` → `05 00 01` (opcode `0x05`, dst `A=0x00`, src `B=0x01`).

## Examples

| File                      | Demonstrates                                    |
|---------------------------|--------------------------------------------------|
| `examples/addition.asm`   | arithmetic + memory store                        |
| `examples/subtraction.asm`| subtraction + memory store                        |
| `examples/loop.asm`       | branching/labels, the Zero flag driving a loop   |
| `examples/fibonacci.asm`  | iterative loops, register copy, flags            |
| `examples/memory.asm`     | stores + stack round-trip (PUSH/POP)             |
| `examples/acceptance.asm` | the section-34 acceptance program                 |

## Project structure

```
include/          module interfaces (types, isa, registers, alu, memory,
                  decoder, cpu, assembler, debugger, execution_trace)
src/              implementations, grouped by module
tests/            one standalone test program per module + integration
examples/         sample assembly programs
docs/             architecture / isa / instruction cycle / memory / debugger
Makefile          GNU Make build (make / make test / make run / make clean)
build.ps1         PowerShell build for Windows without make
```

## Testing

Six standalone test programs cover each module plus full-program integration:

```
test_alu        test_memory      test_decoder
test_cpu        test_assembler   test_integration
```

Run with `make test` (or `.\build.ps1 test`).

## Acceptance test

The section-34 program (`examples/acceptance.asm`) is the canonical check:

```asm
START:
    LOAD A, 10
    LOAD B, 20
    ADD A, B
    STORE A, 50

    LOAD C, 5
    SUB A, C
    STORE A, 51

    HALT
```

Expected final state — and what the simulator produces:

```
A = 25        B = 20        C = 5
Memory[50] = 30     Memory[51] = 25     CPU = HALTED
```

## Documentation

- [docs/architecture.md](docs/architecture.md) — CPU architecture and components
- [docs/isa.md](docs/isa.md) — every instruction, encoding, flags
- [docs/instruction_cycle.md](docs/instruction_cycle.md) — fetch/decode/execute
- [docs/memory.md](docs/memory.md) — memory model and API
- [docs/debugger.md](docs/debugger.md) — step/run/breakpoints/trace

## Status & roadmap

The CPU engine, assembler, debugger, trace and tests are complete and the CLI
is fully functional. The next step is an optional graphical front end that
visualises `CPUState`, the ALU, and the `ExecutionTrace` — the architecture
already exposes everything the UI needs through `getState()`, `trace()`, and the
read-only `Memory::data()` view.
