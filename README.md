# CPU SIMULATOR

A software-only **8-bit CPU simulator** built from scratch in C++17 — ISA,
registers, ALU, memory, instruction decoder, assembler, fetch-decode-execute
engine, debugger, execution trace, unit tests, and a connected browser IDE.
A **Digital Logic & Computer Design (DLCD) PBL** project.

> *“We designed an 8-bit CPU architecture, defined our own instruction set,
> created an assembler, simulated memory and registers, implemented an ALU, and
> built a fetch-decode-execute engine capable of executing our machine code.”*

You can use it three ways:

1. **CLI** — assemble + run a `.asm` file, or drive an interactive debugger.
2. **Web UI** — a browser IDE wired to the real C++ engine over a local HTTP API.
3. **HTTP API** — drive the engine from scripts (JSON endpoints).

---

## Quick start (30 seconds)

```powershell
# Windows (this repo was built/tested here — MSYS2 g++ is the toolchain)
.\build.ps1            # build everything (simulator + server)
.\build.ps1 run        # run the acceptance program, print final state
.\build.ps1 serve      # start the web UI at http://localhost:8080
```

On Linux/macOS (or Windows with GNU Make):

```bash
make            # build the simulator + server
make run         # run the acceptance program
make serve      # start the web UI at http://localhost:8080
make test        # build + run all unit tests
make clean
```

Then open **`http://localhost:8080`** in a browser, pick an example, and click
**Assemble & Load → Step/Run**.

---

## Requirements

- A **C++17** compiler: `g++`, `clang++`, or MSVC.
- The web server uses raw sockets: on Windows it links `ws2_32` (MSYS2 `g++`
  provides this); on Linux/macOS sockets are in the libc, nothing extra needed.
- No external libraries. No Emscripten unless you later want the WebAssembly path.

The project compiles cleanly under `-Wall -Wextra -Wpedantic` (and `-Werror`).

---

## 1. CLI

```bash
# Run a program to completion and print the final CPU state:
./build/cpu-simulator --run examples/acceptance.asm

# Interactive debugger REPL:
./build/cpu-simulator examples/acceptance.asm

./build/cpu-simulator --help
```

Interactive commands:

| Command              | What it does                                       |
|----------------------|----------------------------------------------------|
| `step`, `s`          | execute exactly one instruction, then print state |
| `run`, `r`, `continue`, `c` | run until halt / breakpoint / error / limit |
| `reset`              | reset CPU state, keep the loaded program          |
| `state`              | full CPU state (registers + flags)                |
| `regs`               | general-purpose registers                          |
| `flags`, `f`         | status flags (Z C S V)                             |
| `pc`                 | program counter                                    |
| `ir`                 | current (last) instruction                         |
| `mem [start] [count]`| memory dump (default start=0, count=64)           |
| `trace [n]`          | last `n` executed instructions (default 20)       |
| `break <addr>`       | set a breakpoint                                   |
| `unbreak <addr>`     | remove a breakpoint                                |
| `breaks`             | list breakpoints                                  |
| `help`, `?`          | command help                                       |
| `quit`, `q`, `exit`  | leave the debugger                                 |

Addresses accept decimal or `0x`-hex (e.g. `break 0x06`). See
[`docs/debugger.md`](docs/debugger.md).

---

## 2. Web UI

A tiny, dependency-free C++ HTTP server (`build/cpu-server`) wraps the existing
`Debugger`/`CPU`/`Assembler`, serves the static frontend, and exposes a JSON API.
Each browser gets its **own private CPU session** (cookie-keyed), so multiple
people can use it at once without clobbering each other; idle sessions expire
after 30 minutes. No logic is duplicated in JavaScript — every value shown comes
from the real engine.

```bash
make serve                                    # or:  .\build.ps1 serve
./build/cpu-server --port 8080 --web frontend # run the server binary directly
```

Then open `http://localhost:8080`. You can:

