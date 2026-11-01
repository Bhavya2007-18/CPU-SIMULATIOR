"use strict";
// CPU Simulator frontend — drives the real C++ engine through the local
// HTTP API exposed by build/cpu-server (see src/server/server.cpp).

const $ = (id) => document.getElementById(id);
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const hex2 = (n) => n.toString(16).toUpperCase().padStart(2, "0");
const bin8 = (n) => n.toString(2).padStart(8, "0").replace(/(.{4})/, "$1 ");

const app = {
  listing: [],                 // [{addr,length,text,bytes}]
  breakpoints: new Set(),      // addresses
  clockHz: 5,                  // 0 = MAX (bulk run)
  running: false,
  pc: 0,
  halted: false,
  prevMem: new Array(256).fill(0),
  memCells: [],                // DOM refs [row][col]
  memAddrLabels: [],
};

// ---------------- API helpers ----------------
async function apiGet(path) {
  try { const r = await fetch(path); return await r.json(); }
  catch (e) { console.error(e); return { ok: false, error: String(e) }; }
}
async function apiPost(path, body, contentType) {
  try {
    const opt = { method: "POST" };
    if (body !== undefined) { opt.body = body; if (contentType) opt.headers = { "Content-Type": contentType }; }
    const r = await fetch(path, opt); return await r.json();
  } catch (e) { console.error(e); return { ok: false, error: String(e) }; }
}

// ---------------- rendering ----------------
function setStatus(msg, kind) {
  $("statusMsg").textContent = msg;
  const dot = $("engineDot"), label = $("engineLabel");
  if (kind === "error") { dot.className = "w-1.5 h-1.5 rounded-full bg-red-500"; label.className = "text-red-400 font-medium tracking-wide"; label.textContent = "ERROR"; }
  else if (app.halted) { dot.className = "w-1.5 h-1.5 rounded-full bg-amber-500"; label.className = "text-amber-400 font-medium tracking-wide"; label.textContent = "HALTED"; }
  else if (app.running) { dot.className = "w-1.5 h-1.5 rounded-full bg-blue-500"; label.className = "text-blue-400 font-medium tracking-wide"; label.textContent = "RUNNING"; }
  else { dot.className = "w-1.5 h-1.5 rounded-full bg-emerald-500"; label.className = "text-emerald-400 font-medium tracking-wide"; label.textContent = "READY"; }
}

function renderState(s) {
  if (!s) return;
  app.pc = s.PC; app.halted = s.halted;
  $("hdrPC").textContent = "0x" + hex2(s.PC);
  $("hdrCycle").textContent = s.cycle ?? 0;
  // general registers
  const regs = [["A", s.A, true], ["B", s.B], ["C", s.C], ["D", s.D]];
  $("regBody").innerHTML = regs.map(([n, v, acc]) => `
    <tr class="hover:bg-zinc-800/40 text-zinc-200">
      <td class="py-1.5 px-2 font-bold ${acc ? "text-amber-300" : "text-zinc-300"}">${n}${acc ? " (ACC)" : ""}</td>
      <td class="py-1.5 px-1 text-blue-300">0x${hex2(v)}</td>
      <td class="py-1.5 px-1 text-zinc-400">${v}</td>
      <td class="py-1.5 px-1 text-zinc-500 text-[10px]">${bin8(v).trim()}</td>
    </tr>`).join("");
  // control
  $("cPC").innerHTML = `0x${hex2(s.PC)} <span class="text-zinc-500 font-normal">(${s.PC})</span>`;
  $("cIR").textContent = s.lastInstruction || "—";
  $("cSP").innerHTML = `0x${hex2(s.SP)} <span class="text-zinc-500 font-normal">(${s.SP})</span>`;
  // flags
  const fset = (id, on) => { const e = $(id); e.textContent = on ? 1 : 0; e.className = "font-bold mt-0.5 " + (on ? "f-on" : "text-zinc-500"); };
  fset("fZ", s.Z); fset("fC", s.C); fset("fS", s.S); fset("fV", s.V);
  // pipeline phase indicator
  ["phFETCH","phDECODE","phEXECUTE","phWB"].forEach(id => { const e=$(id); e.className="py-1 bg-zinc-900 border border-ide-border rounded text-zinc-500"; });
  if (app.listing.length) $("phEXECUTE").className = "py-1 bg-blue-600 text-white font-semibold rounded border border-blue-500";
  setStatus($("statusMsg").textContent);
}

