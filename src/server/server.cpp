#include "server.h"

#include "debugger.h"
#include "execution_trace.h"
#include "decoder.h"
#include "instruction.h"
#include "isa.h"
#include "memory.h"
#include "types.h"

#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// ---- Raw sockets: Winsock2 on Windows, POSIX elsewhere -------------------
#ifdef _WIN32
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  using sock_t = SOCKET;
  #define BAD_SOCK INVALID_SOCKET
#else
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <unistd.h>
  using sock_t = int;
  #define BAD_SOCK (-1)
#endif

namespace cpusim {

namespace {

// The single interactive session. Local single-user tool, so a global is fine.
Debugger g_dbg;
bool g_loaded = false;

// ----- small string / JSON helpers ----------------------------------------

std::string trim(const std::string& s) {
    auto a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    auto b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

std::string toLower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string jsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else out += c;
        }
    }
    return out;
}

std::string jsonStr(const std::string& s) { return "\"" + jsonEscape(s) + "\""; }

std::string stateToJson(const CPUState& s) {
    std::ostringstream o;
    o << "{\"A\":" << static_cast<unsigned>(s.A)
      << ",\"B\":" << static_cast<unsigned>(s.B)
      << ",\"C\":" << static_cast<unsigned>(s.C)
      << ",\"D\":" << static_cast<unsigned>(s.D)
      << ",\"PC\":" << static_cast<unsigned>(s.PC)
      << ",\"IR\":" << static_cast<unsigned>(s.IR)
      << ",\"SP\":" << static_cast<unsigned>(s.SP)
      << ",\"Z\":" << (s.zero ? 1 : 0)
      << ",\"C\":" << (s.carry ? 1 : 0)
      << ",\"S\":" << (s.sign ? 1 : 0)
      << ",\"V\":" << (s.overflow ? 1 : 0)
      << ",\"halted\":" << (s.halted ? 1 : 0) << "}";
    return o.str();
}

std::string traceToJson(std::size_t maxN) {
    const auto& entries = g_dbg.cpu().trace().entries();
    std::ostringstream o;
    o << "[";
    bool first = true;
    std::size_t start = (maxN == 0 || maxN >= entries.size()) ? 0 : entries.size() - maxN;
    for (std::size_t i = start; i < entries.size(); ++i) {
        const TraceEntry& e = entries[i];
        if (!first) o << ",";
        first = false;
        o << "{\"cycle\":" << e.cycle
          << ",\"pcBefore\":" << static_cast<unsigned>(e.pcBefore)
          << ",\"pcAfter\":" << static_cast<unsigned>(e.stateAfter.PC)
          << ",\"instruction\":" << jsonStr(formatInstruction(e.instruction))
          << ",\"before\":" << stateToJson(e.stateBefore)
          << ",\"after\":" << stateToJson(e.stateAfter) << "}";
    }
    o << "]";
    return o.str();
}

// Decode the loaded program into a per-instruction listing for the UI.
std::string listingToJson() {
    std::size_t programBytes = g_dbg.programSize();
    Decoder dec;
    Address pc = PROGRAM_ORIGIN;
    std::size_t consumed = 0;
    std::ostringstream o;
    o << "[";
    bool first = true;
    while (consumed < programBytes) {
        Instruction ins{};
        try {
            ins = dec.decode(g_dbg.cpu().memory(), pc);
        } catch (...) {
            break;  // defensive; assembled programs should decode cleanly
        }
        if (!first) o << ",";
        first = false;
        o << "{\"addr\":" << static_cast<unsigned>(pc)
          << ",\"length\":" << ins.length
          << ",\"text\":" << jsonStr(formatInstruction(ins))
          << ",\"bytes\":[";
        for (std::size_t b = 0; b < ins.length; ++b) {
            if (b) o << ",";
            o << static_cast<unsigned>(g_dbg.cpu().memory().read(
                static_cast<Address>(static_cast<std::size_t>(pc) + b)));
        }
        o << "]}";
        consumed += ins.length;
        pc = static_cast<Address>(static_cast<std::size_t>(pc) + ins.length);
    }
    o << "]";
    return o.str();
}

std::size_t countInstructions() {
    std::size_t programBytes = g_dbg.programSize();
    Decoder dec;
    Address pc = PROGRAM_ORIGIN;
    std::size_t consumed = 0, count = 0;
    while (consumed < programBytes) {
        try { Instruction ins = dec.decode(g_dbg.cpu().memory(), pc);
              consumed += ins.length;
              pc = static_cast<Address>(static_cast<std::size_t>(pc) + ins.length);
              ++count; }
        catch (...) { break; }
    }
    return count;
}

