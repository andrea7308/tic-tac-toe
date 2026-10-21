#include "game_service.h"

#include <algorithm>
#include <cctype>
#include <random>
#include <stdexcept>

#include "../enums/status.h"
#include "../enums/symbol.h"
#include "../exceptions/InvalidMoveException.h"

namespace {
const char TOKEN_ALPHABET[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const char CODE_ALPHABET[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
}

std::string GameService::makeRandomToken(std::size_t length) {
    std::random_device device;
    std::uniform_int_distribution<std::size_t> distribution(0, sizeof(TOKEN_ALPHABET) - 2);
    std::string token;
    token.reserve(length);
    for (std::size_t index = 0; index < length; ++index) {
        token += TOKEN_ALPHABET[distribution(device)];
    }
    return token;
}

std::string GameService::makeJoinCode(std::size_t length) {
    std::random_device device;
    std::uniform_int_distribution<std::size_t> distribution(0, sizeof(CODE_ALPHABET) - 2);
    std::string code;
    code.reserve(length);
    for (std::size_t index = 0; index < length; ++index) {
        code += CODE_ALPHABET[distribution(device)];
    }
    return code;
}

std::string GameService::normalizeName(const std::string& name, const std::string& fallback) {
    std::string value = name;
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return fallback;
    const auto last = value.find_last_not_of(" \t\r\n");
    value = value.substr(first, last - first + 1);
    if (value.size() > 20) value.resize(20);
    return value;
}

std::string GameService::normalizeCode(const std::string& code) {
    std::string value;
    for (unsigned char character : code) {
        if (!std::isspace(character) && character != '-') {
            value += static_cast<char>(std::toupper(character));
        }
    }
    return value;
}

std::string GameService::statusName(GameStatus status) {
    switch (status) {
        case GameStatus::WINNER_X:
        case GameStatus::WINNER_O: return "winner";
        case GameStatus::DRAW: return "draw";
        default: return "in_progress";
    }
}

std::string GameService::symbolName(Symbol symbol) {
    switch (symbol) {
        case Symbol::X: return "X";
        case Symbol::O: return "O";
        default: return "";
    }
}

std::shared_ptr<GameService::Session> GameService::findSession(const std::string& gameId) const {
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    const auto found = sessions_.find(gameId);
    if (found == sessions_.end()) throw std::out_of_range("Game was not found");
    return found->second;
}

std::string GameService::authorize(const Session& session, const std::string& token) {
    if (!token.empty() && token == session.xToken) return "X";
    if (!token.empty() && token == session.oToken) return "O";
    throw std::invalid_argument("A valid player token is required");
}

GameSnapshot GameService::snapshot(const Session& session, const std::string& token) {
    GameSnapshot result;
    result.gameId = session.id;
    result.joinCode = session.joinCode;
    result.playerToken = token;
    result.localPlayer = token == session.xToken ? "X" : "O";
    result.currentPlayer = symbolName(session.game->getCurrentPlayer()->getSymbol());
    result.status = statusName(session.game->getStatus());
    result.playerX = session.playerX->getName();
    result.playerO = session.playerO->getName();
    result.ready = !session.oToken.empty();
    result.localPassAndPlay = session.localPassAndPlay;

    const auto winner = session.game->getWinner();
    if (winner) {
        result.winnerSymbol = symbolName(winner->getSymbol());
        result.winnerName = winner->getName();
    }

    const Board& board = session.game->getBoard();
    for (int row = 0; row < board.getSize(); ++row) {
        for (int column = 0; column < board.getSize(); ++column) {
            result.board.push_back(symbolName(board.getCell(row, column).getSymbol()));
        }
    }
    return result;
}

GameSnapshot GameService::createGame(const std::string& playerName) {
    auto session = std::make_shared<Session>();
    session->id = makeRandomToken(24);
    session->xToken = makeRandomToken(48);
    session->localPassAndPlay = false;
    session->playerX = std::make_shared<Player>(normalizeName(playerName, "Player X"), Symbol::X);
    session->playerO = std::make_shared<Player>("Waiting for player", Symbol::O);
    session->game = std::make_unique<Game>(session->playerX, session->playerO, 3);

    std::lock_guard<std::mutex> lock(sessionsMutex_);
    do {
        session->joinCode = makeJoinCode(6);
    } while (joinCodes_.count(session->joinCode) != 0);
    sessions_.emplace(session->id, session);
    joinCodes_.emplace(session->joinCode, session->id);
    std::lock_guard<std::mutex> sessionLock(session->mutex);
    return snapshot(*session, session->xToken);
}

GameSnapshot GameService::createLocalGame(const std::string& playerXName, const std::string& playerOName) {
    auto session = std::make_shared<Session>();
    session->id = makeRandomToken(24);
    session->xToken = makeRandomToken(48);
    session->oToken = session->xToken;
    session->localPassAndPlay = true;
    session->playerX = std::make_shared<Player>(normalizeName(playerXName, "Player X"), Symbol::X);
    session->playerO = std::make_shared<Player>(normalizeName(playerOName, "Player O"), Symbol::O);
    session->game = std::make_unique<Game>(session->playerX, session->playerO, 3);

    std::lock_guard<std::mutex> lock(sessionsMutex_);
    sessions_.emplace(session->id, session);
    std::lock_guard<std::mutex> sessionLock(session->mutex);
    return snapshot(*session, session->xToken);
}

GameSnapshot GameService::joinGame(const std::string& joinCode, const std::string& playerName) {
    std::shared_ptr<Session> session;
    {
        std::lock_guard<std::mutex> lock(sessionsMutex_);
        const auto code = joinCodes_.find(normalizeCode(joinCode));
        if (code == joinCodes_.end()) throw std::out_of_range("Game code was not found");
        session = sessions_.at(code->second);
    }

    std::lock_guard<std::mutex> lock(session->mutex);
    if (!session->oToken.empty()) throw std::runtime_error("This game already has two players");
    session->localPassAndPlay = false;
    session->oToken = makeRandomToken(48);
    session->playerO = std::make_shared<Player>(normalizeName(playerName, "Player O"), Symbol::O);
    session->game = std::make_unique<Game>(session->playerX, session->playerO, 3);
    return snapshot(*session, session->oToken);
}

GameSnapshot GameService::getState(const std::string& gameId, const std::string& playerToken) const {
    auto session = findSession(gameId);
    std::lock_guard<std::mutex> lock(session->mutex);
    authorize(*session, playerToken);
    return snapshot(*session, playerToken);
}

GameSnapshot GameService::updateNames(const std::string& gameId, const std::string& playerToken,
                                      const std::string& playerXName, const std::string& playerOName) {
    auto session = findSession(gameId);
    std::lock_guard<std::mutex> lock(session->mutex);
    const auto role = authorize(*session, playerToken);
    if (session->localPassAndPlay || role == "X") {
        session->playerX->setName(normalizeName(playerXName, "Player X"));
    }
    if (session->localPassAndPlay || role == "O") {
        session->playerO->setName(normalizeName(playerOName, "Player O"));
    }
    return snapshot(*session, playerToken);
}

GameSnapshot GameService::makeMove(const std::string& gameId, const std::string& playerToken, int row, int column) {
    auto session = findSession(gameId);
    std::lock_guard<std::mutex> lock(session->mutex);
    const auto role = authorize(*session, playerToken);
    if (session->oToken.empty()) throw std::runtime_error("Waiting for the second player to join");
    if (!session->localPassAndPlay && symbolName(session->game->getCurrentPlayer()->getSymbol()) != role) {
        throw std::runtime_error("It is the other player's turn");
    }
    session->game->makeMove(row, column);
    return snapshot(*session, playerToken);
}

GameSnapshot GameService::restart(const std::string& gameId, const std::string& playerToken) {
    auto session = findSession(gameId);
    std::lock_guard<std::mutex> lock(session->mutex);
    authorize(*session, playerToken);
    session->game = std::make_unique<Game>(session->playerX, session->playerO, 3);
    return snapshot(*session, playerToken);
}