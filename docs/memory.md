# Memory

A flat 256-byte address space, byte-addressable with an 8-bit address
(`Address = uint8_t`, range `0x00–0xFF`).

## Layout

```
0x00 ┌─────────────────────────────┐
     │  Program code               │   loaded by loadProgram() at address 0
     │  ...                        │
     ├─────────────────────────────┤
     │  (free / data)              │   used by STORE for variables
     │  ...                        │
0xFF │  Stack grows down from here │   SP starts at 0xFF, pre-decrement push
     └─────────────────────────────┘
```

Code is loaded at the program origin (`0x00`). There is no hardware protection
between code, data and stack — they share the single 256-byte space. In
practice the stack (growing down from `0xFF`) and short programs (starting at
`0x00`) never collide. Deep recursion could overwrite code; that is a normal
property of such a tiny machine and is documented, not "fixed".

## API (`memory.h`)

```cpp
Byte  read(Address address) const;
void  write(Address address, Byte value);
void  reset();
void  loadProgram(const std::vector<Byte>& program, Address startAddr = 0x00);
const std::array<Byte, 256>& data() const;   // read-only inspection
```

- `read`/`write` take an `Address`, so an out-of-range address cannot exist —
  validation is structural.
- `reset()` zeroes all 256 bytes.
- `loadProgram` clears memory, then copies the program in starting at
  `startAddr`. If the program would run past byte 256 it throws (it is never
  silently wrapped).
- `data()` exposes a read-only view for the debugger / future UI; it is not
  meant to be mutated through.

## Reading values back

`LOAD` is an *immediate* load and `STORE` writes a register *to* memory. There
is no base-ISA instruction that loads a register *from* an arbitrary memory
address. A memory round-trip is still possible through the stack:

```asm
PUSH A      ; push a value
POP  B      ; pop it into another register  -> B = A
```

Memory contents produced by `STORE` are verified by inspecting memory through
the debugger (`mem` command) or `Memory::read`, not by re-loading them.
