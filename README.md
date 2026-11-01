# CPU-SIMULATIOR

> **A software-based 8-bit CPU architecture simulator for visualizing how a processor fetches, decodes, and executes instructions.**

CPU SIMULATOR is a browser-based educational CPU simulator that recreates the fundamental components and execution cycle of a simple 8-bit processor entirely in software.

The project allows users to write assembly-like instructions, execute them step-by-step, and observe how data moves through the **Control Unit, ALU, Registers, and Memory**.

Built as a **Digital Logic & Computer Design (DLCD) Project-Based Learning (PBL)** project.

---

## 🚀 Features

### 🧠 CPU Architecture

- 8-bit CPU architecture
- Program Counter (PC)
- Instruction Register (IR)
- Accumulator
- General-purpose registers
- Stack Pointer
- Status/Flag register
- Fetch → Decode → Execute cycle
- Clock-cycle based execution

### ⚙️ ALU

The Arithmetic Logic Unit supports:

- Addition
- Subtraction
- AND
- OR
- XOR
- NOT
- Increment
- Decrement

### 🚩 Flags

The simulator tracks processor flags including:

- Zero Flag
- Carry Flag
- Sign Flag
- Overflow Flag

### 💾 Memory

- 8-bit memory architecture
- Addressable RAM
- Read/write operations
- Memory visualization
- Program storage
- Runtime memory updates

### 📝 Assembly

Write programs using the simulator's instruction set.

Example:

```asm
LOAD A, 10
LOAD B, 20
ADD A, B
STORE A, 50
HALT
```

The assembler converts the instructions into the simulator's internal machine representation before execution.

### ▶️ Execution Controls

- Run
- Pause
- Step-by-step execution
- Reset
- Execution speed control
- Register inspection
- Memory inspection

### 📊 Visualization

The simulator provides a visual representation of:

```text
Program
   ↓
Memory
   ↓
Instruction Register
   ↓
Control Unit
   ↓
Registers
   ↓
ALU
   ↓
Memory
```

This makes the internal operation of a processor easier to understand and observe.

---

# 🏗️ System Architecture

```text
                    ┌───────────────────┐
                    │     PROGRAM       │
                    │  Assembly Code    │
                    └─────────┬─────────┘
                              │
                              ▼
                    ┌───────────────────┐
                    │     ASSEMBLER     │
                    │ Instruction Parser│
                    └─────────┬─────────┘
                              │
                              ▼
                    ┌───────────────────┐
                    │      MEMORY       │
                    │      256 × 8      │
                    └─────────┬─────────┘
                              │
                              ▼
              ┌───────────────────────────────┐
              │             CPU               │
              │                               │
              │   ┌───────────────────────┐   │
              │   │    CONTROL UNIT       │   │
              │   └───────────┬───────────┘   │
              │               │               │
              │   ┌───────────▼───────────┐   │
              │   │       REGISTERS       │   │
              │   └───────────┬───────────┘   │
              │               │               │
              │   ┌───────────▼───────────┐   │
              │   │          ALU          │   │
              │   └───────────┬───────────┘   │
              │               │               │
              │   ┌───────────▼───────────┐   │
              │   │    FLAGS / STATUS     │   │
              │   └───────────────────────┘   │
              └───────────────┬───────────────┘
                              │
                              ▼
                         RESULT / RAM
```

---

# 🔄 CPU Execution Cycle

Every instruction follows the basic processor cycle:

```text
┌─────────┐
│  FETCH  │
└────┬────┘
     ↓
┌─────────┐
│ DECODE  │
└────┬────┘
     ↓
┌─────────┐
│ EXECUTE │
└────┬────┘
     ↓
┌─────────┐
│  WRITE  │
└────┬────┘
     ↓
┌─────────┐
│ UPDATE  │
│  FLAGS  │
└────┬────┘
     ↓
   NEXT
INSTRUCTION
```

### Example

For:

```asm
ADD A, B
```

the CPU performs:

```text
1. Fetch ADD instruction
        ↓
2. Decode instruction
        ↓
3. Read A and B
        ↓
4. Send operands to ALU
        ↓
5. Perform addition
        ↓
6. Store result in A
        ↓
7. Update flags
        ↓
8. Increment Program Counter
```

---

# 📚 Instruction Set

The simulator supports a custom instruction set designed for the 8-bit architecture.

| Instruction | Description |
|---|---|
| `LOAD` | Load an immediate value into a register |
| `STORE` | Store register value into memory |
| `MOV` | Copy data between registers |
| `ADD` | Addition |
| `SUB` | Subtraction |
| `AND` | Bitwise AND |
| `OR` | Bitwise OR |
| `XOR` | Bitwise XOR |
| `NOT` | Bitwise NOT |
| `INC` | Increment |
| `DEC` | Decrement |
| `CMP` | Compare values |
| `JMP` | Unconditional jump |
| `JZ` | Jump if Zero Flag is set |
| `HALT` | Stop execution |

