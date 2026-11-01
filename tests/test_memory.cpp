#include "memory.h"
#include "test_framework.h"

#include <vector>

using namespace cpusim;
using namespace cpusim::test;

int main() {
    Memory m;

    // read / write
    m.write(10, 0xAB);
    CHECK_EQ(m.read(10), 0xAB);

    // boundaries (Address is uint8_t so 0 and 255 are the extremes)
    m.write(0, 0x01);
    CHECK_EQ(m.read(0), 0x01);
    m.write(255, 0x02);
    CHECK_EQ(m.read(255), 0x02);

    // reset clears everything
    m.reset();
    CHECK_EQ(m.read(10), 0);
    CHECK_EQ(m.read(0), 0);
    CHECK_EQ(m.read(255), 0);

    // loadProgram: writes bytes starting at an address and zeroes the rest
    std::vector<Byte> prog = {0x02, 0x00, 0x0A, 0x01};  // LOAD A,10 ; HALT
    m.loadProgram(prog, 0);
    CHECK_EQ(m.read(0), 0x02);
    CHECK_EQ(m.read(1), 0x00);
    CHECK_EQ(m.read(2), 0x0A);
    CHECK_EQ(m.read(3), 0x01);
    CHECK_EQ(m.read(4), 0);     // beyond program is zero

    // loadProgram at a non-zero start
    std::vector<Byte> small = {0xEE, 0xFF};
    m.loadProgram(small, 100);
    CHECK_EQ(m.read(100), 0xEE);
    CHECK_EQ(m.read(101), 0xFF);
    CHECK_EQ(m.read(0), 0);     // earlier region cleared by internal reset

    // a program too large for memory is rejected
    bool threw = false;
    try {
        std::vector<Byte> big(257, 0);
        m.loadProgram(big, 0);
    } catch (...) { threw = true; }
    CHECK(threw);

    // a program that would run past the end from a start address is rejected
    threw = false;
    try {
        std::vector<Byte> tail(4, 0);
        m.loadProgram(tail, 253);  // 253 + 4 = 257 > 256
    } catch (...) { threw = true; }
    CHECK(threw);

    return report();
}