std::string memToJson(Address start, std::size_t count) {
    if (start >= MEMORY_SIZE) return "{\"start\":0,\"count\":0,\"bytes\":[]}";
    if (count > static_cast<std::size_t>(MEMORY_SIZE) - start)
        count = static_cast<std::size_t>(MEMORY_SIZE) - start;
    std::ostringstream o;
    o << "{\"start\":" << static_cast<unsigned>(start)
      << ",\"count\":" << count << ",\"bytes\":[";
    for (std::size_t i = 0; i < count; ++i) {
        if (i) o << ",";
        o << static_cast<unsigned>(g_dbg.cpu().memory().read(
            static_cast<Address>(static_cast<std::size_t>(start) + i)));
    }
    o << "]}";
    return o.str();
}

std::string breakpointsToJson() {
    std::ostringstream o;
    o << "[";
    bool first = true;
    for (unsigned a = 0; a < MEMORY_SIZE; ++a) {
        if (g_dbg.isBreakpoint(static_cast<Address>(a))) {
            if (!first) o << ",";
            first = false;
            o << a;
        }
    }
    o << "]";
    return o.str();
}

std::string okState() {
    CPUState s = g_dbg.cpu().getState();
    std::ostringstream o;
    o << "{\"ok\":true,\"halted\":" << (s.halted ? 1 : 0)
      << ",\"cycle\":" << g_dbg.cpu().trace().size()
      << ",\"lastInstruction\":" << jsonStr(formatInstruction(g_dbg.cpu().lastInstruction()))
      << ",\"state\":" << stateToJson(s) << "}";
    return o.str();
}

std::string errJson(const std::string& msg) {
    return "{\"ok\":false,\"error\":" + jsonStr(msg) + "}";
}

// ----- query string parsing ------------------------------------------------

struct Query {
    std::unordered_map<std::string, std::string> kv; // need <unordered_map>
};

// Minimal query parse (key=value&key=value).
std::unordered_map<std::string, std::string> parseQuery(const std::string& q) {
    std::unordered_map<std::string, std::string> m;
    std::size_t pos = 0;
    while (pos < q.size()) {
        auto amp = q.find('&', pos);
        std::string pair = (amp == std::string::npos) ? q.substr(pos) : q.substr(pos, amp - pos);
        auto eq = pair.find('=');
        if (eq != std::string::npos)
            m[pair.substr(0, eq)] = pair.substr(eq + 1);
        if (amp == std::string::npos) break;
        pos = amp + 1;
    }
    return m;
}

int getInt(const std::unordered_map<std::string, std::string>& q, const std::string& k, int def) {
    auto it = q.find(k);
    if (it == q.end()) return def;
    try { return std::stoi(it->second); } catch (...) { return def; }
}

// ----- socket I/O ----------------------------------------------------------

bool sendAll(sock_t s, const std::string& data) {
    std::size_t total = 0;
    while (total < data.size()) {
        int n = static_cast<int>(send(s, data.data() + total,
            static_cast<int>(data.size() - total), 0));
        if (n <= 0) return false;
        total += static_cast<std::size_t>(n);
    }
    return true;
}

void closeSocket(sock_t s) {
#ifdef _WIN32
    closesocket(s);
#else
    close(s);
#endif
}

// Read the full HTTP request (headers + body up to Content-Length).
bool readRequest(sock_t s, std::string& method, std::string& path,
                 std::string& query, std::string& body) {
    std::string buf;
    char tmp[4096];
    std::size_t headerEnd = std::string::npos;
    while (headerEnd == std::string::npos) {
        int n = recv(s, tmp, sizeof(tmp), 0);
        if (n <= 0) return false;
        buf.append(tmp, static_cast<std::size_t>(n));
        headerEnd = buf.find("\r\n\r\n");
        if (buf.size() > (1u << 20)) return false;  // refuse oversized headers
    }
    std::string headers = buf.substr(0, headerEnd);
    std::string rest = buf.substr(headerEnd + 4);

    auto eol = headers.find("\r\n");
    std::string requestLine = (eol == std::string::npos) ? headers : headers.substr(0, eol);
    auto sp1 = requestLine.find(' ');
    auto sp2 = requestLine.find(' ', sp1 == std::string::npos ? 0 : sp1 + 1);
    if (sp1 == std::string::npos || sp2 == std::string::npos) return false;
    method = requestLine.substr(0, sp1);
    std::string target = requestLine.substr(sp1 + 1, sp2 - sp1 - 1);

    auto q = target.find('?');
    if (q != std::string::npos) {
        path = target.substr(0, q);
        query = target.substr(q + 1);
    } else {
        path = target;
    }

    std::string lower = toLower(headers);
    std::size_t cl = 0;
    {
        auto p = lower.find("content-length:");
        if (p != std::string::npos) {
            auto e = lower.find("\r\n", p);
            std::string val = lower.substr(p + 15, e - (p + 15));
            try { cl = static_cast<std::size_t>(std::stoul(trim(val))); } catch (...) { cl = 0; }
        }
    }
    body = rest;
    while (body.size() < cl) {
        int n = recv(s, tmp, sizeof(tmp), 0);
        if (n <= 0) break;
        body.append(tmp, static_cast<std::size_t>(n));
    }
    if (cl > 0 && body.size() > cl) body.resize(cl);
    return true;
}

