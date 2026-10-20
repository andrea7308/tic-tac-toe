#include <memory>
#include <iostream>
#include "entities/player.h"
#include "game.h"

int main() {
    auto alice = std::make_shared<Player>("Alice", Symbol::X);
    auto bob = std::make_shared<Player>("Bob", Symbol::O);

    Game game(alice, bob, 3);

    std::cout << "========== TIC TAC TOE ==========\n";

    // Alice (X) completes the top row and wins
    game.makeMove(0, 0);  // X at (0,0)
    game.makeMove(1, 0);  // O at (1,0)
    game.makeMove(0, 1);  // X at (0,1)
    game.makeMove(1, 1);  // O at (1,1)
    game.makeMove(0, 2);  // X at (0,2) - Alice wins!

    game.printBoard();

    std::cout << "Result: ";
    switch (game.getStatus()) {
        case GameStatus::WINNER_X: std::cout << "WINNER_X\n"; break;
        case GameStatus::WINNER_O: std::cout << "WINNER_O\n"; break;
        case GameStatus::DRAW: std::cout << "DRAW\n"; break;
        default: std::cout << "IN_PROGRESS\n";
    }

    auto winner = game.getWinner();
    if (winner != nullptr) {
        std::cout << "Winner: " << winner->getName() << "\n";
    }

    return 0;
}