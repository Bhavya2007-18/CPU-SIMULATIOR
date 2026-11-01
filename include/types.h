#ifndef CPU_SIM_TYPES_H
#define CPU_SIM_TYPES_H

#include <cstdint>
#include <cstddef>

namespace cpusim {

// ---------------------------------------------------------------------------
// Architecture-wide fixed-width type aliases.
//
// Using architecture-specific names (Byte/Address/Word) instead of raw
// uint8_t/int keeps CPU intent clear and centralizes the data widths.
// ---------------------------------------------------------------------------

using Byte   = uint8_t;   // 8-bit data word (registers, ALU operands, memory)
using Address = uint8_t;  // 8-bit memory address (0..255)
using Word   = uint16_t;  // 16-bit scratch value for carry-aware arithmetic

// ---------------------------------------------------------------------------
// Architecture constants. constexpr so they are usable in switch/context
// where a plain int would be a "magic number".
// ---------------------------------------------------------------------------

// MEMORY_SIZE is 256, which does NOT fit in an 8-bit Address (max 255), so it
// is a std::size_t. The 8-bit Address values are the smaller constants below.
constexpr std::size_t MEMORY_SIZE     = 256;   // 256 bytes of RAM
constexpr Address STACK_TOP         = 0xFF;    // SP starts here; stack grows down
constexpr Address PROGRAM_ORIGIN    = 0x00;    // programs load at address 0
constexpr std::size_t MAX_EXECUTION_STEPS = 100000; // run() safety limit

} // namespace cpusim

#endif // CPU_SIM_TYPES_H
