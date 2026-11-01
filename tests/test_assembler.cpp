#include "assembler.h"
#include "test_framework.h"

#include <string>

using namespace cpusim;
using namespace cpusim::test;

int main() {
    Assembler as;

    // valid assembly with exact byte encoding
    {
        auto r = as.assemble("LOAD A, 10\nADD A, B\nHALT\n");
        // LOAD A,10 -> 02 00 0A ; ADD A,B -> 05 00 01 ; HALT -> 01
        CHECK_EQ(r.code.size(), 7u);
        CHECK_EQ(r.code[0], 0x02);
        CHECK_EQ(r.code[1], 0x00);
        CHECK_EQ(r.code[2], 0x0A);
        CHECK_EQ(r.code[3], 0x05);
        CHECK_EQ(r.code[4], 0x00);
        CHECK_EQ(r.code[5], 0x01);
        CHECK_EQ(r.code[6], 0x01);
    }

    // labels are resolved to addresses; comments ignored
    {
        auto r = as.assemble(
            "; a leading comment\n"
            "START:\n"
            "    LOAD A, 5\n"      // @0, len 3
            "    JMP START\n"      // @3, len 2 -> next 5
            "    HALT\n");         // @5
        CHECK(r.labels.count("START") == 1u);
        CHECK_EQ(r.labels["START"], 0);
        CHECK_EQ(r.code.size(), 6u);
        CHECK_EQ(r.code[3], 0x0E);    // JMP
        CHECK_EQ(r.code[4], 0);        // target = START = 0
        CHECK_EQ(r.code[5], 0x01);    // HALT
    }

    // a label that points at a later address (forward reference)
    {
        auto r = as.assemble("JMP SKIP\nSKIP:\nHALT\n");
        // JMP SKIP @0 len2 ->2 ; SKIP: HALT @2
        CHECK_EQ(r.labels["SKIP"], 2);
        CHECK_EQ(r.code[1], 2);        // JMP target
    }

    // hex literals (0x prefix)
    {
        auto r = as.assemble("LOAD A, 0x0F\nSTORE A, 0xFF\nHALT\n");
        CHECK_EQ(r.code[2], 0x0F);
        CHECK_EQ(r.code[5], 0xFF);
    }

    // case-insensitive mnemonics and registers
    {
        auto r = as.assemble("load a, 1\nAdd A, b\nhalt\n");
        CHECK_EQ(r.code[0], 0x02);
        CHECK_EQ(r.code[3], 0x05);
    }

    // invalid register -> line-numbered error
    {
        bool threw = false;
        std::string msg;
        try { (void)as.assemble("LOAD E, 10\nHALT\n"); }
        catch (const AssemblyError& e) { threw = true; msg = e.what(); }
        CHECK(threw);
        CHECK(msg.find("Line 1") != std::string::npos);
        CHECK(msg.find("register") != std::string::npos);
    }

    // unknown instruction -> error
    {
        bool threw = false;
        try { (void)as.assemble("FOO A, B\nHALT\n"); }
        catch (const AssemblyError&) { threw = true; }
        CHECK(threw);
    }

    // unknown label reference -> error
    {
        bool threw = false;
        std::string msg;
        try { (void)as.assemble("JMP NOWHERE\nHALT\n"); }
        catch (const AssemblyError& e) { threw = true; msg = e.what(); }
        CHECK(threw);
        CHECK(msg.find("unknown label") != std::string::npos);
    }

    // duplicate label -> error
    {
        bool threw = false;
        try { (void)as.assemble("L:\nLOAD A, 1\nL:\nHALT\n"); }
        catch (const AssemblyError&) { threw = true; }
        CHECK(threw);
    }

    // wrong operand count -> error
    {
        bool threw = false;
        try { (void)as.assemble("ADD A\nHALT\n"); }
        catch (const AssemblyError&) { threw = true; }
        CHECK(threw);
    }

    // out-of-range literal -> error
    {
        bool threw = false;
        try { (void)as.assemble("LOAD A, 300\nHALT\n"); }
        catch (const AssemblyError&) { threw = true; }
        CHECK(threw);
    }

    // blank/comment-only lines are fine
    {
        auto r = as.assemble("\n   ; just a comment\n\nHALT\n");
        CHECK_EQ(r.code.size(), 1u);
        CHECK_EQ(r.code[0], 0x01);
    }

    return report();
}
