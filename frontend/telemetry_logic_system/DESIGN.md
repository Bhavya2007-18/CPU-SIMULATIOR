---
name: Telemetry Logic System
colors:
  surface: '#101319'
  surface-dim: '#101319'
  surface-bright: '#363940'
  surface-container-lowest: '#0b0e14'
  surface-container-low: '#191c22'
  surface-container: '#1d2026'
  surface-container-high: '#272a31'
  surface-container-highest: '#32353c'
  on-surface: '#e1e2eb'
  on-surface-variant: '#bdc8d1'
  inverse-surface: '#e1e2eb'
  inverse-on-surface: '#2e3037'
  outline: '#87929a'
  outline-variant: '#3e484f'
  surface-tint: '#7bd0ff'
  primary: '#8ed5ff'
  on-primary: '#00354a'
  primary-container: '#38bdf8'
  on-primary-container: '#004965'
  inverse-primary: '#00668a'
  secondary: '#4cd6fb'
  on-secondary: '#003642'
  secondary-container: '#00b2d6'
  on-secondary-container: '#003f4e'
  tertiary: '#56e5a9'
  on-tertiary: '#003824'
  tertiary-container: '#30c88f'
  on-tertiary-container: '#004e34'
  error: '#ffb4ab'
  on-error: '#690005'
  error-container: '#93000a'
  on-error-container: '#ffdad6'
  primary-fixed: '#c4e7ff'
  primary-fixed-dim: '#7bd0ff'
  on-primary-fixed: '#001e2c'
  on-primary-fixed-variant: '#004c69'
  secondary-fixed: '#b3ebff'
  secondary-fixed-dim: '#4cd6fb'
  on-secondary-fixed: '#001f27'
  on-secondary-fixed-variant: '#004e5f'
  tertiary-fixed: '#6ffbbe'
  tertiary-fixed-dim: '#4edea3'
  on-tertiary-fixed: '#002113'
  on-tertiary-fixed-variant: '#005236'
  background: '#101319'
  on-background: '#e1e2eb'
  surface-variant: '#32353c'
typography:
  headline-lg:
    fontFamily: Inter
    fontSize: 1.5rem
    fontWeight: '600'
    lineHeight: 2rem
    letterSpacing: -0.015em
  headline-md:
    fontFamily: Inter
    fontSize: 1.125rem
    fontWeight: '600'
    lineHeight: 1.5rem
    letterSpacing: -0.01em
  headline-sm:
    fontFamily: Inter
    fontSize: 0.875rem
    fontWeight: '600'
    lineHeight: 1.25rem
    letterSpacing: 0.02em
  body-md:
    fontFamily: Inter
    fontSize: 0.8125rem
    fontWeight: '400'
    lineHeight: 1.25rem
  body-sm:
    fontFamily: Inter
    fontSize: 0.75rem
    fontWeight: '400'
    lineHeight: 1rem
  label-code-lg:
    fontFamily: JetBrains Mono
    fontSize: 0.875rem
    fontWeight: '500'
    lineHeight: 1.25rem
    letterSpacing: -0.01em
  label-code-md:
    fontFamily: JetBrains Mono
    fontSize: 0.75rem
    fontWeight: '400'
    lineHeight: 1.125rem
    letterSpacing: 0em
  label-code-sm:
    fontFamily: JetBrains Mono
    fontSize: 0.6875rem
    fontWeight: '500'
    lineHeight: 0.875rem
    letterSpacing: 0.03em
rounded:
  sm: 0.125rem
  DEFAULT: 0.25rem
  md: 0.375rem
  lg: 0.5rem
  xl: 0.75rem
  full: 9999px
spacing:
  gutter: 0.5rem
  gutter-md: 0.75rem
  margin: 0.75rem
  margin-lg: 1rem
  space-xs: 0.125rem
  space-sm: 0.25rem
  space-md: 0.5rem
  space-lg: 0.75rem
  space-xl: 1rem
---

## Brand & Style

The design system is engineered for computer architecture students, digital logic designers, and embedded systems researchers operating an interactive 8-bit CPU simulator laboratory. The UI projects the uncompromising rigor of high-performance developer environments, aerospace ground-control dashboards, and hardware logic analyzers.

The design philosophy combines **Technical Minimalism** with **Instrumental Precision**:
- Visual clutter is eliminated; data density is prioritized.
- UI chrome yields completely to real-time machine state, registers, memory maps, micro-operation timelines, and assembly listings.
- Every pixel, border, and color code carries semantic operational meaning (e.g., active execution, bus assertion, latch state, fault triggers).
- Interactions are rapid, tactile, and predictable, replicating dedicated laboratory hardware rather than a passive educational reader.

## Colors

The palette is anchored in deep cosmic tones calibrated to minimize ocular strain during prolonged debugging and simulation sessions:

- **Canvas & Base Surface**: `#0e1117` provides a deep obsidian foundation.
- **Panel Primary**: `#161b22` delineates major workspace quadrants (Assembly Editor, Register Bank, Memory Matrix, Microcode Sequencer).
- **Secondary Containers & Header Ribbons**: `#21262d` establishes clear structure for toolbars, tab headers, and data cells.
- **Structural Borders & Rulers**: `#30363d` provides crisp, hairline structural separation without visual heaviness.
- **Active Bus & Instruction Pointer**: Electric cyan (`#38bdf8`) and deep cyan (`#00b4d8`) denote current program counter (PC) states, signal transitions, and focused instructions.
- **Functional Semantics**:
  - **Halt / Verified State**: Emerald Green (`#10b981`) indicates successful step execution, stable clock cycles, and zero flags.
  - **Breakpoint / Trap / Warning**: Amber (`#f59e0b`) flags clock phase holds, hit breakpoints, and register saturation.
  - **Bus Fault / Syntax Error**: High-visibility Crimson (`#ef4444`) isolates parity faults, stack underflows/overflows, and micro-instruction decode traps.