function buildMemoryGrid() {
  const g = $("memGrid"); g.innerHTML = "";
  for (let r = 0; r < 16; r++) {
    const a = document.createElement("div"); a.className = "mem-addr"; a.textContent = hex2(r * 16); g.appendChild(a);
    for (let c = 0; c < 16; c++) {
      const cell = document.createElement("div"); cell.className = "mem-cell mem-zero"; cell.textContent = "00";
      g.appendChild(cell); (app.memCells[r * 16 + c] = cell);
    }
  }
}

function renderMemory(bytes) {
  for (let i = 0; i < 256; i++) {
    const v = bytes[i] ?? 0; const cell = app.memCells[i]; if (!cell) continue;
    cell.textContent = hex2(v);
    let cls = "mem-cell";
    if (v === 0) cls += " mem-zero";
    if (v !== app.prevMem[i]) cls += " mem-changed";
    if (i === app.pc) cls += " mem-pc";
    cell.className = cls;
  }
  app.prevMem = bytes.slice();
}

function highlightListingText(text) {
  // very light syntax highlight for the listing line text
  return text
    .replace(/^(\w+)\b/, '<span class="op">$1</span>')
    .replace(/([ABCD])(?![A-Za-z])/g, '<span class="reg">$1</span>')
    .replace(/\b(\d+)\b/g, '<span class="num">$1</span>')
    .replace(/^(\w+:)/, '<span class="lab">$1</span>');
}

function renderListing() {
  const box = $("listing");
  if (!app.listing.length) { box.innerHTML = '<div class="p-3 text-zinc-600">No program loaded. Click “Assemble &amp; Load”.</div>'; return; }
  box.innerHTML = app.listing.map(it => {
    const isPC = it.addr === app.pc;
    const hasBP = app.breakpoints.has(it.addr);
    return `<div class="lrow ${isPC ? "active" : ""}">
      <div class="lgutter ${hasBP ? "has" : ""}" data-addr="${it.addr}"><span class="dot"></span></div>
      <span class="laddr">0x${hex2(it.addr)}</span>
      <span class="lbytes">${it.bytes.map(hex2).join(" ")}</span>
      <span class="ltext">${highlightListingText(it.text)}</span>
      ${isPC ? '<span class="ml-auto pr-2 text-[10px] text-blue-400">◀ PC</span>' : ""}
    </div>`;
  }).join("");
  box.querySelectorAll(".lgutter").forEach(d => d.addEventListener("click", () => toggleBreak(parseInt(d.dataset.addr, 10))));
}

function renderBreakpoints() {
  const box = $("breakpointList");
  const arr = [...app.breakpoints].sort((a, b) => a - b);
  if (!arr.length) { box.innerHTML = '<div class="text-zinc-600">(none)</div>'; return; }
  box.innerHTML = arr.map(a => `
    <label class="flex items-center justify-between p-1 rounded hover:bg-ide-elevated cursor-pointer group">
      <span class="flex items-center gap-2"><span class="w-2 h-2 rounded-full bg-red-500"></span><span class="text-zinc-300">0x${hex2(a)}</span></span>
      <button class="text-zinc-500 hover:text-red-400" data-unbreak="${a}"><span class="material-symbols-outlined text-[12px]">close</span></button>
    </label>`).join("");
  box.querySelectorAll("[data-unbreak]").forEach(b => b.addEventListener("click", () => toggleBreak(parseInt(b.dataset.unbreak, 10))));
}

