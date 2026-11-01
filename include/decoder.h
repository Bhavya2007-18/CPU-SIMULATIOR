#ifndef CPU_SIM_DECODER_H
#define CPU_SIM_DECODER_H

#include "memory.h"
#include "instruction.h"

#include <stdexcept>
#include <string>

namespace cpusim {

// Thrown for any CPU fault: invalid opcode, bad register code, malformed
// instruction, stack overflow/underflow, execution after HALT, etc.
// Carries a human-readable message suitable for the debugger / UI.
class CpuException : public std::runtime_error {
public:
    explicit CpuException(const std::string& msg) : std::runtime_error(msg) {}
};

// Converts machine-code bytes back into the internal Instruction form.
// Stateless, so a single instance can be shared by many CPUs.
class Decoder {
public:
    // Decode one instruction starting at pc in memory.
    // Throws CpuException on an invalid opcode or malformed operands.
    Instruction decode(const Memory& mem, Address pc) const;
};

} // namespace cpusim

#endif // CPU_SIM_DECODER_H
