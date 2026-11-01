#include "cpu.h"
#include "assembler.h"
#include "test_framework.h"

#include <string>

using namespace cpusim;
using namespace cpusim::test;

// Assemble, load, and run a program, returning the final CPU state. The same
// CPU is reused (loadProgram + reset reinitialise it).
static CPUState runProgram(const std::string& src, CPU& cpu) {
    Assembler as;
    auto r = as.assemble(src);
    cpu.memory().loadProgram(r.code);
    cpu.reset();
    cpu.run();
    return cpu.getState();
}

int main() {
    CPU cpu;

    // ---- Addition ----
    {
        auto s = runProgram(
            "LOAD A, 10\nLOAD B, 20\nADD A, B\nSTORE A, 50\nHALT\n", cpu);
        CHECK_EQ(s.A, 30);
        CHECK_EQ(s.B, 20);
        CHECK_EQ(cpu.memory().read(50), 30);
        CHECK(s.halted);
    }

    // ---- The section-34 acceptance program ----
    {
        auto s = runProgram(
            "START:\n"
            "    LOAD A, 10\n"
            "    LOAD B, 20\n"
            "    ADD A, B\n"
            "    STORE A, 50\n"
            "    LOAD C, 5\n"
            "    SUB A, C\n"
            "    STORE A, 51\n"
            "    HALT\n", cpu);
        CHECK_EQ(s.A, 25);
        CHECK_EQ(s.B, 20);
        CHECK_EQ(s.C, 5);
        CHECK_EQ(cpu.memory().read(50), 30);
        CHECK_EQ(cpu.memory().read(51), 25);
        CHECK(s.halted);
    }

    // ---- Fibonacci (F(11) = 89) ----
    {
        auto s = runProgram(
            "LOAD A, 0\n"
            "LOAD B, 1\n"
            "LOAD C, 10\n"
            "LOOP:\n"
            "    MOV D, B\n"
            "    ADD B, A\n"
            "    MOV A, D\n"
            "    DEC C\n"
            "    JNZ LOOP\n"
            "STORE B, 100\n"
            "HALT\n", cpu);
        CHECK_EQ(s.A, 55);
        CHECK_EQ(s.B, 89);
        CHECK_EQ(s.C, 0);
        CHECK_EQ(cpu.memory().read(100), 89);
        CHECK(s.halted);
    }

    // ---- Countdown loop ----
    {
        auto s = runProgram(
            "LOAD A, 5\nLOOP:\nDEC A\nJNZ LOOP\nHALT\n", cpu);
        CHECK_EQ(s.A, 0);
        CHECK(s.zero);
        CHECK(s.halted);
    }

    // ---- Subtraction + memory store ----
    {
        auto s = runProgram(
            "LOAD A, 50\nLOAD B, 20\nSUB A, B\nSTORE A, 60\nHALT\n", cpu);
        CHECK_EQ(s.A, 30);
        CHECK_EQ(cpu.memory().read(60), 30);
    }

    // ---- CALL / RET subroutine ----
    {
        auto s = runProgram(
            "LOAD A, 1\nCALL DOUBLE\nHALT\nDOUBLE:\nADD A, A\nRET\n", cpu);
        CHECK_EQ(s.A, 2);
        CHECK_EQ(s.SP, 0xFF);
        CHECK(s.halted);
    }

    // ---- Stack copy via PUSH/POP ----
    {
        auto s = runProgram("LOAD A, 42\nPUSH A\nPOP B\nHALT\n", cpu);
        CHECK_EQ(s.A, 42);
        CHECK_EQ(s.B, 42);
        CHECK_EQ(s.SP, 0xFF);
    }

    // ---- Logic + memory round-trip (memory.asm) ----
    {
        auto s = runProgram(
            "LOAD A, 5\nLOAD B, 10\nADD A, B\nSTORE A, 10\n"
            "LOAD C, 3\nSUB A, C\nSTORE A, 11\n"
            "PUSH A\nPOP D\nSTORE D, 12\nHALT\n", cpu);
        CHECK_EQ(s.A, 12);
        CHECK_EQ(s.D, 12);
        CHECK_EQ(cpu.memory().read(10), 15);
        CHECK_EQ(cpu.memory().read(11), 12);
        CHECK_EQ(cpu.memory().read(12), 12);
    }

    return report();
}
