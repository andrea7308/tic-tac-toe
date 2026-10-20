#ifndef SYMBOL_H
#define SYMBOL_H

enum class Symbol {
    X,
    O,
    EMPTY
};

char getDisplayChar(Symbol symbol);

#endif //SYMBOL_H