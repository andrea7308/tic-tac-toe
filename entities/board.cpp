#include <iostream>
#include <string>

#include "board.h"
#include "../exceptions/InvalidMoveException.h"

Board::Board(int size) : size_(size) {
    initializeBoard();
}

void Board::initializeBoard() {
    grid_.resize(size_);
    for (int row = 0; row < size_; row++) {
        grid_[row].resize(size_);
    }
}

void Board::validatePosition(int row, int col) const {
    if (row < 0 || row >= size_ || col < 0 || col >= size_) {
        throw InvalidMoveException(
            "Position (" + std::to_string(row) + ", " +
            std::to_string(col) + ") is out of bounds"
        );
    }
}

void Board::placeSymbol(int row, int col, Symbol symbol) {
    validatePosition(row, col);
    grid_[row][col].setSymbol(symbol);
}

bool Board::isCellEmpty(int row, int col) const {
    validatePosition(row, col);
    return grid_[row][col].isEmpty();
}

bool Board::isFull() const {
    for (int row = 0; row < size_; row++) {
        for (int col = 0; col < size_; col++) {
            if (grid_[row][col].isEmpty()) {
                return false;
            }
        }
    }
    return true;
}

const Cell& Board::getCell(int row, int col) const {
    validatePosition(row, col);
    return grid_[row][col];
}

int Board::getSize() const {
    return size_;
}

void Board::printBoard() const {
    std::cout << std::endl;
    for (int row = 0; row < size_; row++) {
        for (int col = 0; col < size_; col++) {
            std::cout << " " << getDisplayChar(grid_[row][col].getSymbol()) << " ";
            if (col < size_ - 1) {
                std::cout << "|";
            }
        }
        std::cout << std::endl;
        if (row < size_ - 1) {
            std::cout << std::string(size_ * 4 - 1, '-') << std::endl;
        }
    }
    std::cout << std::endl;
}