- edit assembly (or open an example), then **Assemble & Load**
- **Step** one instruction, or **Run** at a clock speed (1 / 5 / 20 Hz, or MAX)
- **Pause** an animated run, **Reset**
- toggle **breakpoints** from the listing gutter
- watch registers (hex / dec / binary), control registers (PC / IR / SP), flags,
  a 256-byte memory grid, and the live **execution trace**

The design follows the provided `frontend/*` mockups and the
“deterministic instrumentation” design system (zinc-charcoal palette, JetBrains
Mono data, matte / no-glow aesthetic).

---

## 3. HTTP API (for scripts / automation)

The server endpoints (all return JSON; `assemble` takes a `text/plain` body):

| Method | Endpoint                          | Purpose                          |
|--------|-----------------------------------|----------------------------------|
| POST   | `/api/assemble`                   | assemble source (body = source)  |
| POST   | `/api/step`                       | execute one instruction          |
| POST   | `/api/run`                        | run to halt/breakpoint/error/limit |
| POST   | `/api/reset`                      | reset the CPU                    |
| GET    | `/api/state`                      | current registers/flags/PC/IR/SP |
| GET    | `/api/memory?start=&count=`       | memory bytes                     |
| GET    | `/api/trace?count=`               | execution history                |
| POST   | `/api/break?addr=N`               | set a breakpoint                 |
| POST   | `/api/unbreak?addr=N`            | clear a breakpoint               |
| GET    | `/api/breakpoints`                | list breakpoints                |
| GET    | `/api/examples`                   | list example `.asm` files        |
| GET    | `/api/example?name=addition.asm`  | fetch an example's source        |

Example (PowerShell):

```powershell
$body = @'
LOAD A, 10
ADD A, A
HALT
'@
Invoke-RestMethod -Method POST -Uri "http://localhost:8080/api/assemble" -Body $body -ContentType "text/plain"
Invoke-RestMethod -Method POST -Uri "http://localhost:8080/api/run"
Invoke-RestMethod -Uri "http://localhost:8080/api/state"
```

---

## Architecture

- **8-bit** data, **8-bit** addresses, **256 bytes** of RAM.
- Registers: `A B C D` (general purpose), `PC`, `IR`, `SP`.
- Flags: `Z` (zero), `C` (carry/borrow), `S` (sign), `V` (overflow).
- `step()` = **one** instruction: Fetch → Decode → Execute → Writeback → Flags.

```
Assembly ─► Assembler ─► Machine code ─► Memory ─► CPU
                                                      │
   ┌──────────────────────────────────────────────────┘
   ▼
 Fetch ─► Decode ─► Execute ─► ALU / Registers / Memory ─► CPU State ─► Debugger
```

## Instruction set (21 instructions)

```
NOP  HALT
LOAD  STORE  MOV
ADD  SUB  AND  OR  XOR  NOT  INC  DEC  CMP
JMP  JZ  JNZ
PUSH  POP  CALL  RET
```

Instruction length is a pure function of the opcode — 1, 2, or 3 bytes:

| Length | Instructions                                   |
|--------|------------------------------------------------|
| 1 byte | `NOP`, `HALT`, `RET`                           |
| 2 bytes| `NOT INC DEC`, `JMP JZ JNZ`, `PUSH POP`, `CALL`|
| 3 bytes| `LOAD STORE MOV`, `ADD SUB AND OR XOR CMP`     |

Example encoding: `ADD A, B` → `05 00 01` (opcode `0x05`, dst `A=0x00`, src `B=0x01`).
The full encoding table and flag rules are in [`docs/isa.md`](docs/isa.md).

## Writing your own program

Assembly syntax:

- **Mnemonics and registers are case-insensitive** (`add A, b` is fine).
- **Comments** start with `;` and run to the end of the line.
- **Labels** are `NAME:` on their own line or before an instruction.
- **Literals**: decimal (`10`, `255`) or hex with the `0x` prefix (`0x0A`).
- **Operands** are separated by commas. Registers are `A B C D`.

```asm
; Countdown: print 5..1, stop
        LOAD A, 5
LOOP:
        DEC A        ; A = A - 1, sets Z when A hits 0
        JNZ LOOP     ; loop while A != 0
        HALT
```

