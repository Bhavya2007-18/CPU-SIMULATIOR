#include "registers.h"

namespace cpusim {

void RegisterFile::reset() noexcept {
    gp_.fill(0);
    pc_ = PROGRAM_ORIGIN;
    sp_ = STACK_TOP;
    ir_ = 0;
}

Byte RegisterFile::read(Register r) const noexcept {
    const auto idx = static_cast<Byte>(r);
    if (idx < REGISTER_COUNT) return gp_[idx];
    return 0;  // unreachable for a valid Register, guarded defensively
}

void RegisterFile::write(Register r, Byte value) noexcept {
    const auto idx = static_cast<Byte>(r);
    if (idx < REGISTER_COUNT) gp_[idx] = value;
}

} // namespace cpusim
