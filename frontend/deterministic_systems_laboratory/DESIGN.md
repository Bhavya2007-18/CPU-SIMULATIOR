---
name: Deterministic Systems Laboratory
colors:
  surface: '#131316'
  surface-dim: '#131316'
  surface-bright: '#39393c'
  surface-container-lowest: '#0e0e11'
  surface-container-low: '#1b1b1e'
  surface-container: '#1f1f22'
  surface-container-high: '#2a2a2d'
  surface-container-highest: '#353438'
  on-surface: '#e4e1e6'
  on-surface-variant: '#c2c6d6'
  inverse-surface: '#e4e1e6'
  inverse-on-surface: '#303033'
  outline: '#8c909f'
  outline-variant: '#424754'
  surface-tint: '#adc6ff'
  primary: '#adc6ff'
  on-primary: '#002e6a'
  primary-container: '#4d8eff'
  on-primary-container: '#00285d'
  inverse-primary: '#005ac2'
  secondary: '#c6c5cf'
  on-secondary: '#2f3038'
  secondary-container: '#4a4b53'
  on-secondary-container: '#bcbbc5'
  tertiary: '#c8c6c9'
  on-tertiary: '#303033'
  tertiary-container: '#919094'
  on-tertiary-container: '#29292c'
  error: '#ffb4ab'
  on-error: '#690005'
  error-container: '#93000a'
  on-error-container: '#ffdad6'
  primary-fixed: '#d8e2ff'
  primary-fixed-dim: '#adc6ff'
  on-primary-fixed: '#001a42'
  on-primary-fixed-variant: '#004395'
  secondary-fixed: '#e3e1ec'
  secondary-fixed-dim: '#c6c5cf'
  on-secondary-fixed: '#1a1b22'
  on-secondary-fixed-variant: '#46464e'
  tertiary-fixed: '#e4e1e5'
  tertiary-fixed-dim: '#c8c6c9'
  on-tertiary-fixed: '#1b1b1e'
  on-tertiary-fixed-variant: '#47464a'
  background: '#131316'
  on-background: '#e4e1e6'
  surface-variant: '#353438'
typography:
  headline-lg:
    fontFamily: Inter
    fontSize: 1.25rem
    fontWeight: '600'
    lineHeight: 1.5rem
  headline-md:
    fontFamily: Inter
    fontSize: 1rem
    fontWeight: '600'
    lineHeight: 1.25rem
  headline-sm:
    fontFamily: Inter
    fontSize: 0.875rem
    fontWeight: '600'
    lineHeight: 1.125rem
  body-lg:
    fontFamily: Inter
    fontSize: 0.875rem
    fontWeight: '400'
    lineHeight: 1.25rem
  body-md:
    fontFamily: Inter
    fontSize: 0.8125rem
    fontWeight: '400'
    lineHeight: 1.125rem
  body-sm:
    fontFamily: Inter
    fontSize: 0.75rem
    fontWeight: '400'
    lineHeight: 1rem
  mono-code:
    fontFamily: JetBrains Mono
    fontSize: 0.8125rem
    fontWeight: '400'
    lineHeight: 1.25rem
  mono-data:
    fontFamily: JetBrains Mono
    fontSize: 0.75rem
    fontWeight: '500'
    lineHeight: 1rem
  mono-data-compact:
    fontFamily: JetBrains Mono
    fontSize: 0.6875rem
    fontWeight: '500'
    lineHeight: 0.875rem
  label-md:
    fontFamily: Inter
    fontSize: 0.75rem
    fontWeight: '500'
    lineHeight: 1rem
  label-sm:
    fontFamily: Inter
    fontSize: 0.6875rem
    fontWeight: '600'
    lineHeight: 0.875rem
  label-micro:
    fontFamily: JetBrains Mono
    fontSize: 0.625rem
    fontWeight: '600'
    lineHeight: 0.75rem
rounded:
  sm: 0.125rem
  DEFAULT: 0.25rem
  md: 0.375rem
  lg: 0.5rem
  xl: 0.75rem
  full: 9999px
spacing:
  gutter: 0.25rem
  margin: 0.5rem
  space-xs: 0.125rem
  space-sm: 0.25rem
  space-md: 0.5rem
  space-lg: 0.75rem
  space-xl: 1rem
---

## Brand & Style

This design system is tailored for an 8-bit CPU architecture simulator, debugger, and instruction-level laboratory environment. It rejects ornamental sci-fi tropes, decorative neon, ambient glows, simulated 3D elements, and bloated spatial padding. Instead, it embodies the philosophy of high-precision electronic instrumentation, benchtop logic analyzers, and industry-standard IDEs (such as JetBrains and VS Code).

