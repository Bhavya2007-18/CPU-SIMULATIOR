#include "cpu.h"
#include "execution_trace.h"

#include <utility>  // std::move

namespace cpusim {

CPU::CPU() : trace_(std::make_unique<ExecutionTrace>()) {
    reset();
}

// Destructor defined here where ExecutionTrace is a complete type.
CPU::~CPU() = default;

void CPU::reset() noexcept {
    regs_.reset();
    flags_.reset();
    lastInstr_ = {};
    halted_ = false;
    trace_->clear();
    // Memory and breakpoints are intentionally preserved across reset so a
    // loaded program can be re-run with the same breakpoints.
}

CPUState CPU::getState() const noexcept {
    CPUState s;
    s.A = regs_.read(Register::A);
    s.B = regs_.read(Register::B);
    s.C = regs_.read(Register::C);
    s.D = regs_.read(Register::D);
    s.PC = regs_.pc();
    s.IR = regs_.ir();
    s.SP = regs_.sp();
    s.zero = flags_.zero;
    s.carry = flags_.carry;
    s.sign = flags_.sign;
    s.overflow = flags_.overflow;
    s.halted = halted_;
    return s;
}

// ---- Breakpoints ----------------------------------------------------------
void CPU::setBreakpoint(Address addr)            { breakpoints_.insert(addr); }
void CPU::clearBreakpoint(Address addr)          { breakpoints_.erase(addr); }
void CPU::clearBreakpoints() noexcept            { breakpoints_.clear(); }
bool CPU::isBreakpoint(Address addr) const noexcept {
    return breakpoints_.find(addr) != breakpoints_.end();
}

void CPU::clearTrace() noexcept { trace_->clear(); }

// ---- Fetch + Decode -------------------------------------------------------
Instruction CPU::fetchAndDecode() {
    Address pc = regs_.pc();
    Instruction ins = decoder_.decode(mem_, pc);   // throws on bad opcode/operands
    regs_.setIR(static_cast<Byte>(ins.opcode));    // IR holds the opcode byte
    // Advance PC by the instruction's byte length (8-bit wraparound).
    regs_.setPC(static_cast<Address>(static_cast<std::size_t>(pc) + ins.length));
    return ins;
}

void CPU::applyFlags(const ALUResult& r) noexcept {
    flags_.zero = r.zero;
    flags_.carry = r.carry;
    flags_.sign = r.sign;
    flags_.overflow = r.overflow;
}

// ---- Stack helpers (pre-decrement push, post-increment pop) ----------------
void CPU::pushByte(Byte value) {
    Byte sp = regs_.sp();
    if (sp == 0x00) throw CpuException("Stack overflow (SP=0x00)");
    sp -= 1;                       // pre-decrement
    mem_.write(sp, value);
    regs_.setSP(sp);
}

Byte CPU::popByte() {
    Byte sp = regs_.sp();
    if (sp == STACK_TOP) throw CpuException("Stack underflow (SP=0xFF, stack empty)");
    Byte value = mem_.read(sp);
    sp += 1;                        // post-increment
    regs_.setSP(sp);
    return value;
}

// ---- Execute --------------------------------------------------------------
void CPU::execute(const Instruction& ins) {
    switch (ins.opcode) {
        case Opcode::NOP:
            break;

        case Opcode::HALT:
            halted_ = true;
            break;

        case Opcode::LOAD:                      // Rd = imm
            regs_.write(ins.regDst, ins.immediate);
            break;

        case Opcode::STORE:                      // mem[addr] = Rd
            mem_.write(ins.address, regs_.read(ins.regDst));
            break;

        case Opcode::MOV:                        // Rd = Rs
            regs_.write(ins.regDst, regs_.read(ins.regSrc));
            break;

        case Opcode::ADD: {
            ALUResult r = alu_.add(regs_.read(ins.regDst), regs_.read(ins.regSrc));
            regs_.write(ins.regDst, r.value); applyFlags(r);
            break;
        }
        case Opcode::SUB: {
            ALUResult r = alu_.sub(regs_.read(ins.regDst), regs_.read(ins.regSrc));
            regs_.write(ins.regDst, r.value); applyFlags(r);
            break;
        }
        case Opcode::AND: {
            ALUResult r = alu_.bitwiseAnd(regs_.read(ins.regDst), regs_.read(ins.regSrc));
            regs_.write(ins.regDst, r.value); applyFlags(r);
            break;
        }
        case Opcode::OR: {
            ALUResult r = alu_.bitwiseOr(regs_.read(ins.regDst), regs_.read(ins.regSrc));
            regs_.write(ins.regDst, r.value); applyFlags(r);
            break;
        }
        case Opcode::XOR: {
            ALUResult r = alu_.bitwiseXor(regs_.read(ins.regDst), regs_.read(ins.regSrc));
            regs_.write(ins.regDst, r.value); applyFlags(r);
            break;
        }
        case Opcode::NOT: {
            ALUResult r = alu_.bitwiseNot(regs_.read(ins.regDst));
            regs_.write(ins.regDst, r.value); applyFlags(r);
            break;
        }
        case Opcode::INC: {
            ALUResult r = alu_.inc(regs_.read(ins.regDst));
            regs_.write(ins.regDst, r.value); applyFlags(r);
            break;
        }
        case Opcode::DEC: {
            ALUResult r = alu_.dec(regs_.read(ins.regDst));
            regs_.write(ins.regDst, r.value); applyFlags(r);
            break;
        }
        case Opcode::CMP: {                       // set flags from (Rd - Rs)
            ALUResult r = alu_.sub(regs_.read(ins.regDst), regs_.read(ins.regSrc));
            applyFlags(r);
            break;
        }

        case Opcode::JMP:                        // PC = addr
            regs_.setPC(ins.address);
            break;
        case Opcode::JZ:                         // if Z then PC = addr
            if (flags_.zero) regs_.setPC(ins.address);
            break;
        case Opcode::JNZ:                         // if !Z then PC = addr
            if (!flags_.zero) regs_.setPC(ins.address);
            break;

        case Opcode::PUSH:                        // mem[--SP] = Rd
            pushByte(regs_.read(ins.regDst));
            break;
        case Opcode::POP:                         // Rd = mem[SP++]
            regs_.write(ins.regDst, popByte());
            break;

        case Opcode::CALL: {                      // push return, PC = addr
            // fetchAndDecode already advanced PC past the 2-byte CALL, so the
            // current PC is the correct return address.
            pushByte(regs_.pc());
            regs_.setPC(ins.address);
            break;
        }
        case Opcode::RET: {                       // PC = pop()
            regs_.setPC(popByte());
            break;
        }
    } // end switch (all opcodes handled; no default needed)
}

// ---- Step / Run -----------------------------------------------------------
void CPU::step() {
    if (halted_) return;                // executing after HALT is a silent no-op

    CPUState stateBefore = getState();
    Address pcBefore = regs_.pc();

    Instruction ins = fetchAndDecode();  // FETCH + DECODE, advances PC
    lastInstr_ = ins;
    execute(ins);                        // EXECUTE + writeback + flags

    TraceEntry entry{};
    entry.cycle       = trace_->size() + 1;  // 1-based, assigned before add
    entry.pcBefore    = pcBefore;
    entry.instruction = ins;
    entry.stateBefore = stateBefore;
    entry.stateAfter  = getState();
    trace_->add(std::move(entry));
}

std::size_t CPU::run(bool skipCurrentBreakpoint, std::size_t maxSteps) {
    std::size_t executed = 0;
    bool skipOnce = skipCurrentBreakpoint;
    while (executed < maxSteps) {
        if (halted_) break;
        if (!skipOnce && isBreakpoint(regs_.pc())) {
            break;                       // stop BEFORE executing a breakpoint
        }
        skipOnce = false;
        step();                           // may throw CpuException (propagates)
        ++executed;
    }
    return executed;
}

} // namespace cpusim
