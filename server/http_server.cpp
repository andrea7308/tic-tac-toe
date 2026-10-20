#include "http_server.h"

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <vector>

#include "../exceptions/InvalidMoveException.h"

namespace {
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
using tcp = asio::ip::tcp;

json::value snapshotJson(const GameSnapshot& state) {
    json::array board;
    for (const auto& cell : state.board) {
        if (cell.empty()) board.emplace_back(nullptr);
        else board.emplace_back(cell);
    }

    json::object winner;
    if (!state.winnerSymbol.empty()) {
        winner["symbol"] = state.winnerSymbol;
        winner["name"] = state.winnerName;
    }

    return json::object{
        {"gameId", state.gameId},
        {"joinCode", state.joinCode},
        {"playerToken", state.playerToken},
        {"localPlayer", state.localPlayer},
        {"board", std::move(board)},
        {"currentPlayer", state.currentPlayer},
        {"status", state.status},
        {"winner", state.winnerSymbol.empty() ? json::value(nullptr) : json::value(std::move(winner))},
        {"players", json::object{{"X", state.playerX}, {"O", state.playerO}}},
        {"ready", state.ready},
    };
}

std::string requiredString(const json::object& body, const char* key) {
    const auto* value = body.if_contains(key);
    if (!value || !value->is_string()) throw std::invalid_argument(std::string("Missing string field: ") + key);
    return std::string(value->as_string());
}

int requiredInt(const json::object& body, const char* key) {
    const auto* value = body.if_contains(key);
    if (!value || !value->is_int64()) throw std::invalid_argument(std::string("Missing integer field: ") + key);
    return static_cast<int>(value->as_int64());
}

std::string optionalPlayerName(const json::object& body) {
    const auto* name = body.if_contains("name");
    if (name && name->is_string()) return std::string(name->as_string());
    const auto* players = body.if_contains("players");
    if (players && players->is_object()) {
        const auto& object = players->as_object();
        const auto* symbol = object.if_contains("X");
        if (symbol && symbol->is_string()) return std::string(symbol->as_string());
    }
    return "Player X";
}

std::string bearerToken(const http::request<http::string_body>& request) {
    const auto header = request[http::field::authorization];
    const std::string value(header);
    const std::string prefix = "Bearer ";
    if (value.compare(0, prefix.size(), prefix) != 0) return {};
    return value.substr(prefix.size());
}

std::string mimeType(const std::filesystem::path& path) {
    const auto extension = path.extension().string();
    if (extension == ".html") return "text/html; charset=utf-8";
    if (extension == ".css") return "text/css; charset=utf-8";
    if (extension == ".js") return "text/javascript; charset=utf-8";
    if (extension == ".json") return "application/json; charset=utf-8";
    if (extension == ".svg") return "image/svg+xml";
    return "application/octet-stream";
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::out_of_range("File was not found");
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

std::string safeStaticPath(const std::string& target, const std::string& root) {
    std::string path = target.substr(0, target.find('?'));
    if (path == "/") path = "/index.html";
    const auto relative = std::filesystem::path(path).relative_path().lexically_normal();
    if (relative.empty() || *relative.begin() == "..") throw std::out_of_range("File was not found");
    const auto full = (std::filesystem::path(root) / relative).lexically_normal();
    const auto rootPath = std::filesystem::path(root).lexically_normal();
    auto rootIt = rootPath.begin();
    auto fullIt = full.begin();
    for (; rootIt != rootPath.end() && fullIt != full.end(); ++rootIt, ++fullIt) {
        if (*rootIt != *fullIt) throw std::out_of_range("File was not found");
    }
    if (rootIt != rootPath.end()) throw std::out_of_range("File was not found");
    return full.string();
}

template <class Body, class Fields>
http::response<http::string_body> makeResponse(http::status status, unsigned version,
                                                const Body& body, const Fields& fields) {
    http::response<http::string_body> response{status, version};
    response.set(http::field::server, "TicTacToeCpp");
    response.set(http::field::access_control_allow_origin, "*");
    response.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
    response.set(http::field::access_control_allow_headers, "Content-Type, Authorization");
    for (const auto& field : fields) response.set(field.first, field.second);
    response.body() = body;
    response.prepare_payload();
    return response;
}

http::response<http::string_body> jsonResponse(http::status status, unsigned version, json::value body) {
    return makeResponse(status, version, json::serialize(body),
                        std::vector<std::pair<http::field, std::string>>{{http::field::content_type, "application/json"}});
}

http::response<http::string_body> errorResponse(http::status status, unsigned version, const std::string& message) {
    return jsonResponse(status, version, json::object{{"message", message}});
}

http::response<http::string_body> routeRequest(const http::request<http::string_body>& request,
                                               GameService& games, const std::string& frontendDirectory) {
    const auto target = std::string(request.target());
    const auto path = target.substr(0, target.find('?'));
    if (request.method() == http::verb::options) {
        return makeResponse(http::status::no_content, request.version(), std::string{},
                            std::vector<std::pair<http::field, std::string>>{});
    }

    try {
        if (path == "/api/games" && request.method() == http::verb::post) {
            const auto body = json::parse(request.body()).as_object();
            return jsonResponse(http::status::created, request.version(), snapshotJson(games.createGame(optionalPlayerName(body))));
        }
        if (path == "/api/games/join" && request.method() == http::verb::post) {
            const auto body = json::parse(request.body()).as_object();
            return jsonResponse(http::status::ok, request.version(), snapshotJson(
                games.joinGame(requiredString(body, "joinCode"), requiredString(body, "name"))));
        }

        const std::string prefix = "/api/games/";
        if (path.compare(0, prefix.size(), prefix) == 0) {
            const auto remainder = path.substr(prefix.size());
            const auto slash = remainder.find('/');
            const auto gameId = remainder.substr(0, slash);
            if (gameId.empty()) return errorResponse(http::status::not_found, request.version(), "Game was not found");
            const auto token = bearerToken(request);
            if (slash == std::string::npos && request.method() == http::verb::get) {
                return jsonResponse(http::status::ok, request.version(), snapshotJson(games.getState(gameId, token)));
            }
            if (slash != std::string::npos && request.method() == http::verb::post) {
                const auto action = remainder.substr(slash + 1);
                if (action == "moves") {
                    const auto body = json::parse(request.body()).as_object();
                    return jsonResponse(http::status::ok, request.version(), snapshotJson(
                        games.makeMove(gameId, token, requiredInt(body, "row"), requiredInt(body, "column"))));
                }
                if (action == "restart") {
                    return jsonResponse(http::status::ok, request.version(), snapshotJson(games.restart(gameId, token)));
                }
            }
            return errorResponse(http::status::not_found, request.version(), "Game route was not found");
        }

        if (request.method() == http::verb::get) {
            const auto filePath = safeStaticPath(target, frontendDirectory);
            const auto content = readFile(filePath);
            return makeResponse(http::status::ok, request.version(), content,
                                std::vector<std::pair<http::field, std::string>>{{http::field::content_type, mimeType(filePath)}});
        }
        return errorResponse(http::status::not_found, request.version(), "Route was not found");
    } catch (const std::out_of_range& error) {
        return errorResponse(http::status::not_found, request.version(), error.what());
    } catch (const std::invalid_argument& error) {
        const auto status = std::string(error.what()).find("player token") != std::string::npos
            ? http::status::unauthorized : http::status::bad_request;
        return errorResponse(status, request.version(), error.what());
    } catch (const InvalidMoveException& error) {
        return errorResponse(http::status::conflict, request.version(), error.what());
    } catch (const std::runtime_error& error) {
        return errorResponse(http::status::conflict, request.version(), error.what());
    } catch (const std::exception& error) {
        return errorResponse(http::status::bad_request, request.version(), error.what());
    }
}

void handleConnection(tcp::socket socket, GameService& games, const std::string& frontendDirectory) {
    beast::error_code error;
    beast::flat_buffer buffer;
    http::request<http::string_body> request;
    http::read(socket, buffer, request, error);
    if (error) return;
    auto response = routeRequest(request, games, frontendDirectory);
    response.keep_alive(false);
    http::write(socket, response, error);
    socket.shutdown(tcp::socket::shutdown_send, error);
}
}

HttpServer::HttpServer(GameService& gameService, std::string address, std::uint16_t port, std::string frontendDirectory)
    : gameService_(gameService), address_(std::move(address)), port_(port), frontendDirectory_(std::move(frontendDirectory)) {}

void HttpServer::run() {
    asio::io_context context{1};
    const auto address = asio::ip::make_address(address_);
    tcp::acceptor acceptor{context, {address, port_}};
    std::cout << "Tic Tac Toe server listening on http://" << address_ << ':' << port_ << std::endl;
    for (;;) {
        tcp::socket socket{context};
        acceptor.accept(socket);
        std::thread(handleConnection, std::move(socket), std::ref(gameService_), frontendDirectory_).detach();
    }
}