void sendResponse(sock_t s, int code, const std::string& type, const std::string& body) {
    std::ostringstream o;
    o << "HTTP/1.1 " << code << (code == 200 ? " OK" : "") << "\r\n"
      << "Content-Type: " << type << "\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Connection: close\r\n"
      << "Access-Control-Allow-Origin: *\r\n"
      << "Cache-Control: no-store\r\n\r\n";
    sendAll(s, o.str() + body);
}

std::string mimeFor(const std::string& path) {
    auto dot = path.find_last_of('.');
    std::string ext = (dot == std::string::npos) ? "" : toLower(path.substr(dot + 1));
    if (ext == "html" || ext == "htm") return "text/html; charset=utf-8";
    if (ext == "js")   return "application/javascript; charset=utf-8";
    if (ext == "css")  return "text/css; charset=utf-8";
    if (ext == "json") return "application/json; charset=utf-8";
    if (ext == "svg")  return "image/svg+xml";
    if (ext == "png")  return "image/png";
    if (ext == "ico")  return "image/x-icon";
    return "application/octet-stream";
}

// ----- static file serving -------------------------------------------------

std::string readFile(const std::string& p, bool& ok) {
    std::ifstream in(p, std::ios::binary);
    if (!in) { ok = false; return ""; }
    std::ostringstream ss; ss << in.rdbuf();
    ok = true;
    return ss.str();
}

// ----- API handlers --------------------------------------------------------

void handleApi(sock_t s, const std::string& method, const std::string& path,
               const std::string& queryStr, const std::string& body) {
    auto q = parseQuery(queryStr);

    if (path == "/api/assemble" && method == "POST") {
        if (!g_dbg.loadSource(body))
            sendResponse(s, 200, "application/json",
                "{\"ok\":false,\"error\":" + jsonStr(g_dbg.lastError()) + "}");
        else {
            g_loaded = true;
            std::ostringstream o;
            o << "{\"ok\":true,\"bytes\":" << g_dbg.programSize()
              << ",\"instructions\":" << countInstructions()
              << ",\"listing\":" << listingToJson() << "}";
            sendResponse(s, 200, "application/json", o.str());
        }
        return;
    }
    if (path == "/api/step" && method == "POST") {
        g_dbg.step();  // no-op if halted; sets lastError on a fault
        bool err = !g_dbg.lastError().empty();
        if (err)
            sendResponse(s, 200, "application/json",
                "{\"ok\":false,\"error\":" + jsonStr(g_dbg.lastError()) +
                ",\"state\":" + stateToJson(g_dbg.cpu().getState()) + "}");
        else
            sendResponse(s, 200, "application/json", okState());
        return;
    }
    if (path == "/api/run" && method == "POST") {
        std::size_t n = g_dbg.run();
        bool err = !g_dbg.lastError().empty();
        const char* stoppedAt = "limit";
        if (err) stoppedAt = "error";
        else if (g_dbg.cpu().halted()) stoppedAt = "halt";
        else if (g_dbg.isBreakpoint(g_dbg.cpu().getState().PC)) stoppedAt = "breakpoint";
        CPUState st = g_dbg.cpu().getState();
        std::ostringstream o;
        o << "{\"ok\":" << (err ? 0 : 1)
          << ",\"executed\":" << n
          << ",\"stoppedAt\":" << jsonStr(stoppedAt)
          << ",\"error\":" << (err ? jsonStr(g_dbg.lastError()) : "null")
          << ",\"cycle\":" << g_dbg.cpu().trace().size()
          << ",\"halted\":" << (st.halted ? 1 : 0)
          << ",\"lastInstruction\":" << jsonStr(formatInstruction(g_dbg.cpu().lastInstruction()))
          << ",\"state\":" << stateToJson(st) << "}";
        sendResponse(s, 200, "application/json", o.str());
        return;
    }
    if (path == "/api/reset" && method == "POST") {
        g_dbg.reset();
        sendResponse(s, 200, "application/json", okState());
        return;
    }
    if (path == "/api/state" && method == "GET") {
        sendResponse(s, 200, "application/json", okState());
        return;
    }
    if (path == "/api/memory" && method == "GET") {
        int start = getInt(q, "start", 0);
        int count = getInt(q, "count", 256);
        if (start < 0) start = 0;
        if (count < 0) count = 0;
        sendResponse(s, 200, "application/json",
            memToJson(static_cast<Address>(start), static_cast<std::size_t>(count)));
        return;
    }
    if (path == "/api/trace" && method == "GET") {
        std::size_t n = static_cast<std::size_t>(getInt(q, "count", 50));
        sendResponse(s, 200, "application/json", "{\"trace\":" + traceToJson(n) + "}");
        return;
    }
    if (path == "/api/breakpoints" && method == "GET") {
        sendResponse(s, 200, "application/json", "{\"breakpoints\":" + breakpointsToJson() + "}");
        return;
    }
    if (path == "/api/break" && method == "POST") {
        int a = getInt(q, "addr", -1);
        if (a >= 0 && static_cast<std::size_t>(a) < MEMORY_SIZE) g_dbg.setBreakpoint(static_cast<Address>(a));
        sendResponse(s, 200, "application/json", "{\"breakpoints\":" + breakpointsToJson() + "}");
        return;
    }
    if (path == "/api/unbreak" && method == "POST") {
        int a = getInt(q, "addr", -1);
        if (a >= 0 && static_cast<std::size_t>(a) < MEMORY_SIZE) g_dbg.clearBreakpoint(static_cast<Address>(a));
        sendResponse(s, 200, "application/json", "{\"breakpoints\":" + breakpointsToJson() + "}");
        return;
    }
    if (path == "/api/examples" && method == "GET") {
        std::ostringstream o; o << "{\"files\":[";
        bool first = true;
        try {
            for (auto& e : std::filesystem::directory_iterator("examples")) {
                if (!e.is_regular_file()) continue;
                if (e.path().extension() != ".asm") continue;
                if (!first) o << ",";
                first = false;
                o << jsonStr(e.path().filename().string());
            }
        } catch (...) {}
        o << "]}";
        sendResponse(s, 200, "application/json", o.str());
        return;
    }
    if (path == "/api/example" && method == "GET") {
        std::string name = q.count("name") ? q["name"] : "";
        if (name.empty() || name.find('/') != std::string::npos ||
            name.find('\\') != std::string::npos || name.find("..") != std::string::npos) {
            sendResponse(s, 200, "application/json", errJson("invalid example name"));
            return;
        }
        bool ok = false;
        std::string content = readFile("examples/" + name, ok);
        if (!ok) { sendResponse(s, 200, "application/json", errJson("not found")); return; }
        sendResponse(s, 200, "application/json",
            "{\"ok\":true,\"name\":" + jsonStr(name) + ",\"source\":" + jsonStr(content) + "}");
        return;
    }

    sendResponse(s, 404, "application/json", errJson("unknown endpoint: " + method + " " + path));
}

