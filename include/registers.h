#ifndef CPU_SIM_REGISTERS_H
#define CPU_SIM_REGISTERS_H

#include "isa.h"
#include "types.h"

#include <array>

namespace cpusim {

// ---------------------------------------------------------------------------
// Register file: the CPU's storage of general-purpose registers plus the
// special-purpose registers (PC, SP, IR).
//
// Access is through methods (read/write) rather than exposed references, so
// register mutation stays controlled and observable.
// ---------------------------------------------------------------------------
class RegisterFile {
public:
    void reset() noexcept;

    // General-purpose 8-bit registers (A, B, C, D).
    Byte read(Register r) const noexcept;
    void write(Register r, Byte value) noexcept;

    // Program Counter.
    Byte pc() const noexcept { return pc_; }
    void setPC(Byte value) noexcept { pc_ = value; }

    // Stack Pointer.
    Byte sp() const noexcept { return sp_; }
    void setSP(Byte value) noexcept { sp_ = value; }

    // Instruction Register (opcode byte of the executing instruction).
    Byte ir() const noexcept { return ir_; }
    void setIR(Byte value) noexcept { ir_ = value; }

private:
    std::array<Byte, REGISTER_COUNT> gp_{};  // A, B, C, D
    Byte pc_ = PROGRAM_ORIGIN;
    Byte sp_ = STACK_TOP;
    Byte ir_ = 0;
};

} // namespace cpusim

#endif // CPU_SIM_REGISTERS_H
