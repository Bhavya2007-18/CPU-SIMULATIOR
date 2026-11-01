#ifndef CPU_SIM_ISA_H
#define CPU_SIM_ISA_H

#include "types.h"

#include <optional>
#include <string_view>

namespace cpusim {

// ---------------------------------------------------------------------------
// Instruction set opcodes. The numeric value IS the machine-code opcode byte.
// ---------------------------------------------------------------------------
enum class Opcode : Byte {
    NOP   = 0x00,  // NOP
    HALT  = 0x01,  // HALT
    LOAD  = 0x02,  // LOAD  Rd, imm    -> Rd = imm
    STORE = 0x03,  // STORE Rd, addr   -> mem[addr] = Rd
    MOV   = 0x04,  // MOV   Rd, Rs     -> Rd = Rs
    ADD   = 0x05,  // ADD   Rd, Rs     -> Rd = Rd + Rs
    SUB   = 0x06,  // SUB   Rd, Rs     -> Rd = Rd - Rs
    AND   = 0x07,  // AND   Rd, Rs     -> Rd = Rd & Rs
    OR    = 0x08,  // OR    Rd, Rs     -> Rd = Rd | Rs
    XOR   = 0x09,  // XOR   Rd, Rs     -> Rd = Rd ^ Rs
    NOT   = 0x0A,  // NOT   Rd         -> Rd = ~Rd
    INC   = 0x0B,  // INC   Rd         -> Rd = Rd + 1
    DEC   = 0x0C,  // DEC   Rd         -> Rd = Rd - 1
    CMP   = 0x0D,  // CMP   Rd, Rs     -> set flags from (Rd - Rs)
    JMP   = 0x0E,  // JMP   addr       -> PC = addr
    JZ    = 0x0F,  // JZ    addr       -> if Z  then PC = addr
    JNZ   = 0x10,  // JNZ   addr       -> if !Z then PC = addr
    PUSH  = 0x11,  // PUSH  Rd         -> mem[--SP] = Rd
    POP   = 0x12,  // POP   Rd         -> Rd = mem[SP++]
    CALL  = 0x13,  // CALL  addr       -> push (PC+2); PC = addr
    RET   = 0x14,  // RET             -> PC = pop()
};

// General-purpose registers. The numeric value IS the register code byte.
enum class Register : Byte {
    A = 0x00,
    B = 0x01,
    C = 0x02,
    D = 0x03,
};

constexpr Byte REGISTER_COUNT = 4;  // A, B, C, D general-purpose registers

// Number of bytes an instruction occupies in machine code.
// Defined here so the assembler, decoder and CPU agree on one source of truth.
constexpr std::size_t instructionLength(Opcode op) noexcept {
    switch (op) {
        case Opcode::NOP: case Opcode::HALT: case Opcode::RET:
            return 1;
        case Opcode::NOT: case Opcode::INC: case Opcode::DEC:
        case Opcode::JMP: case Opcode::JZ:  case Opcode::JNZ:
        case Opcode::PUSH: case Opcode::POP: case Opcode::CALL:
            return 2;
        case Opcode::LOAD: case Opcode::STORE: case Opcode::MOV:
        case Opcode::ADD:  case Opcode::SUB:   case Opcode::AND:
        case Opcode::OR:   case Opcode::XOR:   case Opcode::CMP:
            return 3;
    }
    return 0; // not a valid opcode
}

constexpr bool isValidOpcode(Byte b) noexcept {
    return b <= static_cast<Byte>(Opcode::RET);
}

constexpr bool isValidRegister(Byte b) noexcept {
    return b < REGISTER_COUNT;
}

constexpr Register toRegister(Byte b) noexcept {
    return static_cast<Register>(b);
}

// ---------------------------------------------------------------------------
// Name <-> string conversions. Shared by the decoder, assembler, debugger and
// the future UI so the mnemonic spelling is defined in exactly one place.
// ---------------------------------------------------------------------------

inline const char* opcodeName(Opcode op) {
    switch (op) {
        case Opcode::NOP:   return "NOP";
        case Opcode::HALT:  return "HALT";
        case Opcode::LOAD:  return "LOAD";
        case Opcode::STORE: return "STORE";
        case Opcode::MOV:   return "MOV";
        case Opcode::ADD:   return "ADD";
        case Opcode::SUB:   return "SUB";
        case Opcode::AND:   return "AND";
        case Opcode::OR:    return "OR";
        case Opcode::XOR:   return "XOR";
        case Opcode::NOT:   return "NOT";
        case Opcode::INC:   return "INC";
        case Opcode::DEC:   return "DEC";
        case Opcode::CMP:   return "CMP";
        case Opcode::JMP:   return "JMP";
        case Opcode::JZ:    return "JZ";
        case Opcode::JNZ:   return "JNZ";
        case Opcode::PUSH:  return "PUSH";
        case Opcode::POP:   return "POP";
        case Opcode::CALL:  return "CALL";
        case Opcode::RET:   return "RET";
    }
    return "???";
}

inline std::optional<Opcode> opcodeFromString(std::string_view s) {
    if (s == "NOP")   return Opcode::NOP;
    if (s == "HALT")  return Opcode::HALT;
    if (s == "LOAD")  return Opcode::LOAD;
    if (s == "STORE") return Opcode::STORE;
    if (s == "MOV")   return Opcode::MOV;
    if (s == "ADD")   return Opcode::ADD;
    if (s == "SUB")   return Opcode::SUB;
    if (s == "AND")   return Opcode::AND;
    if (s == "OR")    return Opcode::OR;
    if (s == "XOR")   return Opcode::XOR;
    if (s == "NOT")   return Opcode::NOT;
    if (s == "INC")   return Opcode::INC;
    if (s == "DEC")   return Opcode::DEC;
    if (s == "CMP")   return Opcode::CMP;
    if (s == "JMP")   return Opcode::JMP;
    if (s == "JZ")    return Opcode::JZ;
    if (s == "JNZ")   return Opcode::JNZ;
    if (s == "PUSH")  return Opcode::PUSH;
    if (s == "POP")   return Opcode::POP;
    if (s == "CALL")  return Opcode::CALL;
    if (s == "RET")   return Opcode::RET;
    return std::nullopt;
}

inline const char* registerName(Register r) {
    switch (r) {
        case Register::A: return "A";
        case Register::B: return "B";
        case Register::C: return "C";
        case Register::D: return "D";
    }
    return "?";
}

inline std::optional<Register> registerFromString(std::string_view s) {
    if (s.size() != 1) return std::nullopt;
    switch (s[0]) {
        case 'A': case 'a': return Register::A;
        case 'B': case 'b': return Register::B;
        case 'C': case 'c': return Register::C;
        case 'D': case 'd': return Register::D;
        default: return std::nullopt;
    }
}

} // namespace cpusim

#endif // CPU_SIM_ISA_H
