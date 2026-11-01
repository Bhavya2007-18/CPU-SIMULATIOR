#ifndef CPU_SIM_TEST_FRAMEWORK_H
#define CPU_SIM_TEST_FRAMEWORK_H

#include <cstddef>
#include <iostream>
#include <string>

// Minimal, dependency-free test helpers. Each test_*.cpp is its own program
// with a main() that runs checks and returns report() (0 on success).

namespace cpusim::test {

inline std::size_t g_checks  = 0;
inline std::size_t g_failures = 0;

inline void check(bool cond, const char* expr, const char* file, int line) {
    ++g_checks;
    if (!cond) {
        ++g_failures;
        std::cout << "  FAIL " << file << ":" << line << "  (" << expr << ")\n";
    }
}

template <typename T>
inline void checkEq(T a, T b, const char* ea, const char* eb,
                    const char* file, int line) {
    ++g_checks;
    if (!(a == b)) {
        ++g_failures;
        std::cout << "  FAIL " << file << ":" << line << "  (" << ea << " == " << eb
                  << ")  got " << a << " vs " << b << "\n";
    }
}

inline int report() {
    std::size_t passed = g_checks - g_failures;
    std::cout << passed << "/" << g_checks << " checks passed";
    if (g_failures) std::cout << "  [" << g_failures << " FAILED]";
    std::cout << "\n";
    return g_failures == 0 ? 0 : 1;
}

} // namespace cpusim::test

#define CHECK(cond) ::cpusim::test::check((cond), #cond, __FILE__, __LINE__)
#define CHECK_EQ(a, b) \
    ::cpusim::test::checkEq<long long>( \
        static_cast<long long>(a), static_cast<long long>(b), \
        #a, #b, __FILE__, __LINE__)

#endif // CPU_SIM_TEST_FRAMEWORK_H
