#include "debugger.h"
#include "execution_trace.h"
#include "instruction.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace cpusim {

namespace {
std::string dec(Byte b) { return std::to_string(static_cast<unsigned>(b)); }
std::string hex2(Byte b) {
    std::ostringstream os;
    os << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
       << static_cast<unsigned>(b);
    return os.str();
}
} // namespace

Debugger::Debugger() = default;

bool Debugger::loadSource(const std::string& source) {
    Assembler as;
    try {
        AssemblyResult r = as.assemble(source);
        cpu_.memory().loadProgram(r.code, PROGRAM_ORIGIN);
        cpu_.reset();
        loaded_ = true;
        programSize_ = r.programSize;
        lastError_.clear();
        return true;
    } catch (const AssemblyError& e) {
        lastError_ = e.what();
        loaded_ = false;
        return false;
    } catch (const std::exception& e) {
        lastError_ = e.what();
        loaded_ = false;
        return false;
    }
}

bool Debugger::loadFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        lastError_ = "Cannot open file: " + path;
        loaded_ = false;
        return false;
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return loadSource(ss.str());
}

void Debugger::reset()      { cpu_.reset(); }

bool Debugger::step() {
    if (cpu_.halted()) return false;
    try {
        cpu_.step();
        return true;
    } catch (const CpuException& e) {
        lastError_ = e.what();
        return false;
    }
}

std::size_t Debugger::run() {
    lastError_.clear();
    try {
        // continue-semantics: skip a breakpoint at the current PC so a stopped
        // run can resume.
        return cpu_.run(true, MAX_EXECUTION_STEPS);
    } catch (const CpuException& e) {
        lastError_ = e.what();
        return 0;
    }
}

void Debugger::setBreakpoint(Address a)       { cpu_.setBreakpoint(a); }
void Debugger::clearBreakpoint(Address a)     { cpu_.clearBreakpoint(a); }
void Debugger::clearBreakpoints() noexcept    { cpu_.clearBreakpoints(); }
bool Debugger::isBreakpoint(Address a) const noexcept { return cpu_.isBreakpoint(a); }

std::string Debugger::stateReport() const {
    CPUState s = cpu_.getState();
    std::ostringstream os;
    os << "PC: " << dec(s.PC)
       << "   IR: " << formatInstruction(cpu_.lastInstruction())
       << "   SP: " << dec(s.SP)
       << "   " << (s.halted ? "[HALTED]" : "[running]") << "\n";
    os << "A: " << dec(s.A) << "   B: " << dec(s.B)
       << "   C: " << dec(s.C) << "   D: " << dec(s.D) << "\n";
    os << "Flags: Z=" << s.zero << " C=" << s.carry
       << " S=" << s.sign << " V=" << s.overflow << "\n";
    return os.str();
}

std::string Debugger::flagsReport() const {
    CPUState s = cpu_.getState();
    std::ostringstream os;
    os << "Z:" << (s.zero ? 1 : 0) << "  C:" << (s.carry ? 1 : 0)
       << "  S:" << (s.sign ? 1 : 0) << "  V:" << (s.overflow ? 1 : 0) << "\n";
    return os.str();
}

std::string Debugger::memoryDump(Address start, std::size_t count) const {
    if (static_cast<std::size_t>(start) >= MEMORY_SIZE) return "(start out of range)\n";
    count = std::min(count, MEMORY_SIZE - static_cast<std::size_t>(start));
    std::ostringstream os;
    for (std::size_t i = 0; i < count; ++i) {
        if (i % 16 == 0) {
            if (i) os << "\n";
            os << hex2(static_cast<Address>(static_cast<std::size_t>(start) + i)) << ": ";
        }
        os << hex2(cpu_.memory().read(
            static_cast<Address>(static_cast<std::size_t>(start) + i))) << " ";
    }
    os << "\n";
    return os.str();
}

std::string Debugger::traceDump(std::size_t lastN) const {
    const auto& t = cpu_.trace();
    if (t.size() == 0) return "(no instructions executed yet)\n";
    std::ostringstream os;
    std::size_t start = (lastN == 0 || lastN >= t.size()) ? 0 : t.size() - lastN;
    for (std::size_t i = start; i < t.size(); ++i) {
        const TraceEntry& e = t.at(i);
        const CPUState& b = e.stateBefore;
        const CPUState& a = e.stateAfter;
        os << "Cycle " << e.cycle
           << " | PC " << dec(e.pcBefore) << " -> " << dec(a.PC)
           << " | " << formatInstruction(e.instruction) << "\n";
        os << "  before: A=" << dec(b.A) << " B=" << dec(b.B)
           << " C=" << dec(b.C) << " D=" << dec(b.D)
           << " Z=" << b.zero << " C=" << b.carry
           << " S=" << b.sign << " V=" << b.overflow << "\n";
        os << "  after : A=" << dec(a.A) << " B=" << dec(a.B)
           << " C=" << dec(a.C) << " D=" << dec(a.D)
           << " Z=" << a.zero << " C=" << a.carry
           << " S=" << a.sign << " V=" << a.overflow << "\n";
    }
    return os.str();
}

std::string Debugger::breakpointsReport() const {
    // Reuse CPU's breakpoint set via repeated isBreakpoint queries (small, 256).
    std::ostringstream os;
    bool any = false;
    for (unsigned a = 0; a < MEMORY_SIZE; ++a) {
        if (cpu_.isBreakpoint(static_cast<Address>(a))) {
            os << dec(static_cast<Address>(a)) << " ";
            any = true;
        }
    }
    if (!any) os << "(none)";
    os << "\n";
    return os.str();
}

} // namespace cpusim
