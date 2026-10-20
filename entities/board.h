#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include "cell.h"
#include "../enums/symbol.h"

class Board {
    std::vector<std::vector<Cell>> grid_;
    int size_;
    void initializeBoard();
    void validatePosition(int row, int col) const;
public:
    explicit Board(int size);
    void placeSymbol(int row, int col, Symbol symbol);
    bool isCellEmpty(int row, int col) const;
    bool isFull() const;
    const Cell& getCell(int row, int col) const;
    int getSize() const;
    void printBoard() const;
};

#endif //BOARD_H