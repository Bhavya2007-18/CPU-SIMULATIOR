#ifndef CPU_SIM_INSTRUCTION_H
#define CPU_SIM_INSTRUCTION_H

#include "isa.h"

#include <cstddef>
#include <string>

namespace cpusim {

// ---------------------------------------------------------------------------
// Decoded instruction. Internal representation independent of assembly syntax.
//
// Only fields relevant to the opcode are meaningful; unused fields stay at
// their default (zero) values. `length` records how many machine-code bytes
// the instruction occupied, so the CPU can advance PC correctly.
// ---------------------------------------------------------------------------
struct Instruction {
    Opcode      opcode    = Opcode::NOP;
    Register    regDst    = Register::A;  // destination / first register operand
    Register    regSrc    = Register::A;  // source / second register operand
    Byte        immediate = 0;           // LOAD Rd, imm
    Address     address   = 0;           // STORE / JMP / JZ / JNZ / CALL addr
    std::size_t length    = 1;           // bytes consumed in machine code
};

// Render an instruction as assembly text, e.g. "ADD A,B", "LOAD A,10", "JMP 20".
// Implemented in decoder.cpp; kept here so the whole project shares one formatter.
std::string formatInstruction(const Instruction& ins);

} // namespace cpusim

#endif // CPU_SIM_INSTRUCTION_H
