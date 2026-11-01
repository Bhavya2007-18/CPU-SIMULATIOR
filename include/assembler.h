#ifndef CPU_SIM_ASSEMBLER_H
#define CPU_SIM_ASSEMBLER_H

#include "types.h"

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace cpusim {

// Thrown on any assembly error. The message includes the offending line, e.g.
//   "Line 3: Unknown register 'E'"
class AssemblyError : public std::runtime_error {
public:
    explicit AssemblyError(const std::string& msg) : std::runtime_error(msg) {}
};

// Result of assembling a source program.
struct AssemblyResult {
    std::vector<Byte> code;                              // machine code bytes
    std::unordered_map<std::string, Address> labels;     // resolved label -> address
    std::size_t programSize = 0;                         // bytes emitted
    std::size_t lineCount = 0;                           // source lines processed
};

// ---------------------------------------------------------------------------
// Two-pass assembler.
//
// Pass 1: assign addresses to every instruction and label (needs instruction
//         sizes, which are fixed by the ISA).
// Pass 2: emit machine code, resolving label references to addresses.
//
// Supports decimal (`10`) and hex (`0x0A`, `0FFh`) literals, comments (`;`),
// labels (`NAME:`), and the full ISA. Produces clear, line-numbered errors.
// ---------------------------------------------------------------------------
class Assembler {
public:
    AssemblyResult assemble(const std::string& source) const;
};

} // namespace cpusim

#endif // CPU_SIM_ASSEMBLER_H
