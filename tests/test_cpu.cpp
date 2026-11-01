#include "cpu.h"
#include "execution_trace.h"
#include "assembler.h"
#include "test_framework.h"

#include <string>

using namespace cpusim;
using namespace cpusim::test;

static CPUState runOk(const std::string& src, CPU& cpu) {
    Assembler as;
    auto r = as.assemble(src);
    cpu.memory().loadProgram(r.code);
    cpu.reset();
    cpu.run();
    return cpu.getState();
}

int main() {
    Assembler as;

    // reset clears state but keeps the loaded program
    {
        CPU cpu;
        auto r = as.assemble("LOAD A, 9\nHALT\n");
        cpu.memory().loadProgram(r.code);
        cpu.run();
        CHECK_EQ(cpu.getState().A, 9);
        cpu.reset();
        CHECK_EQ(cpu.getState().A, 0);
        CHECK_EQ(cpu.getState().PC, 0);
        CHECK(!cpu.halted());
    }

    // step executes exactly one instruction
    {
        CPU cpu;
        auto r = as.assemble("LOAD A, 10\nHALT\n");
        cpu.memory().loadProgram(r.code);
        cpu.step();
        CPUState s = cpu.getState();
        CHECK_EQ(s.A, 10);
        CHECK_EQ(s.PC, 3);     // LOAD is 3 bytes
        CHECK(!s.halted);
        cpu.step();            // executes HALT
        CHECK(cpu.halted());
        cpu.step();            // step after halt is a no-op
        CHECK(cpu.halted());
    }

    // ADD carry + zero flags (255 + 1 wraps to 0)
    {
        CPU cpu;
        auto s = runOk("LOAD A, 255\nLOAD B, 1\nADD A, B\nHALT\n", cpu);
        CHECK_EQ(s.A, 0);
        CHECK(s.carry);
        CHECK(s.zero);
    }

    // SUB borrow (5 - 9 wraps to 252)
    {
        CPU cpu;
        auto s = runOk("LOAD A, 5\nLOAD B, 9\nSUB A, B\nHALT\n", cpu);
        CHECK_EQ(s.A, 252);
        CHECK(s.carry);
    }

    // CMP sets flags but does not change the register
    {
        CPU cpu;
        auto s = runOk("LOAD A, 7\nLOAD B, 7\nCMP A, B\nHALT\n", cpu);
        CHECK_EQ(s.A, 7);
        CHECK(s.zero);
        CHECK(!s.carry);
    }

    // JZ taken when Z is set (skips the LOAD B)
    {
        CPU cpu;
        auto s = runOk(
            "LOAD A, 1\n"
            "DEC A\n"          // A=0 -> Z=1
            "JZ END\n"
            "LOAD B, 9\n"      // skipped
            "END:\n"
            "HALT\n", cpu);
        CHECK_EQ(s.B, 0);
        CHECK(s.halted);
    }

    // JZ not taken when Z is clear (falls through to LOAD B)
    {
        CPU cpu;
        auto s = runOk(
            "LOAD A, 2\n"
            "DEC A\n"          // A=1 -> Z=0
            "JZ END\n"
            "LOAD B, 9\n"
            "END:\n"
            "HALT\n", cpu);
        CHECK_EQ(s.B, 9);
    }

    // loop with JNZ
    {
        CPU cpu;
        auto s = runOk(
            "LOAD A, 3\n"
            "LOOP:\n"
            "DEC A\n"
            "JNZ LOOP\n"
            "HALT\n", cpu);
        CHECK_EQ(s.A, 0);
        CHECK(s.zero);
    }

    // CALL / RET
    {
        CPU cpu;
        auto s = runOk(
            "LOAD A, 1\n"
            "CALL DOUBLE\n"
            "HALT\n"
            "DOUBLE:\n"
            "ADD A, A\n"
            "RET\n", cpu);
        CHECK_EQ(s.A, 2);
        CHECK_EQ(s.SP, 0xFF);    // stack balanced
        CHECK(s.halted);
    }

    // PUSH / POP (copy via the stack)
    {
        CPU cpu;
        auto s = runOk("LOAD A, 42\nPUSH A\nPOP B\nHALT\n", cpu);
        CHECK_EQ(s.A, 42);
        CHECK_EQ(s.B, 42);
        CHECK_EQ(s.SP, 0xFF);
    }

    // stack underflow (POP from empty stack) raises
    {
        CPU cpu;
        auto r = as.assemble("POP A\nHALT\n");
        cpu.memory().loadProgram(r.code);
        cpu.reset();
        bool threw = false;
        try { cpu.run(); } catch (const CpuException&) { threw = true; }
        CHECK(threw);
    }

    // infinite loop is bounded by the step limit
    {
        CPU cpu;
        auto r = as.assemble("LOOP:\nJMP LOOP\n");
        cpu.memory().loadProgram(r.code);
        cpu.reset();
        std::size_t n = cpu.run(false, 100);
        CHECK_EQ(n, 100);
        CHECK(!cpu.halted());
    }

    // breakpoints stop run() before executing the breakpoint instruction
    {
        CPU cpu;
        auto r = as.assemble("LOAD A, 1\nLOAD B, 2\nHALT\n");
        cpu.memory().loadProgram(r.code);
        cpu.reset();
        // LOAD A,1 @0 (len3) -> PC3 ; LOAD B,2 @3 (len3) -> PC6 ; HALT @6
        cpu.setBreakpoint(3);          // stop before LOAD B
        std::size_t n = cpu.run(false, 1000);
        CHECK_EQ(n, 1);
        CHECK(!cpu.halted());
        CHECK_EQ(cpu.getState().PC, 3);
        CHECK_EQ(cpu.getState().A, 1);
    }

    // trace records one entry per executed instruction
    {
        CPU cpu;
        auto r = as.assemble("LOAD A, 1\nLOAD B, 2\nHALT\n");
        cpu.memory().loadProgram(r.code);
        cpu.reset();
        cpu.run();
        CHECK_EQ(cpu.trace().size(), 3u);
        CHECK_EQ(cpu.trace().at(0).instruction.opcode, Opcode::LOAD);
    }

    return report();
}
