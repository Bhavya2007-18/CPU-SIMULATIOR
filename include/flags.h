#ifndef CPU_SIM_FLAGS_H
#define CPU_SIM_FLAGS_H

#include "types.h"

namespace cpusim {

// ---------------------------------------------------------------------------
// CPU status flags.
//
// Updated by ALU operations and read by conditional jumps (JZ/JNZ).
// Naming matches the CPU state snapshot exposed to the debugger/UI.
// ---------------------------------------------------------------------------
struct Flags {
    bool zero      = false;  // Z: last result was 0
    bool carry     = false;  // C: unsigned carry-out (ADD) or borrow (SUB)
    bool sign      = false;  // S: bit 7 (sign bit) of last result
    bool overflow  = false;  // V: signed-arithmetic overflow

    void reset() noexcept {
        zero = carry = sign = overflow = false;
    }

    friend bool operator==(const Flags& a, const Flags& b) noexcept {
        return a.zero == b.zero && a.carry == b.carry &&
               a.sign == b.sign && a.overflow == b.overflow;
    }
    friend bool operator!=(const Flags& a, const Flags& b) noexcept {
        return !(a == b);
    }
};

} // namespace cpusim

#endif // CPU_SIM_FLAGS_H
