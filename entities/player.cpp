#include <string>
#include <stdexcept>
#include "player.h"

Player::Player(const std::string& name, Symbol symbol) : name_(name), symbol_(symbol) {
    if (symbol_ == Symbol::EMPTY) {
        throw std::invalid_argument("Player cannot have EMPTY symbol");
    }
}

const std::string& Player::getName() const {
    return name_;
}

Symbol Player::getSymbol() const {
    return symbol_;
}

std::string Player::toString() const {
    return name_ + " (" + getDisplayChar(symbol_) + ")";
}


