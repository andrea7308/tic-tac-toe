#ifndef PLAYER_H
#define PLAYER_H
#include <string>
#include "../enums/symbol.h"

class Player {
    std::string name_;
    const Symbol symbol_;

public:
    Player(const std::string& name, Symbol symbol);
    const std::string& getName() const;
    void setName(const std::string& name);
    Symbol getSymbol() const;
    std::string toString() const;
};

#endif //PLAYER_H