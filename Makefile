# CPU Simulator - GNU Make build (Linux / macOS / MinGW-make).
# On Windows without make, use:  powershell -File build.ps1 <target>

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Iinclude
BUILD    := build
BIN      := $(BUILD)/cpu-simulator

LIB_SRCS := src/cpu/registers.cpp src/cpu/decoder.cpp src/cpu/cpu.cpp \
            src/cpu/execution_trace.cpp src/alu/alu.cpp src/memory/memory.cpp \
            src/assembler/assembler.cpp src/debugger/debugger.cpp
MAIN_SRC := src/main.cpp

TESTS := test_alu test_memory test_decoder test_cpu test_assembler test_integration

.PHONY: all test run clean

all: $(BIN)

$(BIN): $(MAIN_SRC) $(LIB_SRCS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(MAIN_SRC) $(LIB_SRCS) -o $(BIN)

$(BUILD):
	mkdir -p $(BUILD)

# Build each test executable, linking the shared library sources.
$(BUILD)/test_%: tests/test_%.cpp $(LIB_SRCS) | $(BUILD)
	$(CXX) $(CXXFLAGS) tests/test_$*.cpp $(LIB_SRCS) -o $@

test: $(addprefix $(BUILD)/,$(TESTS))
	@set -e; for t in $(TESTS); do \
	    echo "=== $$t ==="; ./$(BUILD)/$$t || exit 1; \
	done

run: $(BIN)
	./$(BIN) --run examples/acceptance.asm

clean:
	rm -rf $(BUILD)
