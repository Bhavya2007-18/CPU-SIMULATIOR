#include "cpu.h"
#include "debugger.h"
#include "decoder.h"
#include "instruction.h"
#include "types.h"

#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace cpusim {

namespace {

// Parse an address/value argument: decimal or 0x-hex.
std::optional<int> parseAddr(const std::string& tok) {
    if (tok.empty()) return std::nullopt;
    try {
        std::size_t pos = 0;
        int v = 0;
        if (tok.size() >= 2 && tok[0] == '0' && (tok[1] == 'x' || tok[1] == 'X')) {
            v = static_cast<int>(std::stoul(tok.substr(2), &pos, 16));
            if (pos != tok.size() - 2) return std::nullopt;
        } else {
            v = static_cast<int>(std::stoul(tok, &pos, 10));
            if (pos != tok.size()) return std::nullopt;
        }
        if (v < 0 || v > 255) return std::nullopt;
        return v;
    } catch (...) {
        return std::nullopt;
    }
}

std::vector<std::string> splitWords(const std::string& s) {
    std::vector<std::string> out;
    std::istringstream is(s);
    std::string w;
    while (is >> w) out.push_back(w);
    return out;
}

// Count instructions in the loaded program by walking memory with the decoder.
std::size_t countInstructions(const CPU& cpu, std::size_t programBytes) {
    Decoder dec;
    std::size_t count = 0;
    Address pc = PROGRAM_ORIGIN;
    std::size_t consumed = 0;
    while (consumed < programBytes) {
        Instruction ins = dec.decode(cpu.memory(), pc);
        consumed += ins.length;
        pc = static_cast<Address>(static_cast<std::size_t>(pc) + ins.length);
        ++count;
    }
    return count;
}

void printHelp() {
    std::cout <<
        "Commands:\n"
        "  step, s                 execute one instruction\n"
        "  run, r, continue, c     run until halt / breakpoint / error / limit\n"
        "  reset                   reset CPU (keep loaded program)\n"
        "  state                   full CPU state (registers + flags)\n"
        "  regs                    general-purpose registers\n"
        "  flags, f                status flags\n"
        "  pc                      program counter\n"
        "  ir                      current instruction\n"
        "  mem [start] [count]     memory dump (default start=0 count=64)\n"
        "  trace [n]               last n executed instructions (default 20)\n"
        "  break <addr>            set a breakpoint at address\n"
        "  unbreak <addr>          remove a breakpoint\n"
        "  breaks                  list breakpoints\n"
        "  help, ?                 this help\n"
        "  quit, q, exit           leave the debugger\n";
}

void printUsage() {
    std::cout <<
        "Usage: cpu-simulator [options] <file.asm>\n"
        "Options:\n"
        "  --run      assemble and run to completion, print final state, exit\n"
        "  --help     show help\n"
        "  (none)     start the interactive debugger\n";
}

void reportStop(const Debugger& dbg) {
    if (!dbg.lastError().empty()) {
        std::cout << "Error: " << dbg.lastError() << "\n";
    } else if (dbg.cpu().halted()) {
        std::cout << "CPU HALTED\n";
    } else if (dbg.isBreakpoint(dbg.cpu().getState().PC)) {
        std::cout << "Stopped at breakpoint (PC=" << static_cast<unsigned>(dbg.cpu().getState().PC) << ")\n";
    } else {
        std::cout << "Execution limit reached (possible infinite loop)\n";
    }
}

int interactive(Debugger& dbg) {
    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) { std::cout << "\n"; break; }
        auto words = splitWords(line);
        if (words.empty()) continue;
        const std::string& cmd = words[0];

        if (cmd == "quit" || cmd == "q" || cmd == "exit") break;
        if (cmd == "help" || cmd == "?") { printHelp(); continue; }

