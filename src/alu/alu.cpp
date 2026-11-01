#include "alu.h"

namespace cpusim {

namespace {
// Flags for bitwise operations: carry and overflow are cleared, zero/sign set.
ALUResult makeLogical(Byte value) noexcept {
    return ALUResult{
        value,
        value == 0,            // zero
        false,                 // carry    (n/a for logic)
        (value & 0x80) != 0,   // sign
        false                  // overflow (n/a for logic)
    };
}
} // namespace

ALUResult ALU::add(Byte a, Byte b) const noexcept {
    Word sum = static_cast<Word>(a) + static_cast<Word>(b);
    Byte result = static_cast<Byte>(sum);                  // low 8 bits
    bool carry = sum > 0xFF;                                // unsigned carry-out
    // Signed overflow: a,b same sign but result sign differs.
    bool overflow = ((~(a ^ b) & (a ^ result)) & 0x80) != 0;
    return ALUResult{result, result == 0, carry, (result & 0x80) != 0, overflow};
}

ALUResult ALU::sub(Byte a, Byte b) const noexcept {
    Word diff = static_cast<Word>(a) - static_cast<Word>(b); // wraps in 16 bits
    Byte result = static_cast<Byte>(diff);                   // low 8 bits = (a-b) mod 256
    bool carry = a < b;                                      // borrow
    // Signed overflow: a,b opposite sign and result sign differs from a.
    bool overflow = (((a ^ b) & (a ^ result)) & 0x80) != 0;
    return ALUResult{result, result == 0, carry, (result & 0x80) != 0, overflow};
}

ALUResult ALU::inc(Byte a) const noexcept {
    return add(a, 1);
}

ALUResult ALU::dec(Byte a) const noexcept {
    return sub(a, 1);
}

ALUResult ALU::bitwiseAnd(Byte a, Byte b) const noexcept {
    return makeLogical(static_cast<Byte>(a & b));
}

ALUResult ALU::bitwiseOr(Byte a, Byte b) const noexcept {
    return makeLogical(static_cast<Byte>(a | b));
}

ALUResult ALU::bitwiseXor(Byte a, Byte b) const noexcept {
    return makeLogical(static_cast<Byte>(a ^ b));
}

ALUResult ALU::bitwiseNot(Byte a) const noexcept {
    return makeLogical(static_cast<Byte>(~a));
}

} // namespace cpusim