// ----- static file handling with path-traversal guard ----------------------

void handleStatic(sock_t s, const std::string& reqPath, const std::string& webRoot) {
    std::string rel = reqPath;
    if (rel.empty() || rel == "/") rel = "/index.html";
    if (rel[0] == '/') rel = rel.substr(1);
    if (rel.find("..") != std::string::npos) { sendResponse(s, 404, "text/plain", "forbidden"); return; }
    std::string full = webRoot;
    if (!full.empty() && full.back() != '/' && full.back() != '\\') full += "/";
    full += rel;
    bool ok = false;
    std::string content = readFile(full, ok);
    if (!ok) {
        // fall back to index.html for client-side routing (single page)
        content = readFile(webRoot + "/index.html", ok);
        if (!ok) { sendResponse(s, 404, "text/plain", "not found: " + rel); return; }
        sendResponse(s, 200, "text/html; charset=utf-8", content);
        return;
    }
    sendResponse(s, 200, mimeFor(rel), content);
}

void handleClient(sock_t s, const std::string& webRoot) {
    std::string method, path, query, body;
    if (!readRequest(s, method, path, query, body)) return;
    if (path.rfind("/api/", 0) == 0)
        handleApi(s, method, path, query, body);
    else
        handleStatic(s, path, webRoot);
}

} // namespace

int runServer(int port, const std::string& webRoot) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
#endif
    sock_t srv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (srv == BAD_SOCK) { std::cerr << "socket() failed\n"; return 1; }

    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<unsigned short>(port));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(srv, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "bind() failed on port " << port << "\n";
        closeSocket(srv);
        return 1;
    }
    if (listen(srv, 16) < 0) {
        std::cerr << "listen() failed\n";
        closeSocket(srv);
        return 1;
    }

    std::cout << "CPU Simulator server listening on http://localhost:" << port
              << "  (web root: " << webRoot << ")\n"
              << "Open the URL in a browser. Ctrl+C to stop.\n";

    for (;;) {
        sock_t cli = accept(srv, nullptr, nullptr);
        if (cli == BAD_SOCK) continue;
        handleClient(cli, webRoot);
        closeSocket(cli);
    }
}

} // namespace cpusim
