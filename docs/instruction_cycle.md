# Instruction Cycle

Every call to `CPU::step()` runs exactly one instruction through the classic
machine cycle:

```
FETCH ──► DECODE ──► EXECUTE ──► WRITEBACK ──► FLAGS ──► NEXT
```

In this simulator `FETCH` and `DECODE` are combined in `CPU::fetchAndDecode()`,
and `EXECUTE` + `WRITEBACK` + `FLAGS` happen in `CPU::execute()`. `NEXT` is
simply the PC update done during fetch.

## 1. Fetch

1. Read the opcode byte at `PC` from memory.
2. If it is not a valid opcode, raise `CpuException("Invalid opcode ...")`.
3. Look up the instruction length from the opcode.
4. If the instruction's bytes would cross the end of memory
   (`PC + length > 256`), raise an error (no silent wraparound).

## 2. Decode

1. Read the operand byte(s) according to the opcode's fixed format.
2. Validate register codes (`0x00–0x03`); reject invalid ones.
3. Build an `Instruction { opcode, regDst, regSrc, immediate, address, length }`.

`IR` is set to the opcode byte — this is the value exposed in `CPUState.IR`.

## 3. Advance PC (the "next" step)

```
PC = PC + instruction.length       // 8-bit wraparound
```

This happens *before* execute so that for `CALL`, the current `PC` is already
the correct return address.

## 4. Execute / Writeback

Dispatched by opcode (see `isa.md`):

- Data movement: `LOAD`, `STORE`, `MOV`, `PUSH`, `POP`.
- Arithmetic/logic: route operands through the **ALU**, write the result back to
  the destination register, then apply the returned flags.
- Control flow: `JMP`, `JZ`, `JNZ` overwrite `PC`; `CALL`/`RET` use the stack.
- `HALT` sets the internal `halted_` flag.

## 5. Flags

`applyFlags(result)` copies the ALU's `zero/carry/sign/overflow` into the
`Flags` register. Conditional jumps later read `Flags.zero`.

## 6. Trace

After execute, `step()` records a `TraceEntry`:

```
{ cycle, pcBefore, instruction, stateBefore, stateAfter }
```

`stateBefore` is captured before fetch; `stateAfter` after execute — so the
trace shows the PC moving (e.g. `PC 15 -> 18` for a 3-byte instruction, or
`PC 5 -> 0` for a jump back to a label).

## `run()` vs `step()`

`run()` repeatedly calls `step()` until one of:

- `HALT` executed,
- a breakpoint is reached (checked *before* executing the instruction),
- a `CpuException` (which propagates to the caller), or
- `MAX_EXECUTION_STEPS` is reached (guards against infinite loops).

`run(skipCurrentBreakpoint = true)` skips the breakpoint check for the
instruction at the current PC, so a stopped run can resume past its breakpoint.
