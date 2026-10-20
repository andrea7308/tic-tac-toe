cmake_minimum_required(VERSION 3.20)
project(tic_tac_toe LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost 1.75 REQUIRED COMPONENTS json)
find_package(Threads REQUIRED)

set(GAME_SOURCES
    game.cpp
    entities/board.cpp
    entities/cell.cpp
    entities/player.cpp
    enums/status.cpp
    enums/symbol.cpp
    exceptions/InvalidMoveException.cpp
)

add_executable(tic-tac-toe main.cpp ${GAME_SOURCES})
target_link_libraries(tic-tac-toe PRIVATE Threads::Threads)

add_executable(tic-tac-toe-server
    server/main.cpp
    server/game_service.cpp
    server/http_server.cpp
    ${GAME_SOURCES}
)
target_link_libraries(tic-tac-toe-server PRIVATE Boost::json Threads::Threads)