> The instruction set may evolve as additional CPU functionality is implemented.

---

# 🧪 Example Program

```asm
LOAD A, 10
LOAD B, 20

ADD A, B

STORE A, 50

LOAD C, 5

SUB A, C

STORE A, 51

HALT
```

### Expected flow

```text
A = 10
B = 20

      ↓ ADD

A = 30

      ↓ STORE

MEM[50] = 30

      ↓ LOAD C

C = 5

      ↓ SUB

A = 25

      ↓ STORE

MEM[51] = 25
```

---

# 🛠️ Technology Stack

The project is designed as a completely software-based application.

### Frontend

- React
- TypeScript
- HTML
- CSS
- Tailwind CSS

### CPU Simulation

- TypeScript
- Custom CPU emulator
- Custom instruction decoder
- Custom assembler/parser

### Visualization

- SVG / Canvas
- Interactive CPU components
- Real-time state updates

### Deployment

The simulator can be deployed as a static web application using platforms such as Vercel or GitHub Pages.

---

# 📁 Project Structure

```text
CPU-SIMULATOR/
│
├── src/
│   │
│   ├── cpu/
│   │   ├── cpu.ts
│   │   ├── controlUnit.ts
│   │   ├── instructionDecoder.ts
│   │   └── clock.ts
│   │
│   ├── alu/
│   │   ├── alu.ts
│   │   └── flags.ts
│   │
│   ├── registers/
│   │   └── registers.ts
│   │
│   ├── memory/
│   │   └── memory.ts
│   │
│   ├── assembler/
│   │   ├── parser.ts
│   │   └── instructionSet.ts
│   │
│   ├── components/
│   │   ├── CPUView.tsx
│   │   ├── RegisterPanel.tsx
│   │   ├── MemoryView.tsx
│   │   ├── ALUView.tsx
│   │   └── CodeEditor.tsx
│   │
│   └── App.tsx
│
├── public/
│
├── README.md
├── package.json
└── tsconfig.json
```

---

# 👥 Team Contributions

The project is divided into four major modules.

### Person 1 — CPU Architecture & Control Unit

Responsible for:

- CPU architecture
- Control Unit
- Program Counter
- Instruction Register
- Fetch/Decode/Execute cycle
- Instruction decoding
- CPU state management

### Person 2 — ALU & Registers

Responsible for:

- Arithmetic operations
- Logical operations
- Registers
- Flag register
- Zero/Carry/Sign/Overflow handling
- ALU testing

### Person 3 — Memory & Instruction System

Responsible for:

- RAM implementation
- Memory addressing
- Instruction set
- Assembly parser
- Instruction encoding
- Program loading

### Person 4 — UI & Integration

Responsible for:

- User interface
- CPU visualization
- Code editor
- Execution controls
- Register/memory visualization
- Animation
- Integration of all modules

---

# 🎯 Project Objectives

The primary objectives of CPU SIMULATOR are to:

1. Understand the internal architecture of a CPU.
2. Simulate the basic operation of a processor entirely through software.
3. Demonstrate the Fetch–Decode–Execute instruction cycle.
4. Understand the role of the ALU and registers.
5. Demonstrate memory addressing and data transfer.
6. Understand how assembly instructions are executed.
7. Visualize the interaction between different CPU components.
8. Provide an interactive learning environment for digital logic and computer architecture.

---

# 📖 DLCD Concepts Demonstrated

This project demonstrates several concepts studied in **Digital Logic & Computer Design**:

- Logic operations
- Arithmetic circuits
- ALU
- Registers
- Flip-flop based state concepts
- Control Unit
- Instruction decoding
- Data buses
- Memory organization
- Addressing
- Counters
- Sequential logic
- Finite-state control
- CPU architecture

---

# 🔮 Future Scope

Possible future improvements include:

- 16-bit CPU architecture
- Pipelining
- Cache memory simulation
- Interrupt handling
- Stack-based instructions
- I/O simulation
- Branch prediction
- Multi-cycle CPU architecture
- Custom machine-code editor
- CPU performance statistics
- Circuit-level visualization
- Logic-gate-level CPU implementation
- Export/import of assembly programs

---

# 📌 Limitations

This project is an educational CPU simulator and does not attempt to reproduce a commercial processor.

The simulated CPU has:

- A simplified instruction set
- Limited memory
- Simplified control logic
- No physical hardware implementation
- No operating system layer

Its purpose is to demonstrate the fundamental principles of processor architecture in an interactive environment.

---

# 🎓 Academic Context

**Project:** CPU SIMULATOR  
**Subject:** Digital Logic & Computer Design (DLCD)  
**Project Type:** Project-Based Learning (PBL)  
**Architecture:** 8-bit Software CPU  
**Implementation:** Fully Software-Based

---

# 📜 License

This project is developed for educational and academic purposes.

---

## CPU SIMULATOR

**Understand the CPU by building the CPU.**