## Typography

Typography balances clean structural chrome with dense monospace data visualization:

- **Inter**: Governs outer navigation, panel titles, modal controls, settings menus, and tooltips. It maintains neutrality, legibility at reduced scales, and minimal emotional friction.
- **JetBrains Mono**: Serves as the operational bedrock for assembly editing, hex view matrices, register bit allocations (Flags: C, Z, S, O), program counter tracks, and clock cycle logs. The font's ligature suppression and distinct character shapes (0 vs O, 1 vs l) prevent syntax misinterpretation.
- **Density Rules**: Line heights are kept tight (1.25 to 1.33 ratio) within memory grids to allow display of 256 memory locations simultaneously without forced pagination.

## Layout & Spacing

The layout is a zero-waste, multi-pane workbench optimized for widescreen monitors, with predictable folding for sub-1200px viewports:

- **Multi-Split Workspace Grid**: The primary viewport is subdivided into persistent quadrants:
  - Left dock (240px–320px): Assembly source file and symbol inspector.
  - Center stage (Flex): Disassembly view and Micro-instruction pipeline trace.
  - Right dock (280px–360px): CPU internal architecture (ALU, Accumulator, B-Register, PC, SP, Flags).
  - Bottom shelf (Resizable 180px–300px): Continuous 256-byte RAM grid and terminal log.
- **Tight Layout Metrics**: Margins and gutters conform to an 8px grid downscaled to 4px and 2px intervals (`0.125rem` to `0.25rem`) for sub-component nesting, ensuring that toolbars and status lines consume minimal screen space.
- **Responsive Stacking**: On tablet screens (<1024px), the right and bottom docks transition into collapsible slide-over drawers with dedicated status-strip toggles.

## Elevation & Depth

This design system avoids blurry drop-shadows and skeuomorphic gradients in favor of **Crisp Orthogonal Tiers** and **Subtle Structural Borders**:

- **Layer 0 (Canvas Base)**: `#0e1117` — Deepest coordinate plane.
- **Layer 1 (Modular Panels)**: `#161b22` bounded by `1px solid #30363d` — Tool containers and execution panes.
- **Layer 2 (Interactive Insets & Ribbons)**: `#21262d` bounded by `1px solid #30363d` — Data grid headers, active register cells, and code gutter strips.
- **Layer 3 (Overlays & Flyouts)**: `#161b22` bounded by `1px solid #38bdf8` (Active) or `#30363d` with a sharp, tight 4px blur (`rgba(0, 0, 0, 0.6)`), applied strictly to context menus and instruction selector popovers.
- **Active Bus Illumination**: Selected elements communicate state through edge highlights (`border-color: #38bdf8`) or subtle inset glow lines rather than broad outer glows.

## Shapes

The interface adopts a **Soft Industrial Geometry (Level 1)**:
- Baseline interactive surfaces use a strict 4px (`0.25rem`) border radius, preserving the compact feel of hardware instrumentation while softening brutal corners.
- Data tables, memory cell grids, and status line indicators use crisp 0px to 2px corners (`0` to `0.125rem`) to maximize pixel alignment across continuous tabular matrices.
- Buttons, input steppers, and pill status flags retain a uniform `0.25rem` rounding; circular pills and heavy border curves are prohibited to prevent visual fragmentation.

## Components

### Execution Controls & Buttons
- **Step / Run / Halt Toolbar Buttons**: 28px height, compact 8px horizontal padding. Neutral state: `#161b22` background with `#30363d` border. 
- **Step Next Button**: Accent highlighted with `#38bdf8` outline and active icon fill.
- **Halt / Stop Button**: Uses `#ef4444` indicator text when CPU is actively cycling; transitions to neutral when idle.

### Register & Flag Chips
- **Hex/Binary Display Chips**: Monospaced `#38bdf8` text on `#21262d` fill with `#30363d` stroke.
- **Single-bit Flags (C, Z, S, O)**: 20px x 20px square indicators. When asserted (1), background transitions to emerald `#10b981` at 20% opacity with solid `#10b981` text and border; when unasserted (0), background matches `#161b22` with muted `#484f58` text.

### Memory Matrix Grid
- **256-Byte Hex Editor**: Dense matrix displaying 16 bytes per row (Offset `0x00` through `0xF0`). 
- **Read/Write Flash Indicators**: Cells flash `#38bdf8` on bus read and `#f59e0b` on bus write during step transitions, fading back to `#e6edf3` text on `#161b22` over 250ms.
- **PC Pointer Overlay**: A high-contrast `#38bdf8` 1px border highlight encompassing the memory byte pointed to by the current Program Counter.

### Input Fields & Steppers
- **Address Jump & Clock Speed Control**: Inset `#0e1117` background with `#30363d` border, switching to `#38bdf8` outline upon focus. Numbers rendered in JetBrains Mono with explicit `0x` prefixes for hexadecimal entries.

### Breakpoint & Code Gutter
- **Assembly Line Numbers**: Muted `#484f58` text aligned right.
- **Breakpoint Anchor**: 8px circular dot toggleable in the gutter. Inactive: invisible; Set: solid `#f59e0b`; Hit: pulsed glowing `#f59e0b` ring.

### Oscilloscope / Logic Analyzer Timeline
- Digital trace showing CLK, FETCH, DECODE, EXECUTE, and READ/WRITE signals using 1px step waveforms rendered in `#38bdf8` and `#10b981`.