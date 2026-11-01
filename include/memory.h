#ifndef CPU_SIM_MEMORY_H
#define CPU_SIM_MEMORY_H

#include "types.h"

#include <array>
#include <vector>

namespace cpusim {

// ---------------------------------------------------------------------------
// Simulated RAM: 256 bytes addressed by an 8-bit Address.
//
// Because Address is uint8_t, every address value (0..255) is valid by
// construction; methods still centralize access so memory mutation is
// controlled and observable by the debugger/UI.
// ---------------------------------------------------------------------------
class Memory {
public:
    static constexpr std::size_t SIZE = MEMORY_SIZE;

    Memory() { reset(); }

    Byte read(Address address) const noexcept;
    void write(Address address, Byte value) noexcept;
    void reset() noexcept;

    // Load a program image starting at startAddr. Throws if it would run past
    // the end of memory (the Address space wraps; an over-long program is an
    // assembler/loader error, not silent truncation).
    void loadProgram(const std::vector<Byte>& program, Address startAddr = PROGRAM_ORIGIN);

    // Read-only view for inspection (debugger / UI). Do not mutate through it.
    const std::array<Byte, SIZE>& data() const noexcept { return mem_; }

private:
    std::array<Byte, SIZE> mem_{};
};

} // namespace cpusim

#endif // CPU_SIM_MEMORY_H
