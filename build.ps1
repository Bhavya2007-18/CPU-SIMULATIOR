# CPU Simulator - PowerShell build script for Windows (when `make` is absent).
# Usage:
#   .\build.ps1            build the simulator + server (all)
#   .\build.ps1 test       build + run all unit tests
#   .\build.ps1 run        run the acceptance program (CLI)
#   .\build.ps1 serve      build + start the web frontend server (http://localhost:8080)
#   .\build.ps1 clean      remove build output

param([string]$Target = "all")

$CXX      = "g++"
$CXXFLAGS = @("-std=c++17", "-Wall", "-Wextra", "-Wpedantic", "-Iinclude")
$BUILD    = "build"

$LIB = @(
    "src/cpu/registers.cpp", "src/cpu/decoder.cpp", "src/cpu/cpu.cpp",
    "src/cpu/execution_trace.cpp", "src/alu/alu.cpp", "src/memory/memory.cpp",
    "src/assembler/assembler.cpp", "src/debugger/debugger.cpp"
)
$SRV_SRC = @("src/server/server.cpp", "src/server/main_server.cpp")
$TESTS = @("test_alu", "test_memory", "test_decoder",
           "test_cpu", "test_assembler", "test_integration")

function Fail([string]$msg) { Write-Output "BUILD FAILED: $msg"; exit 1 }

if (-not (Test-Path $BUILD)) { New-Item -ItemType Directory -Path $BUILD | Out-Null }

function Build-Simulator {
    $a = $CXXFLAGS + $LIB + @("src/main.cpp", "-o", "$BUILD/cpu-simulator.exe")
    & $CXX @a
    if ($LASTEXITCODE -ne 0) { Fail "main build" }
}
function Build-Server {
    $a = $CXXFLAGS + $LIB + $SRV_SRC + @("-lws2_32", "-o", "$BUILD/cpu-server.exe")
    & $CXX @a
    if ($LASTEXITCODE -ne 0) { Fail "server build" }
}
function Build-ServerStatic {
    # Statically linked: runs on a clean Windows machine without MSYS2 runtime DLLs.
    $a = $CXXFLAGS + $LIB + $SRV_SRC + @("-static", "-lws2_32", "-o", "$BUILD/cpu-server-portable.exe")
    & $CXX @a
    if ($LASTEXITCODE -ne 0) { Fail "static server build" }
    Write-Output "Built portable $BUILD/cpu-server-portable.exe"
}

switch ($Target) {
    "all" {
        Build-Simulator
        Build-Server
        Write-Output "Built $BUILD/cpu-simulator.exe and $BUILD/cpu-server.exe"
    }
    "test" {
        Build-Simulator
        foreach ($t in $TESTS) {
            $ta = $CXXFLAGS + $LIB + @("tests/$t.cpp", "-o", "$BUILD/$t.exe")
            & $CXX @ta
            if ($LASTEXITCODE -ne 0) { Fail "building $t" }
            Write-Output "built $t"
        }
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
        Build-Simulator
        & "$BUILD/cpu-simulator.exe" --run examples/acceptance.asm
    }
    "serve" {
        Build-Server
        Write-Output "Starting CPU Simulator server at http://localhost:8080  (Ctrl+C to stop)"
        & "$BUILD/cpu-server.exe"
    }
    "static" { Build-ServerStatic }
    "deploy" {
        Build-ServerStatic
        Write-Output "Portable server ready: $BUILD/cpu-server-portable.exe"
        Write-Output "Copy it + the frontend/ folder to the target machine and run it as a service."
    }
    "clean" {
        if (Test-Path $BUILD) { Remove-Item -Recurse -Force $BUILD }
        Write-Output "cleaned"
    }
    default { Write-Output "Unknown target: $Target. Use: all | test | run | serve | clean"; exit 1 }
}
