#ifndef CPU_SIM_DEBUGGER_H
#define CPU_SIM_DEBUGGER_H

#include "cpu.h"
#include "assembler.h"
#include "types.h"

#include <cstddef>
#include <string>

namespace cpusim {

// ---------------------------------------------------------------------------
// Lightweight debugger. Owns a CPU, provides high-level operations and renders
// human-readable reports. The CLI (main.cpp) drives it through these methods.
// ---------------------------------------------------------------------------
class Debugger {
public:
    Debugger();

    // Load an assembly program from a source string or a file path.
    // On success the program is assembled and loaded into CPU memory; on
    // failure returns false and lastError() holds the message.
    bool loadSource(const std::string& source);
    bool loadFile(const std::string& path);

    void reset();                 // reset CPU state, keep loaded program
    bool step();                  // one instruction; false if halted
    std::size_t run();            // continue until halt/breakpoint/error/limit

    // Breakpoints.
    void setBreakpoint(Address addr);
    void clearBreakpoint(Address addr);
    void clearBreakpoints() noexcept;
    bool isBreakpoint(Address addr) const noexcept;

    // Rendered reports.
    std::string stateReport() const;             // registers + flags + PC/IR/SP
    std::string flagsReport() const;             // Z C S V on one line
    std::string memoryDump(Address start, std::size_t count) const;
    std::string traceDump(std::size_t lastN) const;
    std::string breakpointsReport() const;

    const CPU& cpu() const noexcept { return cpu_; }
    bool isLoaded() const noexcept { return loaded_; }
    const std::string& lastError() const noexcept { return lastError_; }
    std::size_t programSize() const noexcept { return programSize_; }

private:
    CPU cpu_;
    bool loaded_ = false;
    std::size_t programSize_ = 0;
    std::string lastError_;
};

} // namespace cpusim

#endif // CPU_SIM_DEBUGGER_H