Operands by instruction:

```
LOAD Rd, imm     STORE Rd, addr   MOV Rd, Rs
ADD/SUB/AND/OR/XOR/CMP Rd, Rs     NOT/INC/DEC/PUSH/POP Rd
JMP/JZ/JNZ/CALL addr              PUSH/POP Rd   NOP / HALT / RET (no operands)
```

> `LOAD` is an *immediate* load; `STORE` writes a register *to* memory. There is
> no base-ISA memory→register load — a memory round-trip is done via the stack
> (`PUSH A` / `POP B`). See [`docs/memory.md`](docs/memory.md).

## Examples

| File                      | Demonstrates                                    |
|---------------------------|--------------------------------------------------|
| `examples/addition.asm`   | arithmetic + memory store                        |
| `examples/subtraction.asm`| subtraction + memory store                      |
| `examples/loop.asm`       | branching/labels, Zero flag driving a loop       |
| `examples/fibonacci.asm`  | iterative loops, register copy, flags           |
| `examples/memory.asm`     | stores + stack round-trip (PUSH/POP)             |
| `examples/acceptance.asm` | the section-34 acceptance program                |

## Testing

Six standalone test programs cover each module plus full-program integration:

```
test_alu        test_memory      test_decoder
test_cpu        test_assembler   test_integration
```

Run them with `make test` (or `.\build.ps1 test`) — 201 checks total.

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

## Project structure

```
include/    module interfaces (types, isa, instruction, registers, flags,
            alu, memory, decoder, cpu, assembler, debugger, execution_trace, server)
src/        implementations, grouped by module:
              cpu/  alu/  memory/  assembler/  debugger/  server/  main.cpp
tests/      one standalone test program per module + integration
examples/   sample assembly programs
frontend/    web UI (index.html, app.js, app.css) + provided design mockups
docs/       architecture / isa / instruction cycle / memory / debugger
Makefile    GNU Make build (make / make test / make run / make serve / make clean)
build.ps1   PowerShell build for Windows (all / test / run / serve / static / deploy / clean)
```

## Deployment (always-on, on Windows)

Build a portable server exe (statically linked — no MSYS2 runtime needed on the
target machine):

```powershell
.\build.ps1 deploy      # -> build/cpu-server-portable.exe
```

Copy `cpu-server-portable.exe` + the `frontend/` folder to the host, then run it
as a Windows Service so it survives reboots and crashes (using **NSSM**):

```powershell
nssm install CPU-Simulator "C:\cpu-sim\cpu-server-portable.exe" "--port 8080 --web C:\cpu-sim\frontend"
nssm set CPU-Simulator AppDirectory C:\cpu-sim
nssm set CPU-Simulator AppRestartDelay 5000
nssm set CPU-Simulator Start SERVICE_AUTO_START
nssm start CPU-Simulator
```

Expose it publicly with HTTPS (no router port-forwarding) using **Cloudflare
Tunnel**: `cloudflared tunnel create cpu-sim`, point it at `http://localhost:8080`,
then `cloudflared service install`. Both the app service and the tunnel service
auto-start on boot and restart on crash → the site stays up. Because the server
is multi-user, every visitor gets an isolated CPU.

## Documentation

- [docs/architecture.md](docs/architecture.md) — CPU architecture and components
- [docs/isa.md](docs/isa.md) — every instruction, encoding, flags
- [docs/instruction_cycle.md](docs/instruction_cycle.md) — fetch/decode/execute
- [docs/memory.md](docs/memory.md) — memory model and API
- [docs/debugger.md](docs/debugger.md) — step/run/breakpoints/trace

## Status & roadmap

The CPU engine, assembler, debugger, execution trace, tests, CLI, web UI, and
multi-user server are complete and verified. The browser UI drives the real
engine over the local HTTP API.

Possible next steps: the secondary “architecture/datapath” and “ISA reference”
views from the mockups, drag-to-resize panes, a full multi-file editor, a thread
pool for higher concurrency, and the WebAssembly build (run the engine in each
browser tab → pure static hosting, zero ops).
