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
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
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

// ---------------------------------------------------------------------------
// Sessions: one Debugger (private CPU) per browser, keyed by a cookie id.
// Single-threaded request handling means no locking is needed; idle sessions
// are garbage-collected after SESSION_TTL_SEC.
// ---------------------------------------------------------------------------
struct Session {
    Debugger dbg;
    bool loaded = false;
    std::time_t lastUsed = 0;
};

std::unordered_map<std::string, Session> g_sessions;
std::string g_setCookie;  // pending Set-Cookie header for the current response
constexpr long SESSION_TTL_SEC = 30 * 60;

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
    std::string out; out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8]; std::snprintf(buf, sizeof(buf), "\\u%04x", c); out += buf;
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

std::string traceToJson(const Debugger& dbg, std::size_t maxN) {
    const auto& entries = dbg.cpu().trace().entries();
    std::ostringstream o; o << "[";
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

std::string listingToJson(const Debugger& dbg) {
    std::size_t programBytes = dbg.programSize();
    Decoder dec;
    Address pc = PROGRAM_ORIGIN;
    std::size_t consumed = 0;
    std::ostringstream o; o << "[";
    bool first = true;
    while (consumed < programBytes) {
        Instruction ins{};
        try { ins = dec.decode(dbg.cpu().memory(), pc); } catch (...) { break; }
        if (!first) o << ",";
        first = false;
        o << "{\"addr\":" << static_cast<unsigned>(pc)
          << ",\"length\":" << ins.length
          << ",\"text\":" << jsonStr(formatInstruction(ins))
          << ",\"bytes\":[";
        for (std::size_t b = 0; b < ins.length; ++b) {
            if (b) o << ",";
            o << static_cast<unsigned>(dbg.cpu().memory().read(
                static_cast<Address>(static_cast<std::size_t>(pc) + b)));
        }
        o << "]}";
        consumed += ins.length;
        pc = static_cast<Address>(static_cast<std::size_t>(pc) + ins.length);
    }
    o << "]";
    return o.str();
}

std::size_t countInstructions(const Debugger& dbg) {
    std::size_t programBytes = dbg.programSize();
    Decoder dec; Address pc = PROGRAM_ORIGIN; std::size_t consumed = 0, count = 0;
    while (consumed < programBytes) {
        try { Instruction ins = dec.decode(dbg.cpu().memory(), pc);
              consumed += ins.length;
              pc = static_cast<Address>(static_cast<std::size_t>(pc) + ins.length);
              ++count; }
        catch (...) { break; }
    }
    return count;
}

std::string memToJson(const Debugger& dbg, Address start, std::size_t count) {
    if (start >= MEMORY_SIZE) return "{\"start\":0,\"count\":0,\"bytes\":[]}";
    if (count > static_cast<std::size_t>(MEMORY_SIZE) - start)
        count = static_cast<std::size_t>(MEMORY_SIZE) - start;
    std::ostringstream o;
    o << "{\"start\":" << static_cast<unsigned>(start) << ",\"count\":" << count << ",\"bytes\":[";
    for (std::size_t i = 0; i < count; ++i) {
        if (i) o << ",";
        o << static_cast<unsigned>(dbg.cpu().memory().read(
            static_cast<Address>(static_cast<std::size_t>(start) + i)));
    }
    o << "]}";
    return o.str();
}

std::string breakpointsToJson(const Debugger& dbg) {
    std::ostringstream o; o << "[";
    bool first = true;
    for (unsigned a = 0; a < MEMORY_SIZE; ++a) {
        if (dbg.isBreakpoint(static_cast<Address>(a))) {
            if (!first) o << ",";
            first = false;
            o << a;
        }
    }
    o << "]";
    return o.str();
}

std::string okState(const Debugger& dbg) {
    CPUState s = dbg.cpu().getState();
    std::ostringstream o;
    o << "{\"ok\":true,\"halted\":" << (s.halted ? 1 : 0)
      << ",\"cycle\":" << dbg.cpu().trace().size()
      << ",\"lastInstruction\":" << jsonStr(formatInstruction(dbg.cpu().lastInstruction()))
      << ",\"state\":" << stateToJson(s) << "}";
    return o.str();
}

std::string errJson(const std::string& msg) {
    return "{\"ok\":false,\"error\":" + jsonStr(msg) + "}";
}

// ----- query parsing -------------------------------------------------------

std::unordered_map<std::string, std::string> parseQuery(const std::string& q) {
    std::unordered_map<std::string, std::string> m;
    std::size_t pos = 0;
    while (pos < q.size()) {
        auto amp = q.find('&', pos);
        std::string pair = (amp == std::string::npos) ? q.substr(pos) : q.substr(pos, amp - pos);
        auto eq = pair.find('=');
        if (eq != std::string::npos) m[pair.substr(0, eq)] = pair.substr(eq + 1);
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

// ----- session management --------------------------------------------------

std::string genSid() {
    static std::mt19937_64 rng(std::random_device{}());
    std::ostringstream o; o << std::hex << rng() << rng();
    return o.str();
}

std::string parseCookieSid(const std::string& cookie) {
    std::string key = "cpusim_sid=";
    auto p = cookie.find(key);
    if (p == std::string::npos) return "";
    p += key.size();
    auto e = cookie.find(';', p);
    return trim(cookie.substr(p, (e == std::string::npos) ? std::string::npos : e - p));
}

void sweepExpired() {
    std::time_t now = std::time(nullptr);
    for (auto it = g_sessions.begin(); it != g_sessions.end();) {
        if (now - it->second.lastUsed > SESSION_TTL_SEC) it = g_sessions.erase(it);
        else ++it;
    }
}

Session& getOrCreateSession(const std::string& cookie, std::string& sidOut, bool& isNew) {
    sweepExpired();
    std::string sid = parseCookieSid(cookie);
    auto it = g_sessions.find(sid);
    if (it != g_sessions.end()) {
        it->second.lastUsed = std::time(nullptr);
        isNew = false; sidOut = sid; return it->second;
    }
    sid = genSid();
    Session& s = g_sessions[sid];
    s.lastUsed = std::time(nullptr); s.loaded = false;
    isNew = true; sidOut = sid; return s;
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

// Read the full HTTP request (headers + body up to Content-Length), and the
// Cookie header if present.
bool readRequest(sock_t s, std::string& method, std::string& path,
                 std::string& query, std::string& body, std::string& cookie) {
    std::string buf;
    char tmp[4096];
    std::size_t headerEnd = std::string::npos;
    while (headerEnd == std::string::npos) {
        int n = recv(s, tmp, sizeof(tmp), 0);
        if (n <= 0) return false;
        buf.append(tmp, static_cast<std::size_t>(n));
        headerEnd = buf.find("\r\n\r\n");
        if (buf.size() > (1u << 20)) return false;
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
    if (q != std::string::npos) { path = target.substr(0, q); query = target.substr(q + 1); }
    else { path = target; }

    std::string lower = toLower(headers);
    // Content-Length
    std::size_t cl = 0;
    {
        auto p = lower.find("content-length:");
        if (p != std::string::npos) {
            auto e = lower.find("\r\n", p);
            try { cl = static_cast<std::size_t>(std::stoul(trim(lower.substr(p + 15, e - (p + 15))))); }
            catch (...) { cl = 0; }
        }
    }
    // Cookie (match a header line named "cookie:", not "set-cookie:")
    {
        auto p = lower.find("\r\ncookie:");
        if (p != std::string::npos) p += 2;
        else if (lower.rfind("cookie:", 0) == 0) p = 0;
        else p = std::string::npos;
        if (p != std::string::npos) {
            auto e = lower.find("\r\n", p);
            cookie = headers.substr(p + 7, (e == std::string::npos) ? std::string::npos : e - (p + 7));
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
      << "Content-Length: " << body.size() << "\r\n";
    if (!g_setCookie.empty()) o << "Set-Cookie: " << g_setCookie << "\r\n";
    o << "Connection: close\r\n"
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

std::string readFile(const std::string& p, bool& ok) {
    std::ifstream in(p, std::ios::binary);
    if (!in) { ok = false; return ""; }
    std::ostringstream ss; ss << in.rdbuf(); ok = true;
    return ss.str();
}

// ----- API handlers --------------------------------------------------------

void handleApi(sock_t s, const std::string& method, const std::string& path,
               const std::string& queryStr, const std::string& body, Session& sess) {
    Debugger& dbg = sess.dbg;
    auto q = parseQuery(queryStr);

    if (path == "/api/assemble" && method == "POST") {
        if (!dbg.loadSource(body))
            sendResponse(s, 200, "application/json",
                "{\"ok\":false,\"error\":" + jsonStr(dbg.lastError()) + "}");
        else {
            sess.loaded = true;
            std::ostringstream o;
            o << "{\"ok\":true,\"bytes\":" << dbg.programSize()
              << ",\"instructions\":" << countInstructions(dbg)
              << ",\"listing\":" << listingToJson(dbg) << "}";
            sendResponse(s, 200, "application/json", o.str());
        }
        return;
    }
    if (path == "/api/step" && method == "POST") {
        dbg.step();
        if (!dbg.lastError().empty())
            sendResponse(s, 200, "application/json",
                "{\"ok\":false,\"error\":" + jsonStr(dbg.lastError()) +
                ",\"state\":" + stateToJson(dbg.cpu().getState()) + "}");
        else
            sendResponse(s, 200, "application/json", okState(dbg));
        return;
    }
    if (path == "/api/run" && method == "POST") {
        std::size_t n = dbg.run();
        bool err = !dbg.lastError().empty();
        const char* stoppedAt = "limit";
        if (err) stoppedAt = "error";
        else if (dbg.cpu().halted()) stoppedAt = "halt";
        else if (dbg.isBreakpoint(dbg.cpu().getState().PC)) stoppedAt = "breakpoint";
        CPUState st = dbg.cpu().getState();
        std::ostringstream o;
        o << "{\"ok\":" << (err ? 0 : 1) << ",\"executed\":" << n
          << ",\"stoppedAt\":" << jsonStr(stoppedAt)
          << ",\"error\":" << (err ? jsonStr(dbg.lastError()) : "null")
          << ",\"cycle\":" << dbg.cpu().trace().size()
          << ",\"halted\":" << (st.halted ? 1 : 0)
          << ",\"lastInstruction\":" << jsonStr(formatInstruction(dbg.cpu().lastInstruction()))
          << ",\"state\":" << stateToJson(st) << "}";
        sendResponse(s, 200, "application/json", o.str());
        return;
    }
    if (path == "/api/reset" && method == "POST") {
        dbg.reset();
        sendResponse(s, 200, "application/json", okState(dbg));
        return;
    }
    if (path == "/api/state" && method == "GET") {
        sendResponse(s, 200, "application/json", okState(dbg));
        return;
    }
    if (path == "/api/memory" && method == "GET") {
        int start = getInt(q, "start", 0); int count = getInt(q, "count", 256);
        if (start < 0) start = 0;
        if (count < 0) count = 0;
        sendResponse(s, 200, "application/json",
            memToJson(dbg, static_cast<Address>(start), static_cast<std::size_t>(count)));
        return;
    }
    if (path == "/api/trace" && method == "GET") {
        std::size_t n = static_cast<std::size_t>(getInt(q, "count", 50));
        sendResponse(s, 200, "application/json", "{\"trace\":" + traceToJson(dbg, n) + "}");
        return;
    }
    if (path == "/api/breakpoints" && method == "GET") {
        sendResponse(s, 200, "application/json", "{\"breakpoints\":" + breakpointsToJson(dbg) + "}");
        return;
    }
    if (path == "/api/break" && method == "POST") {
        int a = getInt(q, "addr", -1);
        if (a >= 0 && static_cast<std::size_t>(a) < MEMORY_SIZE) dbg.setBreakpoint(static_cast<Address>(a));
        sendResponse(s, 200, "application/json", "{\"breakpoints\":" + breakpointsToJson(dbg) + "}");
        return;
    }
    if (path == "/api/unbreak" && method == "POST") {
        int a = getInt(q, "addr", -1);
        if (a >= 0 && static_cast<std::size_t>(a) < MEMORY_SIZE) dbg.clearBreakpoint(static_cast<Address>(a));
        sendResponse(s, 200, "application/json", "{\"breakpoints\":" + breakpointsToJson(dbg) + "}");
        return;
    }
    if (path == "/api/examples" && method == "GET") {
        std::ostringstream o; o << "{\"files\":[";
        bool first = true;
        try {
            for (auto& e : std::filesystem::directory_iterator("examples")) {
                if (!e.is_regular_file() || e.path().extension() != ".asm") continue;
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
        bool ok = false; std::string content = readFile("examples/" + name, ok);
        if (!ok) { sendResponse(s, 200, "application/json", errJson("not found")); return; }
        sendResponse(s, 200, "application/json",
            "{\"ok\":true,\"name\":" + jsonStr(name) + ",\"source\":" + jsonStr(content) + "}");
        return;
    }
    sendResponse(s, 404, "application/json", errJson("unknown endpoint: " + method + " " + path));
}

// ----- static file handling ------------------------------------------------

void handleStatic(sock_t s, const std::string& reqPath, const std::string& webRoot) {
    std::string rel = reqPath;
    if (rel.empty() || rel == "/") rel = "/index.html";
    if (rel[0] == '/') rel = rel.substr(1);
    if (rel.find("..") != std::string::npos) { sendResponse(s, 404, "text/plain", "forbidden"); return; }
    std::string full = webRoot;
    if (!full.empty() && full.back() != '/' && full.back() != '\\') full += "/";
    full += rel;
    bool ok = false; std::string content = readFile(full, ok);
    if (!ok) {
        content = readFile(webRoot + "/index.html", ok);
        if (!ok) { sendResponse(s, 404, "text/plain", "not found: " + rel); return; }
        sendResponse(s, 200, "text/html; charset=utf-8", content);
        return;
    }
    sendResponse(s, 200, mimeFor(rel), content);
}

void handleClient(sock_t s, const std::string& webRoot) {
    std::string method, path, query, body, cookie;
    if (!readRequest(s, method, path, query, body, cookie)) return;
    if (path.rfind("/api/", 0) == 0) {
        std::string sid; bool isNew = false;
        Session& sess = getOrCreateSession(cookie, sid, isNew);
        g_setCookie = isNew ? ("cpusim_sid=" + sid + "; Path=/; HttpOnly; SameSite=Lax") : "";
        handleApi(s, method, path, query, body, sess);
        g_setCookie.clear();
    } else {
        handleStatic(s, path, webRoot);
    }
}

} // namespace

int runServer(int port, const std::string& webRoot) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { std::cerr << "WSAStartup failed\n"; return 1; }
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
        closeSocket(srv); return 1;
    }
    if (listen(srv, 16) < 0) { std::cerr << "listen() failed\n"; closeSocket(srv); return 1; }
    std::cout << "CPU Simulator server listening on http://localhost:" << port
              << "  (web root: " << webRoot << ")\n"
              << "Multi-user: each browser gets its own CPU session.\n"
              << "Open the URL in a browser. Ctrl+C to stop.\n";
    for (;;) {
        sock_t cli = accept(srv, nullptr, nullptr);
        if (cli == BAD_SOCK) continue;
        handleClient(cli, webRoot);
        closeSocket(cli);
    }
}

} // namespace cpusim