### Personality & Values
- **Deterministic & Grounded:** Every pixel maps directly to a discrete structural utility or logical state. Nothing floats purely for aesthetic indulgence.
- **High Information Density:** Microarchitecture inspection demands simultaneous visibility into instruction pointers, accumulator flags, raw memory grids, and stack pointers without artificial pagination or oversized padding.
- **Quiet Utility:** Low-saturation backgrounds keep eye strain minimal across 10-hour systems debugging sessions. Color is deployed strictly as semantic state signaling (halted, running, breakpoint active, memory mutation).

### Target Audience
Computer science students, microarchitecture researchers, systems engineers, and retro-computing hobbyists who require precise, distraction-free execution stepping and byte-accurate state verification.

## Colors

The system uses a strictly non-emissive, matte palette built from zinc-charcoal values. Surface separation relies entirely on subtle contrast and crisp hairline dividers rather than glow or drop-shadow effects.

### Base Surfaces
- **Canvas Base (`#121214`):** Global window backdrop, inactive docking gutters, and outer viewport margin.
- **Panel Surface (`#18181b`):** Primary workspace panels including the code editor, assembly viewport, and register tables.
- **Raised Container / Header (`#1e1e24`):** Panel tabs, toolbar bands, input backgrounds, and table headers.
- **Active Focus / Sub-row Hover (`#27272a`):** Interactive row highlights and active pane tab fills.

### Structural Lines
- **Subtle Panel Divider (`#27272a`):** Standard 1px divider for internal grid cells and list items.
- **Active Structural Border (`#333338`):** Splitter bars, docking borders, and focused panel perimeters.

### Core Typography
- **Primary Readout (`#f4f4f5`):** Monospace hex values, active instruction text, and core headers.
- **Secondary Readout (`#a1a1aa`):** Inactive register labels, assembly comments, and structural captions.
- **Muted / Inactive (`#71717a`):** Memory row offsets, line numbers, and disabled actions.

### Semantic Instrumentation Accents
- **Engineering Blue (`#3b82f6`):** Focused pane accent, program counter (PC) cursor, selection states. Strictly matte with no box-shadow blooming.
- **Execution Ready / Step Green (`#22c55e`):** Halted/ready state, zero-flag assertions, memory match hits.
- **Warning / Breakpoint Amber (`#f59e0b`):** Hardware/software breakpoints, watchpoint hits, cycle overflow warnings.
- **Exception Red (`#ef4444`):** Illegal opcode traps, stack overflow, parity errors, bus collisions.

## Typography

The typographic hierarchy implements a bifurcated dual-engine model:

1. **System Interface (`Inter`):** Handles window chrome, panel header labels, modal dialogs, status bar notifications, and settings menus. It prioritizes compact vertical rendering and universal readability without eccentric glyph features.
2. **Deterministic Data Engine (`JetBrains Mono`):** Dedicated exclusively to raw computational artifacts: assembly instruction streams, 8-bit hex registers (`A`, `X`, `Y`, `SP`, `PC`), binary flag indicators (`N V - B D I Z C`), memory dumps (`0x0000 - 0xFFFF`), and timing cycle counters. Tabular figures (`tnum`) and zero slash glyphs are enforced across all monospace tokens to prevent visual ambiguity between `0` and `O`.

### Scaling Rules
Font sizes larger than `1.25rem` (`20px`) are barred entirely from panel internals. Compact data layouts require disciplined, sub-pixel aligned line heights (e.g., standard code lines locked to `20px` intervals; memory rows locked to `16px`).

## Layout & Spacing

The interface uses a multi-docked workstation layout (fixed-coordinate docking matrix with variable splitters) rather than a fluid promotional grid.

### Grid & Viewport Partitioning
- **Split-Pane Architecture:** The application frame relies on non-overlapping, edge-to-edge docking partitions: Primary Code Editor / Disassembler (variable flex), Register Matrix (fixed `280px`), Memory Hex Inspector (fixed `360px`), and Execution / Trace Log (bottom dock, collapsable).
- **Splitter Thickness:** Pane splitters are precisely `1px` to `2px` thick, matching `#27272a` or `#333338`.
- **Zero Wasted Margin:** No outer atmospheric canvas borders. The workspace docks flush to the application frame, reserving space strictly for high-frequency telemetry.

