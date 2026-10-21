# C++ Game Server API

The browser is server-only. `frontend/server-api.js` has no local game implementation; it stores the issued game ID and player token in `localStorage` and sends requests to the C++ server. The server runs the existing `Game` rules.

## Run locally

From the repository root:

```sh
cmake -S . -B build/server
cmake --build build/server --target tic-tac-toe-server
./build/server/tic-tac-toe-server 0.0.0.0 8080 frontend
```

Open `http://localhost:8080`. Set `TIC_TAC_TOE_API_BASE_URL` before `server-api.js` loads if the API is hosted at a different origin, for example `https://games.example.com/api`.

## Routes

| Method | Path | Body | Purpose |
| --- | --- | --- | --- |
| `POST` | `/api/games/local` | `{ "players": { "X": "Casey", "O": "Jordan" } }` | Start server-backed pass-and-play on one device. |
| `POST` | `/api/games` | `{ "name": "Alice" }` | Create a game; caller is Player X. |
| `POST` | `/api/games/join` | `{ "joinCode": "ABC234", "name": "Bob" }` | Join an open game; caller is Player O. |
| `GET` | `/api/games/{gameId}` | None | Read state. Requires `Authorization: Bearer {playerToken}`. |
| `PATCH` | `/api/games/{gameId}/players` | `{ "players": { "X": "Casey", "O": "Jordan" } }` | Update player names; same-device sessions can update both names, online players can update their own. |
| `POST` | `/api/games/{gameId}/moves` | `{ "row": 0, "column": 2 }` | Submit a zero-based move. Requires player token. |
| `POST` | `/api/games/{gameId}/restart` | `{}` | Restart the game. Requires player token. |

All successful routes return the state shape below. The create response additionally includes a six-character `joinCode` for the second player and a private `playerToken`. The join response contains the joining player's token. Keep the token private; the frontend stores it in local storage.

The local pass-and-play response sets `localPassAndPlay` to `true`, includes both player names, and does not require a join code. The UI uses one private session token while the C++ `Game` object alternates X and O turns.

The local pass-and-play response sets `localPassAndPlay` to `true`, has both named players, and does not require a join code. The UI uses one private session token while the C++ `Game` object continues alternating X and O turns.

```json
{
  "gameId": "server-generated-id",
  "joinCode": "ABC234",
  "playerToken": "private-player-token",
  "localPlayer": "X",
  "board": [null, null, null, null, null, null, null, null, null],
  "currentPlayer": "X",
  "status": "in_progress",
  "winner": null,
  "players": { "X": "Alice", "O": "Bob" },
  "ready": false,
  "localPassAndPlay": false
}
```

`board` is flat and row-major. `status` is `in_progress`, `winner`, or `draw`; `winner` is either `null` or `{ "symbol": "X", "name": "Alice" }`. Error responses are JSON `{ "message": "..." }` with an appropriate non-2xx status. The client polls state every 1.5 seconds for opponent moves.

## Remote play

Both browsers must use the same server's public HTTPS URL. The current server uses in-memory sessions, so games are lost when the process restarts and multiple server instances do not share games. For production, put it behind HTTPS and a reverse proxy, then move sessions into shared persistent storage before scaling horizontally. The `GameService` session boundary is where persistent storage and later real-time transports (such as WebSocket notifications) can be introduced.
