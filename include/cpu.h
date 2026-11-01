#ifndef CPU_SIM_CPU_H
#define CPU_SIM_CPU_H

#include "types.h"
#include "registers.h"
#include "flags.h"
#include "memory.h"
#include "alu.h"
#include "decoder.h"
#include "instruction.h"

#include <cstddef>
#include <memory>
#include <unordered_set>

namespace cpusim {

class ExecutionTrace;  // forward decl: owned via unique_ptr (see execution_trace.h)

// ---------------------------------------------------------------------------
// Immutable snapshot of the CPU. The debugger / UI use this instead of
// poking at private CPU internals.
// ---------------------------------------------------------------------------
struct CPUState {
    Byte A = 0, B = 0, C = 0, D = 0;   // general-purpose registers
    Byte PC = 0, IR = 0, SP = 0;       // special registers
    bool zero = false, carry = false, sign = false, overflow = false; // flags
    bool halted = false;
};

// ---------------------------------------------------------------------------
// The CPU: orchestrates the fetch -> decode -> execute cycle.
//
// Owns its registers, flags, memory, ALU and decoder. Arithmetic is always
// delegated to the ALU; the CPU never computes results directly.
// ---------------------------------------------------------------------------
class CPU {
public:
    CPU();
    ~CPU();

    CPU(const CPU&) = delete;
    CPU& operator=(const CPU&) = delete;

    // Reset CPU state (registers, flags, PC, halted, trace). The loaded
    // program in memory is preserved so a program can be re-run.
    void reset() noexcept;

    // Execute exactly one instruction. Throws CpuException on a fault.
    // Returns immediately (no-op) if the CPU is already halted.
    void step();

    // Repeatedly step() until HALT, a breakpoint PC, an error, or the step
    // limit. Returns the number of instructions executed.
    // If skipCurrentBreakpoint is true, the instruction at the current PC is
    // executed even if PC is a breakpoint (so a stopped run can resume).
    std::size_t run(bool skipCurrentBreakpoint = false,
                    std::size_t maxSteps = MAX_EXECUTION_STEPS);

    bool halted() const noexcept { return halted_; }

    // Breakpoints: run() stops BEFORE executing an instruction at a breakpoint.
    void setBreakpoint(Address addr);
    void clearBreakpoint(Address addr);
    void clearBreakpoints() noexcept;
    bool isBreakpoint(Address addr) const noexcept;

    // Inspection.
    CPUState getState() const noexcept;
    Memory& memory() noexcept { return mem_; }
    const Memory& memory() const noexcept { return mem_; }
    const Instruction& lastInstruction() const noexcept { return lastInstr_; }

    // Execution history.
    const ExecutionTrace& trace() const noexcept { return *trace_; }
    void clearTrace() noexcept;

private:
    Instruction fetchAndDecode();          // FETCH + DECODE, advances PC
    void execute(const Instruction& ins);  // EXECUTE / writeback
    void applyFlags(const ALUResult& r) noexcept;

    void pushByte(Byte value);             // stack push with overflow check
    Byte popByte();                        // stack pop with underflow check

    RegisterFile regs_;
    Flags flags_;
    Memory mem_;
    ALU alu_;
    Decoder decoder_;
    std::unique_ptr<ExecutionTrace> trace_;
    std::unordered_set<Address> breakpoints_;
    Instruction lastInstr_ = {};
    bool halted_ = false;
};

} // namespace cpusim

#endif // CPU_SIM_CPU_H
