#include <array>
#include <mutex>
#include <stdexcept>
#include <iostream>
#include <memory>
#include "entities/board.h"
#include "entities/player.h"
#include "enums/status.h"
#include "enums/symbol.h"
#include "game.h"
#include "exceptions/InvalidMoveException.h"

Game::Game(std::shared_ptr<Player> p1, std::shared_ptr<Player> p2, int boardSize) 
: board_(std::make_unique<Board>(boardSize))
, players_({p1, p2})
, currentPlayerIndex_(0)
, status_(GameStatus::IN_PROGRESS) {}

bool Game::checkWin(int row, int col, Symbol symbol) const {
    int size = board_->getSize();

    bool win = true;
    for (int c = 0; c < size; c++) {
        if (board_->getCell(row, c).getSymbol() != symbol) {
            win = false;
            break;
        }
    }
    if (win) return true;

    win = true;
    for (int r = 0; r < size; r++) {
        if (board_->getCell(r, col).getSymbol() != symbol) {
            win = false;
            break;
        }
    }
    if (win) return true;
    
    if (row == col) {
        win = true;
        for (int i = 0; i < size; i++) {
            if (board_->getCell(i, i).getSymbol() != symbol) {
                win = false;
                break;
            }
        }
        if (win) return true;
    }


    if (row + col == size - 1) {
        win = true;
        for (int i = 0; i < size; i++) {
            if (board_->getCell(i, size - 1 - i).getSymbol() != symbol) {
                win = false; 
                break;
            }
        }
        if (win) return true;
    }

    return false;
}

void Game::makeMove(int row, int col) {
    std::lock_guard<std::mutex> lock(mtx_);

    if (status_ != GameStatus::IN_PROGRESS) {
        throw InvalidMoveException("Game is already over!");
    } 
    
    if (board_->isCellEmpty(row, col) == false) {
        throw InvalidMoveException(
            "Cell (" + std::to_string(row) + ", " +
            std::to_string(col) + ") is already occupied"
        );
    } 

    auto& currentPlayer = players_[currentPlayerIndex_];
    board_->placeSymbol(row, col, currentPlayer->getSymbol());

    if (checkWin(row, col, currentPlayer->getSymbol())) {
        status_ = (currentPlayer->getSymbol() == Symbol::X) 
            ? GameStatus::WINNER_X 
            : GameStatus::WINNER_O;
        return;
    } 
    
    if (board_->isFull()) {
        status_ = GameStatus::DRAW;
        return;
    } 
    
    currentPlayerIndex_ = (currentPlayerIndex_ == 0) ? 1 : 0;
}

const Board& Game::getBoard() const { return *board_; }

std::shared_ptr<Player> Game::getCurrentPlayer() const {
    return players_[currentPlayerIndex_];
}

GameStatus Game::getStatus() const { return status_; }

std::shared_ptr<Player> Game::getWinner() const {
    if (status_ == GameStatus::WINNER_X) { 
        return players_[0]->getSymbol() == Symbol::X ? players_[0] : players_[1];
    } else if (status_ == GameStatus::WINNER_O) {
        return players_[0]->getSymbol() == Symbol::O ? players_[0] : players_[1];
    }
    return nullptr;
}

void Game::printBoard() const {
    board_->printBoard();
}