### Spacing Scale
- `space-xs` (2px): Gap between micro binary bit switches and status badges.
- `space-sm` (4px): Internal padding for table cells, icon buttons, and segmented controls.
- `space-md` (8px): Toolbar button clusters, input inner padding, and header heights.
- `space-lg` (12px): Modal dialog element spacing and drawer inner margins.
- `space-xl` (16px): Maximum padding used for settings forms and modal borders.

## Elevation & Depth

This system avoids ambient shadows, colored glows, drop shadows, and blurred backdrops. Depth is established purely through surface value tiers and hairline borders.

### Surface Tiers
- **Level 0 (Recessed, `#121214`):** Canvas backing, editor gutters, scrollbar tracks.
- **Level 1 (Base Panel, `#18181b`):** Work surfaces, assembly code listings, memory grids.
- **Level 2 (Raised Surfaces, `#1e1e24`):** Active panel tabs, top command toolbars, status bar, table header cells.
- **Level 3 (Interactive Overlays, `#27272a`):** Dropdown select menus, context menus, tooltips, and floating register hover cards.

### Edge Definition
All overlay elements (tooltips, command palettes, context menus) use a 1px solid border (`#333338`). When a modal window interrupts the viewport, it uses a flat, 50% opacity backdrop overlay (`rgba(0, 0, 0, 0.6)`) with no blur filter.

## Shapes

The geometric framework is tight and mechanical, reflecting precision instrumentation.

### Corner Radii
- **Standard Corners (0.25rem / 4px):** Applied to buttons, input fields, code chips, register badges, panel tab corners, and modal perimeters.
- **Flush / Sharp Corners (0px):** Applied to all panel dock edges, window edges, table cells, splitter handles, and memory map grid cells.
- **Micro Corners (2px):** Applied to individual flag status indicators and breakpoint markers in the editor gutter.
- **Prohibited:** Rounded pill shapes, circular interactive buttons, and organic curves.

## Components

### Buttons & Toolbars
- **Toolbar Action Buttons:** Height fixed at `26px`. Default state is ghost/transparent with `#a1a1aa` icon/label. On hover: background `#27272a`, text `#f4f4f5`. Active/depressed: `#333338`.
- **Primary Action (e.g., "Run / Continue"):** Height `26px` or `28px`. Background `#2563eb`, border 1px solid `#3b82f6`, text `#ffffff`. No ambient drop shadows.
- **Step / Halt Controls:** Segmented button group with 1px internal dividers (`#333338`). Compact padding: `4px 8px`.

### Register Inspector & Status Indicators
- **Register Pair Display:** Boxed key-value metric units. Label (`A`, `X`, `Y`, `PC`, `SP`) rendered in `label-sm` (`#71717a`); value rendered in `mono-data` (`#f4f4f5`).
- **Bitwise Status Flags (N V - B D I Z C):** Monospace 1-bit indicators arranged horizontally. Active bit displays `#22c55e` or `#3b82f6` text against `#1e1e24` background with 1px border. Inactive bit displays `#71717a` against `#121214`.
- **Mutation Flash:** When a register changes state during a single step, the background transitions to `#1e293b` (muted slate blue) and clears after the step without animated movement.

### Assembly Editor & Disassembly Viewport
- **Gutter:** Line numbers and instruction byte offsets in `mono-data-compact` with `#71717a`.
- **Breakpoints:** Small 8x8px square or diamond at the line offset in `#f59e0b` (warning amber) or `#ef4444`. No blur or pulse.
- **Execution Pointer:** Solid non-glowing `#3b82f6` arrow icon (`>`) with a subtle `rgba(59, 130, 246, 0.12)` full-row background fill.

### Memory Hex Table
- **Layout:** Strict grid layout: 16 byte columns (`00` to `0F`) flanked by offset prefix (`0x0200`) and ASCII representation.
- **Monospace Alignment:** JetBrains Mono tabular figures. Modified bytes highlighted with a 1px border in `#f59e0b`. Zero values dimmed to `#52525b` to keep populated bytes visually prominent.

### Input Fields & Search Bars
- **Style:** Background `#121214`, border 1px solid `#27272a`, text `#f4f4f5`, radius `4px`.
- **Focus:** 1px solid `#3b82f6` without glow ring or expansion.
- **Height:** Compact `24px` for inline filter inputs; `28px` for modal dialogs.

### Context Menus & Tooltips
- **Context Menus:** Background `#1e1e24`, border 1px solid `#333338`, radius `4px`, padding `4px`. Menu item height `24px`, hover fill `#27272a`.
- **Tooltips:** Background `#121214`, border 1px solid `#333338`, text `#f4f4f5`, font size `0.6875rem`, padding `2px 6px`. Zero delay on critical hardware tooltips.