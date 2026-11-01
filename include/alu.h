#ifndef CPU_SIM_ALU_H
#define CPU_SIM_ALU_H

#include "types.h"

namespace cpusim {

// ---------------------------------------------------------------------------
// Result of an ALU operation: the computed byte plus all status flags.
// ---------------------------------------------------------------------------
struct ALUResult {
    Byte value;
    bool zero;
    bool carry;
    bool sign;
    bool overflow;
};

// ---------------------------------------------------------------------------
// Independent Arithmetic/Logic Unit.
//
// The CPU never performs arithmetic directly; it always routes operands
// through the ALU and then writes the returned value + flags back to state.
//
// Flag semantics (documented in docs/isa.md):
//   zero      result == 0
//   sign      bit 7 of result set
//   carry     ADD: unsigned carry-out (a+b > 255); SUB/DEC: borrow (a < b)
//   overflow  signed out-of-range, computed with the standard bit-7 formula
// ---------------------------------------------------------------------------
class ALU {
public:
    ALUResult add(Byte a, Byte b) const noexcept;
    ALUResult sub(Byte a, Byte b) const noexcept;
    ALUResult inc(Byte a) const noexcept;  // a + 1
    ALUResult dec(Byte a) const noexcept;  // a - 1

    ALUResult bitwiseAnd(Byte a, Byte b) const noexcept;
    ALUResult bitwiseOr(Byte a, Byte b) const noexcept;
    ALUResult bitwiseXor(Byte a, Byte b) const noexcept;
    ALUResult bitwiseNot(Byte a) const noexcept;  // ~a (single operand)
};

} // namespace cpusim

#endif // CPU_SIM_ALU_H