function renderTrace(trace) {
  $("traceCount").textContent = trace.length + (trace.length === 1 ? " cycle" : " cycles");
  const body = $("traceBody");
  if (!trace.length) { body.innerHTML = '<tr><td class="py-2 px-2 text-zinc-600" colspan="5">No instructions executed yet.</td></tr>'; return; }
  const last = trace.length - 1;
  body.innerHTML = trace.map((e, i) => {
    const cls = i === last ? "active" : (e.after.halted ? "halted-row" : "");
    const b = e.before, a = e.after;
    return `<tr class="${cls} hover:bg-zinc-900/60">
      <td class="py-1 px-2 text-zinc-500">${String(e.cycle).padStart(3, "0")}</td>
      <td class="py-1 px-2">0x${hex2(e.pcBefore)}→0x${hex2(e.pcAfter)}</td>
      <td class="py-1 px-2 text-blue-300">${e.instruction}</td>
      <td class="py-1 px-2 text-zinc-300">${b.A}/${b.B}/${b.C}/${b.D} → ${a.A}/${a.B}/${a.C}/${a.D}</td>
      <td class="py-1 px-2 text-zinc-400">${b.Z}${b.C}${b.S}${b.V} → ${a.Z}${a.C}${a.S}${a.V}</td>
    </tr>`;
  }).join("");
}

async function refreshAll() {
  const [st, mem, bp] = await Promise.all([
    apiGet("/api/state"),
    apiGet("/api/memory?start=0&count=256"),
    apiGet("/api/breakpoints"),
  ]);
  if (st.state) { st.state.lastInstruction = st.lastInstruction; st.state.cycle = st.cycle; }
  renderState(st.state || {});
  renderMemory(mem.bytes || []);
  app.breakpoints = new Set(bp.breakpoints || []);
  renderBreakpoints();
  renderListing();
  const t = await apiGet("/api/trace?count=50");
  renderTrace(t.trace || []);
}

// ---------------- actions ----------------
async function assemble() {
  const src = $("source").value;
  const res = await apiPost("/api/assemble", src, "text/plain");
  if (!res.ok) { setStatus("Assembly error: " + res.error, "error"); $("asmStatusText").textContent = "Assembly failed: " + res.error; $("asmStatusText").className = "text-red-400"; return; }
  app.listing = res.listing || [];
  $("asmStatusText").textContent = `Assembly OK — ${res.instructions} instructions (${res.bytes} bytes) loaded at 0x00`;
  $("asmStatusText").className = "text-emerald-400";
  $("progInfo").textContent = `${res.instructions} instr · ${res.bytes} B`;
  await refreshAll();
}

async function doStep() {
  if (app.halted) { setStatus("CPU is halted — reset to run again."); return; }
  const res = await apiPost("/api/step");
  if (!res.ok && res.error) { setStatus("Fault: " + res.error, "error"); }
  await refreshAll();
}

async function doRun() {
  if (app.halted) { setStatus("CPU is halted — reset to run again."); return; }
  if (app.clockHz === 0) {
    setRunning(true);
    const res = await apiPost("/api/run");
    await refreshAll();
    setRunning(false);
    setStatus(runMsg(res));
    return;
  }
  // animated stepping at clockHz
  setRunning(true); app.running = true;
  const delay = 1000 / app.clockHz;
  try {
    while (app.running) {
      const res = await apiPost("/api/step");
      if (!res.ok && res.error) { setStatus("Fault: " + res.error, "error"); break; }
      await refreshAll();
      if (res.halted) { setStatus("CPU HALTED"); break; }
      if (app.breakpoints.has(app.pc)) { setStatus("Stopped at breakpoint (0x" + hex2(app.pc) + ")"); break; }
      if (delay > 0) await sleep(delay); else await sleep(0);
    }
  } finally { app.running = false; setRunning(false); }
}

function setRunning(on) {
  app.running = on;
  $("btnPause").classList.toggle("opacity-60", !on);
  setStatus($("statusMsg").textContent);
}

