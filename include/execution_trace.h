#ifndef CPU_SIM_EXECUTION_TRACE_H
#define CPU_SIM_EXECUTION_TRACE_H

#include "cpu.h"          // for CPUState
#include "instruction.h"

#include <cstddef>
#include <vector>

namespace cpusim {

// One recorded step of execution. Captures everything the future UI needs to
// replay or animate a program.
struct TraceEntry {
    std::size_t cycle = 0;        // 1-based execution count
    Address     pcBefore = 0;     // PC before fetch
    Instruction instruction{};    // decoded instruction that ran
    CPUState    stateBefore{};    // full state before execute
    CPUState    stateAfter{};     // full state after execute
};

// Append-only history of executed instructions.
class ExecutionTrace {
public:
    void add(TraceEntry entry);
    void clear() noexcept { entries_.clear(); }
    std::size_t size() const noexcept { return entries_.size(); }
    const std::vector<TraceEntry>& entries() const noexcept { return entries_; }
    const TraceEntry& at(std::size_t i) const { return entries_.at(i); }

private:
    std::vector<TraceEntry> entries_;
};

} // namespace cpusim

#endif // CPU_SIM_EXECUTION_TRACE_H
