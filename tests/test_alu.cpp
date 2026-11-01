#include "alu.h"
#include "test_framework.h"

using namespace cpusim;
using namespace cpusim::test;

int main() {
    ALU alu;

    // ---- ADD ----
    { auto r = alu.add(10, 20);
      CHECK_EQ(r.value, 30); CHECK(!r.carry); CHECK(!r.zero); CHECK(!r.sign); CHECK(!r.overflow); }

    { auto r = alu.add(255, 1);                 // wrap to 0
      CHECK_EQ(r.value, 0); CHECK(r.carry); CHECK(r.zero); CHECK(!r.sign); CHECK(!r.overflow); }

    { auto r = alu.add(100, 100);                // 200: signed overflow, sign set
      CHECK_EQ(r.value, 200); CHECK(!r.carry); CHECK(r.sign); CHECK(r.overflow); }

    { auto r = alu.add(200, 100);                // 300 -> 44, carry, sign clear (0x2C)
      CHECK_EQ(r.value, 44); CHECK(r.carry); CHECK(!r.sign); }

    // ---- SUB ----
    { auto r = alu.sub(50, 20);
      CHECK_EQ(r.value, 30); CHECK(!r.carry); CHECK(!r.zero); CHECK(!r.sign); CHECK(!r.overflow); }

    { auto r = alu.sub(10, 20);                  // 10-20 wraps to 246, borrow
      CHECK_EQ(r.value, 246); CHECK(r.carry); CHECK(!r.zero); CHECK(r.sign); }

    { auto r = alu.sub(5, 5);
      CHECK_EQ(r.value, 0); CHECK(r.zero); CHECK(!r.carry); }

    { auto r = alu.sub(0, 1);
      CHECK_EQ(r.value, 255); CHECK(r.carry); CHECK(!r.zero); CHECK(r.sign); }

    // ---- INC / DEC ----
    { auto r = alu.inc(5);  CHECK_EQ(r.value, 6);  CHECK(!r.zero); }
    { auto r = alu.inc(255); CHECK_EQ(r.value, 0);  CHECK(r.carry); CHECK(r.zero); }
    { auto r = alu.dec(5);  CHECK_EQ(r.value, 4);  CHECK(!r.zero); }
    { auto r = alu.dec(0);  CHECK_EQ(r.value, 255); CHECK(r.carry); CHECK(!r.zero); CHECK(r.sign); }

    // ---- Logic ----
    { auto r = alu.bitwiseAnd(0xF0, 0x0F); CHECK_EQ(r.value, 0);   CHECK(r.zero);  CHECK(!r.carry); }
    { auto r = alu.bitwiseOr(0xF0, 0x0F);  CHECK_EQ(r.value, 255); CHECK(r.sign); CHECK(!r.carry); CHECK(!r.zero); }
    { auto r = alu.bitwiseXor(0xFF, 0x0F); CHECK_EQ(r.value, 240); CHECK(r.sign); CHECK(!r.carry); }
    { auto r = alu.bitwiseNot(0x00);       CHECK_EQ(r.value, 255); CHECK(!r.zero); CHECK(r.sign); CHECK(!r.carry); }
    { auto r = alu.bitwiseNot(0xFF);       CHECK_EQ(r.value, 0);   CHECK(r.zero);  CHECK(!r.carry); }
    { auto r = alu.bitwiseAnd(0xFF, 0xFF); CHECK_EQ(r.value, 255); CHECK(!r.zero); }

    return report();
}
