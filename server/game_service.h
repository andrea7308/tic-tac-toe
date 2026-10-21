#ifndef GAME_SERVICE_H
#define GAME_SERVICE_H

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../entities/player.h"
#include "../game.h"

struct GameSnapshot {
    std::string gameId;
    std::string joinCode;
    std::string playerToken;
    std::string localPlayer;
    std::string currentPlayer;
    std::string status;
    std::string winnerSymbol;
    std::string winnerName;
    std::string playerX;
    std::string playerO;
    bool ready;
    bool localPassAndPlay;
    std::vector<std::string> board;
};

class GameService {
    struct Session {
        std::mutex mutex;
        std::string id;
        std::string joinCode;
        std::string xToken;
        std::string oToken;
        bool localPassAndPlay;
        std::shared_ptr<Player> playerX;
        std::shared_ptr<Player> playerO;
        std::unique_ptr<Game> game;
    };

    mutable std::mutex sessionsMutex_;
    std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
    std::unordered_map<std::string, std::string> joinCodes_;

    std::shared_ptr<Session> findSession(const std::string& gameId) const;
    static std::string makeRandomToken(std::size_t length);
    static std::string makeJoinCode(std::size_t length);
    static std::string normalizeName(const std::string& name, const std::string& fallback);
    static std::string normalizeCode(const std::string& code);
    static std::string statusName(GameStatus status);
    static std::string symbolName(Symbol symbol);
    static GameSnapshot snapshot(const Session& session, const std::string& token);
    static std::string authorize(const Session& session, const std::string& token);

public:
    GameSnapshot createLocalGame(const std::string& playerXName, const std::string& playerOName);
    GameSnapshot createGame(const std::string& playerName);
    GameSnapshot joinGame(const std::string& joinCode, const std::string& playerName);
    GameSnapshot getState(const std::string& gameId, const std::string& playerToken) const;
    GameSnapshot updateNames(const std::string& gameId, const std::string& playerToken,
                             const std::string& playerXName, const std::string& playerOName);
    GameSnapshot makeMove(const std::string& gameId, const std::string& playerToken, int row, int column);
    GameSnapshot restart(const std::string& gameId, const std::string& playerToken);
};

#endif