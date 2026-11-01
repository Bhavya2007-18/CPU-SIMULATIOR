#include "decoder.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace cpusim {

namespace {
std::string toHex2(Byte b) {
    std::ostringstream os;
    os << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
       << static_cast<unsigned>(b);
    return os.str();
}

std::string toDec(Byte b) {
    return std::to_string(static_cast<unsigned>(b));
}

// Build a malformed-instruction error with context.
std::string badRegister(Opcode op, Byte code, Address pc) {
    return "Invalid register code 0x" + toHex2(code) + " for " +
           opcodeName(op) + " at PC=" + toDec(pc);
}
} // namespace

Instruction Decoder::decode(const Memory& mem, Address pc) const {
    Byte opcodeByte = mem.read(pc);
    if (!isValidOpcode(opcodeByte)) {
        throw CpuException("Invalid opcode 0x" + toHex2(opcodeByte) +
                            " at PC=" + toDec(pc));
    }
    Opcode op = static_cast<Opcode>(opcodeByte);
    std::size_t len = instructionLength(op);

    // Reject instructions whose operand bytes would wrap past the end of memory.
    if (static_cast<std::size_t>(pc) + len > MEMORY_SIZE) {
        throw CpuException(std::string(opcodeName(op)) + " at PC=" + toDec(pc) +
                            " crosses end of memory");
    }

    Instruction ins;
    ins.opcode = op;
    ins.length = len;

    if (len == 2) {
        Byte op1 = mem.read(static_cast<Address>(pc + 1));
        switch (op) {
            case Opcode::NOT: case Opcode::INC: case Opcode::DEC:
            case Opcode::PUSH: case Opcode::POP:
                if (!isValidRegister(op1)) throw CpuException(badRegister(op, op1, pc));
                ins.regDst = toRegister(op1);
                break;
            case Opcode::JMP: case Opcode::JZ: case Opcode::JNZ: case Opcode::CALL:
                ins.address = op1;
                break;
            default:
                break;
        }
    } else if (len == 3) {
        Byte op1 = mem.read(static_cast<Address>(pc + 1));
        Byte op2 = mem.read(static_cast<Address>(pc + 2));
        switch (op) {
            case Opcode::LOAD:   // LOAD Rd, imm
                if (!isValidRegister(op1)) throw CpuException(badRegister(op, op1, pc));
                ins.regDst = toRegister(op1);
                ins.immediate = op2;
                break;
            case Opcode::STORE:  // STORE Rd, addr
                if (!isValidRegister(op1)) throw CpuException(badRegister(op, op1, pc));
                ins.regDst = toRegister(op1);
                ins.address = op2;
                break;
            case Opcode::MOV: case Opcode::ADD: case Opcode::SUB:
            case Opcode::AND: case Opcode::OR:  case Opcode::XOR:
            case Opcode::CMP:    // Rd, Rs
                if (!isValidRegister(op1)) throw CpuException(badRegister(op, op1, pc));
                if (!isValidRegister(op2)) throw CpuException(badRegister(op, op2, pc));
                ins.regDst = toRegister(op1);
                ins.regSrc = toRegister(op2);
                break;
            default:
                break;
        }
    }
    return ins;
}

// ---------------------------------------------------------------------------
// formatInstruction: one canonical textual form for a decoded instruction,
// shared by the debugger, trace dump, and the future UI.
// ---------------------------------------------------------------------------
std::string formatInstruction(const Instruction& ins) {
    switch (ins.opcode) {
        case Opcode::NOP: case Opcode::HALT: case Opcode::RET:
            return opcodeName(ins.opcode);
        case Opcode::NOT: case Opcode::INC: case Opcode::DEC:
        case Opcode::PUSH: case Opcode::POP:
            return std::string(opcodeName(ins.opcode)) + " " +
                   registerName(ins.regDst);
        case Opcode::LOAD:
            return "LOAD " + std::string(registerName(ins.regDst)) + ", " +
                   toDec(ins.immediate);
        case Opcode::STORE:
            return "STORE " + std::string(registerName(ins.regDst)) + ", " +
                   toDec(ins.address);
        case Opcode::MOV: case Opcode::ADD: case Opcode::SUB:
        case Opcode::AND: case Opcode::OR:  case Opcode::XOR:
        case Opcode::CMP:
            return std::string(opcodeName(ins.opcode)) + " " +
                   registerName(ins.regDst) + ", " + registerName(ins.regSrc);
        case Opcode::JMP: case Opcode::JZ: case Opcode::JNZ: case Opcode::CALL:
            return std::string(opcodeName(ins.opcode)) + " " + toDec(ins.address);
    }
    return "???";
}

} // namespace cpusim
