#ifndef GAME_H
#define GAME_H
#include "entities/board.h"
#include "enums/status.h"

class Game {
    std::unique_ptr<Board> board_;
    std::array<std::shared_ptr<Player>, 2> players_;
    int currentPlayerIndex_;
    GameStatus status_;
    mutable std::mutex mtx_;
    bool checkWin(int row, int col, Symbol symbol) const;

public:
    Game(std::shared_ptr<Player> p1, std::shared_ptr<Player> p2, int boardSize);
    void makeMove(int row, int col);
    const Board& getBoard() const;
    std::shared_ptr<Player> getCurrentPlayer() const;
    GameStatus getStatus() const;
    std::shared_ptr<Player> getWinner() const;
    void printBoard() const;
};

#endif //GAME_H