function runMsg(res) {
  if (!res || !res.ok) return "Run failed: " + (res && res.error);
  if (res.stoppedAt === "halt") return `Run finished — CPU HALTED (${res.executed} instructions)`;
  if (res.stoppedAt === "breakpoint") return `Stopped at breakpoint (0x${hex2(res.state ? res.state.PC : 0)}) — ${res.executed} instructions`;
  if (res.stoppedAt === "limit") return `Execution limit reached (possible infinite loop) — ${res.executed} instructions`;
  return `Run stopped (${res.executed} instructions)`;
}

async function doReset() {
  await apiPost("/api/reset");
  await refreshAll();
  setStatus("CPU reset.");
}

async function toggleBreak(addr) {
  if (app.breakpoints.has(addr)) await apiPost("/api/unbreak?addr=" + addr);
  else await apiPost("/api/break?addr=" + addr);
  const bp = await apiGet("/api/breakpoints");
  app.breakpoints = new Set(bp.breakpoints || []);
  renderBreakpoints(); renderListing();
}

async function clearBreaks() {
  for (const a of [...app.breakpoints]) await apiPost("/api/unbreak?addr=" + a);
  app.breakpoints.clear(); renderBreakpoints(); renderListing();
}

async function loadExample(name) {
  const res = await apiGet("/api/example?name=" + encodeURIComponent(name));
  if (!res.ok) return;
  $("source").value = res.source;
  setStatus("Loaded " + name + ". Click Assemble & Load to build it.");
}

async function loadExamplesList() {
  const res = await apiGet("/api/examples");
  const box = $("examplesList");
  if (!res.files || !res.files.length) { box.innerHTML = '<div class="px-3 text-zinc-600">(no examples found)</div>'; return; }
  box.innerHTML = res.files.map(f =>
    `<div class="flex items-center gap-1.5 px-2 py-1 text-zinc-300 hover:text-zinc-100 hover:bg-ide-elevated/50 rounded cursor-pointer" data-ex="${f}">
      <span class="material-symbols-outlined text-[13px] text-blue-400">code</span><span class="truncate">${f}</span></div>`).join("");
  box.querySelectorAll("[data-ex]").forEach(d => d.addEventListener("click", () => loadExample(d.dataset.ex)));
}

// ---------------- clock speed buttons ----------------
function wireClock() {
  document.querySelectorAll(".clk").forEach(b => b.addEventListener("click", () => {
    app.clockHz = parseInt(b.dataset.hz, 10);
    document.querySelectorAll(".clk").forEach(x => { x.className = "clk px-1.5 py-0.5 text-zinc-400 hover:text-zinc-200 rounded"; });
    b.className = "clk px-1.5 py-0.5 bg-zinc-700 text-zinc-100 font-semibold rounded";
  }));
}

// ---------------- wire up ----------------
function init() {
  buildMemoryGrid();
  wireClock();
  $("btnAssemble").addEventListener("click", assemble);
  $("btnStep").addEventListener("click", doStep);
  $("btnRun").addEventListener("click", doRun);
  $("btnPause").addEventListener("click", () => { app.running = false; setStatus("Paused."); });
  $("btnReset").addEventListener("click", doReset);
  $("clearTrace").addEventListener("click", doReset);   // reset clears the trace
  $("clearBreaks").addEventListener("click", clearBreaks);
  document.addEventListener("keydown", (e) => {
    if (e.key === "F5") { e.preventDefault(); doRun(); }
    else if (e.key === "F6") { e.preventDefault(); doStep(); }
  });
  // seed the editor with the acceptance program so the app is useful on first load
  $("source").value =
    "; The acceptance program (spec section 34)\n" +
    "START:\n" +
    "    LOAD A, 10\n" +
    "    LOAD B, 20\n" +
    "    ADD A, B\n" +
    "    STORE A, 50\n" +
    "    LOAD C, 5\n" +
    "    SUB A, C\n" +
    "    STORE A, 51\n" +
    "    HALT\n";
  loadExamplesList().then(() => loadExample("addition.asm").then(refreshAll));
  refreshAll();
  setStatus("Ready. Click Assemble & Load, then Step/Run.");
}

init();
