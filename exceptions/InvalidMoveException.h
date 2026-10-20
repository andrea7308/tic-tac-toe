#ifndef INVALID_MOVE_EXCEPTION_H
#define INVALID_MOVE_EXCEPTION_H

#include <stdexcept>
#include <string>

class InvalidMoveException : public std::runtime_error {
public:
    explicit InvalidMoveException(const std::string& message)
        : std::runtime_error(message) {}
};

#endif