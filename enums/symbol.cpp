#include "symbol.h"

char getDisplayChar(Symbol symbol) {
    switch (symbol) {
        case Symbol::X: return 'X';
        case Symbol::O: return 'O';
        case Symbol::EMPTY: return '_';
    }
    return '?';
}