        if (cmd == "step" || cmd == "s") {
            if (!dbg.isLoaded()) { std::cout << "No program loaded.\n"; continue; }
            if (dbg.cpu().halted()) { std::cout << "CPU is halted.\n"; continue; }
            if (!dbg.step()) std::cout << "Step failed: " << dbg.lastError() << "\n";
            std::cout << dbg.stateReport();
            continue;
        }
        if (cmd == "run" || cmd == "r" || cmd == "continue" || cmd == "c") {
            if (!dbg.isLoaded()) { std::cout << "No program loaded.\n"; continue; }
            std::size_t n = dbg.run();
            std::cout << "Executed " << n << " instruction(s).\n";
            reportStop(dbg);
            std::cout << dbg.stateReport();
            continue;
        }
        if (cmd == "reset") {
            dbg.reset();
            std::cout << "CPU reset.\n" << dbg.stateReport();
            continue;
        }
        if (cmd == "state") { std::cout << dbg.stateReport(); continue; }
        if (cmd == "regs") {
            CPUState s = dbg.cpu().getState();
            std::cout << "A=" << static_cast<unsigned>(s.A)
                      << " B=" << static_cast<unsigned>(s.B)
                      << " C=" << static_cast<unsigned>(s.C)
                      << " D=" << static_cast<unsigned>(s.D) << "\n";
            continue;
        }
        if (cmd == "flags" || cmd == "f") { std::cout << dbg.flagsReport(); continue; }
        if (cmd == "pc") {
            std::cout << "PC=" << static_cast<unsigned>(dbg.cpu().getState().PC) << "\n"; continue;
        }
        if (cmd == "ir") {
            std::cout << "IR: " << formatInstruction(dbg.cpu().lastInstruction()) << "\n"; continue;
        }
        if (cmd == "mem") {
            int start = 0, count = 64;
            if (words.size() >= 2) { auto a = parseAddr(words[1]); if (a) start = *a; }
            if (words.size() >= 3) { auto c = parseAddr(words[2]); if (c) count = *c; }
            std::cout << dbg.memoryDump(static_cast<Address>(start), static_cast<std::size_t>(count));
            continue;
        }
        if (cmd == "trace") {
            std::size_t n = 20;
            if (words.size() >= 2) { auto c = parseAddr(words[1]); if (c) n = static_cast<std::size_t>(*c); }
            std::cout << dbg.traceDump(n);
            continue;
        }
        if (cmd == "break") {
            if (words.size() < 2) { std::cout << "Usage: break <addr>\n"; continue; }
            auto a = parseAddr(words[1]);
            if (!a) { std::cout << "Invalid address '" << words[1] << "'\n"; continue; }
            dbg.setBreakpoint(static_cast<Address>(*a));
            std::cout << "Breakpoint set at " << *a << "\n";
            continue;
        }
        if (cmd == "unbreak") {
            if (words.size() < 2) { std::cout << "Usage: unbreak <addr>\n"; continue; }
            auto a = parseAddr(words[1]);
            if (!a) { std::cout << "Invalid address '" << words[1] << "'\n"; continue; }
            dbg.clearBreakpoint(static_cast<Address>(*a));
            std::cout << "Breakpoint cleared at " << *a << "\n";
            continue;
        }
        if (cmd == "breaks") { std::cout << "Breakpoints: " << dbg.breakpointsReport(); continue; }

        std::cout << "Unknown command '" << cmd << "'. Type 'help'.\n";
    }
    return 0;
}

} // namespace
} // namespace cpusim

int main(int argc, char** argv) {
    using namespace cpusim;

    bool runMode = false;
    std::string file;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--help" || a == "-h") { printUsage(); return 0; }
        if (a == "--run") { runMode = true; continue; }
        if (!a.empty() && a[0] == '-') {
            std::cout << "Unknown option: " << a << "\n"; printUsage(); return 1;
        }
        file = a;
    }

    if (file.empty()) { printUsage(); return 1; }

    Debugger dbg;
    if (!dbg.loadFile(file)) {
        std::cout << "Failed to load program:\n" << dbg.lastError() << "\n";
        return 1;
    }

    std::size_t instrs = countInstructions(dbg.cpu(), dbg.programSize());
    std::cout << "Loaded " << dbg.programSize() << " bytes (" << instrs << " instruction(s)) from " << file << "\n";

    if (runMode) {
        std::size_t n = dbg.run();
        std::cout << "Executed " << n << " instruction(s).\n";
        reportStop(dbg);
        std::cout << dbg.stateReport();
        return dbg.cpu().halted() ? 0 : 2;
    }

    printHelp();
    return interactive(dbg);
}
