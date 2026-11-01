# Debugger

The CPU engine is fully usable from the command line. There are two entry
points:

- `cpu-simulator --run <file.asm>` — assemble, load, run to completion, print
  the final state, and exit.
- `cpu-simulator <file.asm>` — interactive debugger REPL.

## Interactive commands

| Command              | Description                                           |
|----------------------|--------------------------------------------------------|
| `step`, `s`          | execute exactly one instruction, then print state     |
| `run`, `r`, `continue`, `c` | run until halt / breakpoint / error / limit   |
| `reset`              | reset CPU state, keep the loaded program             |
| `state`              | full CPU state (registers + flags)                    |
| `regs`               | general-purpose registers                            |
| `flags`, `f`         | status flags (Z C S V)                                |
| `pc`                 | program counter                                       |
| `ir`                 | current (last) instruction                            |
| `mem [start] [count]`| memory dump (default start=0, count=64)             |
| `trace [n]`          | last `n` executed instructions (default 20)           |
| `break <addr>`       | set a breakpoint at address `addr`                    |
| `unbreak <addr>`     | remove a breakpoint                                   |
| `breaks`             | list breakpoints                                      |
| `help`, `?`          | command help                                          |
| `quit`, `q`, `exit`  | leave the debugger                                    |

Addresses accept decimal or `0x`-hex (e.g. `break 0x0A`).

## Breakpoints

`run` stops *before* executing an instruction whose `PC` is a breakpoint. A
stopped run resumes past the current breakpoint (so typing `run` again actually
makes progress) — this is `run(skipCurrentBreakpoint = true)` internally.

## Execution trace

Every executed instruction is recorded as a `TraceEntry`:

```
Cycle 6 | PC 15 -> 18 | SUB A, C
  before: A=30 B=20 C=5 D=0 Z=0 C=0 S=0 V=0
  after : A=25 B=20 C=5 D=0 Z=0 C=0 S=0 V=0
```

It captures the cycle number, the PC before/after, the decoded instruction, and
the full register/flag state before and after. The trace is what the future
graphical UI will animate.

## Example session

```
$ ./cpu-simulator examples/acceptance.asm
Loaded 22 bytes (8 instruction(s)) from examples/acceptance.asm
> step
PC: 3   IR: LOAD A, 10   SP: 255   [running]
A: 10   B: 0   C: 0   D: 0
Flags: Z=0 C=0 S=0 V=0
> break 9
Breakpoint set at 9
> run
Executed 1 instruction(s).
Stopped at breakpoint (PC=9)
PC: 9   IR: ADD A, B   SP: 255   [running]
A: 30   B: 20   C: 0   D: 0
> run
Executed 5 instruction(s).
CPU HALTED
> trace 2
Cycle 7 | PC 18 -> 21 | STORE A, 51
  ...
> mem 50 4
32: 1E 19 00 00
> quit
```
