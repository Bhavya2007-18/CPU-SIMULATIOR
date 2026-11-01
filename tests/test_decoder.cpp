#include "decoder.h"
#include "memory.h"
#include "test_framework.h"

using namespace cpusim;
using namespace cpusim::test;

int main() {
    Decoder dec;
    Memory m;

    // LOAD A,10  | ADD A,B | JMP 20 | HALT | NOT C | PUSH D
    // 02 00 0A   | 05 00 01| 0E 14  | 01    | 0A 02 | 11 03
    m.write(0, 0x02); m.write(1, 0x00); m.write(2, 0x0A);
    m.write(3, 0x05); m.write(4, 0x00); m.write(5, 0x01);
    m.write(6, 0x0E); m.write(7, 0x14);
    m.write(8, 0x01);
    m.write(9, 0x0A); m.write(10, 0x02);
    m.write(11, 0x11); m.write(12, 0x03);

    { auto i = dec.decode(m, 0);
      CHECK(i.opcode == Opcode::LOAD);
      CHECK(i.regDst == Register::A);
      CHECK_EQ(i.immediate, 10);
      CHECK_EQ(i.length, 3); }

    { auto i = dec.decode(m, 3);
      CHECK(i.opcode == Opcode::ADD);
      CHECK(i.regDst == Register::A);
      CHECK(i.regSrc == Register::B);
      CHECK_EQ(i.length, 3); }

    { auto i = dec.decode(m, 6);
      CHECK(i.opcode == Opcode::JMP);
      CHECK_EQ(i.address, 20);
      CHECK_EQ(i.length, 2); }

    { auto i = dec.decode(m, 8);
      CHECK(i.opcode == Opcode::HALT);
      CHECK_EQ(i.length, 1); }

    { auto i = dec.decode(m, 9);
      CHECK(i.opcode == Opcode::NOT);
      CHECK(i.regDst == Register::C);
      CHECK_EQ(i.length, 2); }

    { auto i = dec.decode(m, 11);
      CHECK(i.opcode == Opcode::PUSH);
      CHECK(i.regDst == Register::D);
      CHECK_EQ(i.length, 2); }

    // invalid opcode (0x55 is beyond RET = 0x14)
    m.write(20, 0x55);
    { bool threw = false;
      try { (void)dec.decode(m, 20); } catch (const CpuException&) { threw = true; }
      CHECK(threw); }

    // invalid register code for a unary register op (NOT with code 0x09)
    m.write(21, 0x0A); m.write(22, 0x09);
    { bool threw = false;
      try { (void)dec.decode(m, 21); } catch (const CpuException&) { threw = true; }
      CHECK(threw); }

    // invalid register code for a binary op (ADD with bad second register)
    m.write(23, 0x05); m.write(24, 0x00); m.write(25, 0x09);
    { bool threw = false;
      try { (void)dec.decode(m, 23); } catch (const CpuException&) { threw = true; }
      CHECK(threw); }

    // instruction crossing end of memory (3-byte at PC 254)
    m.write(254, 0x02); m.write(255, 0x00); m.write(0, 0x0A);
    { bool threw = false;
      try { (void)dec.decode(m, 254); } catch (const CpuException&) { threw = true; }
      CHECK(threw); }

    return report();
}
