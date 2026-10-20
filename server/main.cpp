#include "http_server.h"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    const std::string address = argc > 1 ? argv[1] : "0.0.0.0";
    const int port = argc > 2 ? std::stoi(argv[2]) : 8080;
    const std::string frontendDirectory = argc > 3 ? argv[3] : "frontend";

    if (port < 1 || port > 65535) {
        std::cerr << "Port must be between 1 and 65535" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        GameService games;
        HttpServer server(games, address, static_cast<std::uint16_t>(port), frontendDirectory);
        server.run();
    } catch (const std::exception& error) {
        std::cerr << "Server failed: " << error.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}