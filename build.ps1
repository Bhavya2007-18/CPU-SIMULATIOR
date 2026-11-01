# CPU Simulator - PowerShell build script for Windows (when `make` is absent).
# Usage:
#   powershell -File build.ps1            # build the simulator (all)
#   powershell -File build.ps1 test        # build + run all unit tests
#   powershell -File build.ps1 run         # run the acceptance program
#   powershell -File build.ps1 clean       # remove build output

param([string]$Target = "all")

$CXX      = "g++"
$CXXFLAGS = @("-std=c++17", "-Wall", "-Wextra", "-Wpedantic", "-Iinclude")
$BUILD    = "build"

$LIB = @(
    "src/cpu/registers.cpp", "src/cpu/decoder.cpp", "src/cpu/cpu.cpp",
    "src/cpu/execution_trace.cpp", "src/alu/alu.cpp", "src/memory/memory.cpp",
    "src/assembler/assembler.cpp", "src/debugger/debugger.cpp"
)
$TESTS = @("test_alu", "test_memory", "test_decoder",
           "test_cpu", "test_assembler", "test_integration")

function Fail([string]$msg) { Write-Output "BUILD FAILED: $msg"; exit 1 }

if (-not (Test-Path $BUILD)) { New-Item -ItemType Directory -Path $BUILD | Out-Null }

switch ($Target) {
    "all" {
        $a = $CXXFLAGS + $LIB + @("src/main.cpp", "-o", "$BUILD/cpu-simulator.exe")
        & $CXX @a
        if ($LASTEXITCODE -ne 0) { Fail "main build" }
        Write-Output "Built $BUILD/cpu-simulator.exe"
    }
    "test" {
        # Build the main binary and every test binary.
        $a = $CXXFLAGS + $LIB + @("src/main.cpp", "-o", "$BUILD/cpu-simulator.exe")
        & $CXX @a; if ($LASTEXITCODE -ne 0) { Fail "main build" }
        foreach ($t in $TESTS) {
            $ta = $CXXFLAGS + $LIB + @("tests/$t.cpp", "-o", "$BUILD/$t.exe")
            & $CXX @ta
            if ($LASTEXITCODE -ne 0) { Fail "building $t" }
            Write-Output "built $t"
        }
        # Run the tests.
        $anyFail = 0
        foreach ($t in $TESTS) {
            Write-Output "=== $t ==="
            & "$BUILD/$t.exe"
            if ($LASTEXITCODE -ne 0) { $anyFail = 1 }
        }
        if ($anyFail -ne 0) { Write-Output "SOME TESTS FAILED"; exit 1 }
        Write-Output "ALL TESTS PASSED"
    }
    "run" {
        $a = $CXXFLAGS + $LIB + @("src/main.cpp", "-o", "$BUILD/cpu-simulator.exe")
        & $CXX @a; if ($LASTEXITCODE -ne 0) { Fail "main build" }
        & "$BUILD/cpu-simulator.exe" --run examples/acceptance.asm
    }
    "clean" {
        if (Test-Path $BUILD) { Remove-Item -Recurse -Force $BUILD }
        Write-Output "cleaned"
    }
    default { Write-Output "Unknown target: $Target. Use: all | test | run | clean"; exit 1 }
}
