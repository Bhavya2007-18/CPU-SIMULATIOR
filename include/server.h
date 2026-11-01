#ifndef CPU_SIM_SERVER_H
#define CPU_SIM_SERVER_H

#include <string>

namespace cpusim {

// Run a local single-user HTTP server that:
//   - serves the static frontend from `webRoot` (index.html, app.js, ...)
//   - exposes a JSON REST API wrapping the existing Debugger/CPU/Assembler
// Blocks until the process is interrupted. Returns a non-zero value on a fatal
// socket error. No third-party dependencies (raw sockets + hand-rolled JSON).
int runServer(int port, const std::string& webRoot);

} // namespace cpusim

#endif // CPU_SIM_SERVER_H
