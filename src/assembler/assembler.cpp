#include "assembler.h"
#include "isa.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace cpusim {

namespace {

using Symbols = std::unordered_map<std::string, Address>;

// ----- small text helpers --------------------------------------------------

std::string toUpper(std::string s) {
    for (char& c : s)
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

bool isIdentStart(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}
bool isIdentChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

bool isIdentifier(const std::string& s) {
    if (s.empty() || !isIdentStart(s[0])) return false;
    for (char c : s) if (!isIdentChar(c)) return false;
    return true;
}

std::string stripComment(const std::string& line) {
    auto pos = line.find(';');
    return pos == std::string::npos ? line : line.substr(0, pos);
}

// Split on whitespace (commas were already turned into spaces upstream).
std::vector<std::string> splitTokens(const std::string& s) {
    std::vector<std::string> out;
    std::istringstream is(s);
    std::string tok;
    while (is >> tok) out.push_back(tok);
    return out;
}

std::string errLine(std::size_t lineNo, const std::string& msg) {
    return "Line " + std::to_string(lineNo) + ": " + msg;
}

// Number of operand tokens expected for an opcode (excluding the mnemonic).
std::size_t operandCount(Opcode op) {
    switch (op) {
        case Opcode::NOP: case Opcode::HALT: case Opcode::RET:
            return 0;
        case Opcode::NOT: case Opcode::INC: case Opcode::DEC:
        case Opcode::PUSH: case Opcode::POP:
        case Opcode::JMP:  case Opcode::JZ:  case Opcode::JNZ: case Opcode::CALL:
            return 1;
        case Opcode::LOAD: case Opcode::STORE: case Opcode::MOV:
        case Opcode::ADD:  case Opcode::SUB:   case Opcode::AND:
        case Opcode::OR:   case Opcode::XOR:   case Opcode::CMP:
            return 2;
    }
    return 0;
}

// Parse a numeric literal (decimal, or 0x-prefixed hex) into a value in [0,255].
// Returns nullopt if the token is not numeric-looking (caller treats as a label).
// Throws AssemblyError if numeric but out of the 8-bit range.
std::optional<int> numericValue(const std::string& tok, std::size_t lineNo) {
    if (tok.empty()) return std::nullopt;
    const bool hex = tok.size() >= 3 && tok[0] == '0' && (tok[1] == 'x' || tok[1] == 'X');
    if (hex) {
        std::string h = tok.substr(2);
        for (char c : h)
            if (!std::isxdigit(static_cast<unsigned char>(c))) return std::nullopt; // not pure hex
        if (h.empty()) return std::nullopt;
        unsigned long v = std::stoul(h, nullptr, 16);
        if (v > 255) throw AssemblyError(errLine(lineNo, "value out of range (0-255): '" + tok + "'"));
        return static_cast<int>(v);
    }
    for (char c : tok)
        if (!std::isdigit(static_cast<unsigned char>(c))) return std::nullopt; // not a decimal
    unsigned long v = std::stoul(tok);  // may throw out_of_range
    if (v > 255) throw AssemblyError(errLine(lineNo, "value out of range (0-255): '" + tok + "'"));
    return static_cast<int>(v);
}

// Resolve an operand expected to be a register.
Register resolveRegister(const std::string& tok, std::size_t lineNo) {
    auto r = registerFromString(tok);
    if (!r) throw AssemblyError(errLine(lineNo, "unknown register '" + tok + "'"));
    return *r;
}

// Resolve an operand expected to be a byte value (immediate or address):
// a numeric literal, or a label reference.
Byte resolveValue(const std::string& tok, const Symbols& sym, std::size_t lineNo) {
    auto num = numericValue(tok, lineNo);            // throws if numeric-but-out-of-range
    if (num) return static_cast<Byte>(*num);
    auto it = sym.find(tok);
    if (it == sym.end())
        throw AssemblyError(errLine(lineNo, "unknown label '" + tok + "'"));
    return it->second;
}

// One parsed source line.
struct ParsedLine {
    std::size_t lineNo = 0;
    std::string label;                  // empty if none
    std::vector<std::string> tokens;    // mnemonic + operands (empty if label-only)
    Opcode opcode = Opcode::NOP;        // set in pass 1 for instruction lines
    bool hasInstruction = false;
};

// Turn a raw source line into a ParsedLine (comments stripped, commas -> spaces).
ParsedLine parseLine(const std::string& raw, std::size_t lineNo) {
    ParsedLine pl;
    pl.lineNo = lineNo;
    std::string body = stripComment(raw);
    // commas act as operand separators; normalise to spaces so splitTokens works.
    for (char& c : body) if (c == ',') c = ' ';

    auto tokens = splitTokens(body);
    if (tokens.empty()) return pl;  // blank / comment-only line

    // Optional leading label: a token ending in ':' with a valid identifier name.
    if (tokens.front().size() >= 2 && tokens.front().back() == ':') {
        std::string name = tokens.front().substr(0, tokens.front().size() - 1);
        if (isIdentifier(name)) {
            pl.label = name;
            tokens.erase(tokens.begin());
        }
    }
    if (!tokens.empty()) {
        pl.tokens = std::move(tokens);
        pl.hasInstruction = true;
    }
    return pl;
}

// Emit machine-code bytes for one instruction (pass 2).
std::vector<Byte> emitInstruction(const ParsedLine& pl, const Symbols& sym) {
    Opcode op = pl.opcode;
    const auto& ops = pl.tokens;
    const auto operands = std::vector<std::string>(ops.begin() + 1, ops.end());

    std::size_t expected = operandCount(op);
    if (operands.size() != expected) {
        throw AssemblyError(errLine(pl.lineNo, std::string(opcodeName(op)) + " expects " +
            std::to_string(expected) + " operand(s), got " +
            std::to_string(operands.size())));
    }

    std::vector<Byte> code;
    code.push_back(static_cast<Byte>(op));
    switch (op) {
        case Opcode::NOP: case Opcode::HALT: case Opcode::RET:
            break;
        case Opcode::NOT: case Opcode::INC: case Opcode::DEC:
        case Opcode::PUSH: case Opcode::POP:
            code.push_back(static_cast<Byte>(resolveRegister(operands[0], pl.lineNo)));
            break;
        case Opcode::JMP: case Opcode::JZ: case Opcode::JNZ: case Opcode::CALL:
            code.push_back(resolveValue(operands[0], sym, pl.lineNo));
            break;
        case Opcode::LOAD:
            code.push_back(static_cast<Byte>(resolveRegister(operands[0], pl.lineNo)));
            code.push_back(resolveValue(operands[1], sym, pl.lineNo));
            break;
        case Opcode::STORE:
            code.push_back(static_cast<Byte>(resolveRegister(operands[0], pl.lineNo)));
            code.push_back(resolveValue(operands[1], sym, pl.lineNo));
            break;
        case Opcode::MOV: case Opcode::ADD: case Opcode::SUB:
        case Opcode::AND: case Opcode::OR:  case Opcode::XOR: case Opcode::CMP:
            code.push_back(static_cast<Byte>(resolveRegister(operands[0], pl.lineNo)));
            code.push_back(static_cast<Byte>(resolveRegister(operands[1], pl.lineNo)));
            break;
    }
    return code;
}

} // namespace

// ---------------------------------------------------------------------------
// assemble: two passes over the source.
// ---------------------------------------------------------------------------
AssemblyResult Assembler::assemble(const std::string& source) const {
    // Split source into raw lines.
    std::vector<std::string> rawLines;
    {
        std::istringstream is(source);
        std::string line;
        while (std::getline(is, line)) rawLines.push_back(line);
    }

    // Pre-parse all lines.
    std::vector<ParsedLine> parsed;
    parsed.reserve(rawLines.size());
    for (std::size_t i = 0; i < rawLines.size(); ++i)
        parsed.push_back(parseLine(rawLines[i], i + 1));

    AssemblyResult result;
    Symbols& symbols = result.labels;

    // Pass 1: assign addresses, collect labels, validate mnemonics.
    Address addr = PROGRAM_ORIGIN;
    for (auto& pl : parsed) {
        if (!pl.label.empty()) {
            if (!isIdentifier(pl.label))
                throw AssemblyError(errLine(pl.lineNo, "invalid label '" + pl.label + "'"));
            if (symbols.count(pl.label))
                throw AssemblyError(errLine(pl.lineNo, "duplicate label '" + pl.label + "'"));
            symbols[pl.label] = addr;
        }
        if (pl.hasInstruction) {
            std::string mnemonic = toUpper(pl.tokens[0]);
            auto op = opcodeFromString(mnemonic);
            if (!op)
                throw AssemblyError(errLine(pl.lineNo, "unknown instruction '" + pl.tokens[0] + "'"));
            pl.opcode = *op;
            std::size_t len = instructionLength(*op);
            if (static_cast<std::size_t>(addr) + len > MEMORY_SIZE)
                throw AssemblyError(errLine(pl.lineNo, "program too large for 256-byte memory"));
            addr = static_cast<Address>(static_cast<std::size_t>(addr) + len);
        }
    }

    // Pass 2: emit machine code, resolving labels to addresses.
    std::vector<Byte>& code = result.code;
    for (const auto& pl : parsed) {
        if (!pl.hasInstruction) continue;
        auto bytes = emitInstruction(pl, symbols);
        code.insert(code.end(), bytes.begin(), bytes.end());
    }

    result.programSize = code.size();
    result.lineCount = parsed.size();
    return result;
}

} // namespace cpusim
