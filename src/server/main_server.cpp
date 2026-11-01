#include "server.h"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    int port = 8080;
    std::string webRoot = "frontend";

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "--port" || a == "-p") && i + 1 < argc)
            port = std::stoi(argv[++i]);
        else if ((a == "--web" || a == "-w") && i + 1 < argc)
            webRoot = argv[++i];
        else if (a == "--help" || a == "-h") {
            std::cout <<
                "Usage: cpu-server [--port N] [--web DIR]\n"
                "  --port N   port to listen on (default 8080)\n"
                "  --web DIR  directory with index.html/app.js (default frontend)\n";
            return 0;
        }
    }
    return cpusim::runServer(port, webRoot);
}
