#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <cstdint>
#include <string>

#include "game_service.h"

class HttpServer {
    GameService& gameService_;
    std::string address_;
    std::uint16_t port_;
    std::string frontendDirectory_;

public:
    HttpServer(GameService& gameService, std::string address, std::uint16_t port, std::string frontendDirectory);
    void run();
};

#endif