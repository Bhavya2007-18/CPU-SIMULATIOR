#include "memory.h"

#include <stdexcept>
#include <string>

namespace cpusim {

Byte Memory::read(Address address) const noexcept {
    return mem_[address];
}

void Memory::write(Address address, Byte value) noexcept {
    mem_[address] = value;
}

void Memory::reset() noexcept {
    mem_.fill(0);
}

void Memory::loadProgram(const std::vector<Byte>& program, Address startAddr) {
    // Reject programs that would run past the end of the 256-byte space
    // rather than silently wrapping the address.
    if (program.size() > MEMORY_SIZE ||
        static_cast<std::size_t>(startAddr) + program.size() > MEMORY_SIZE) {
        throw std::runtime_error(
            "Program too large for memory starting at address " +
            std::to_string(static_cast<unsigned>(startAddr)));
    }
    reset();
    for (std::size_t i = 0; i < program.size(); ++i) {
        mem_[static_cast<Address>(startAddr + i)] = program[i];
    }
}

} // namespace cpusim
