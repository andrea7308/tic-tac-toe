# TIC TAC TOE
### This game is intended to simulate tic tac toe.
## Requirements
 Two players take alternate turns, identified by markers ‘X’ and ‘O’.
 The game is played on a 3x3 grid.
 The system should be modular and extensible to support future features.
 The game should reject invalid moves and notify the player.
 The game should declare a draw if all cells are full and no player has won.
 The game should detect and announce the winner.

## Run the web game

The frontend is served by the C++ game server and does not contain a local game implementation. The server uses the existing `Game` class for move validation and game outcomes.

Requirements: CMake 3.20+, a C++17 compiler, and Boost 1.75+ with the JSON component.

```sh
cmake -S . -B build/server
cmake --build build/server --target tic-tac-toe-server
./build/server/tic-tac-toe-server 0.0.0.0 8080 frontend
```

Open `http://localhost:8080`, create a game, and share its join code with the other player. For players on different networks, deploy the server on a publicly reachable host behind HTTPS and use the same server for both clients. The current service stores sessions in memory, so games do not survive server restarts.

See [frontend/API.md](frontend/API.md) for the HTTP contract and deployment boundaries.
+ Two players take alternate turns, identified by markers ‘X’ and ‘O’.
+ The game is played on a 3x3 grid.
+ The system should be modular and extensible to support future features.
+ The game should reject invalid moves and notify the player.
+ The game should declare a draw if all cells are full and no player has won.
+ The game should detect and announce the winner.
