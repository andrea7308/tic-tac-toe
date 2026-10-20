#ifndef CELL_H
#define CELL_H

#include "../enums/symbol.h"

class Cell {
    Symbol symbol_;

public:
    Cell() : symbol_(Symbol::EMPTY) {}

    Symbol getSymbol() const {
        return symbol_;
    }

    void setSymbol(Symbol symbol) {
        symbol_ = symbol;
    }

    bool isEmpty() const {
        return symbol_ == Symbol::EMPTY;
    }
};

#endif