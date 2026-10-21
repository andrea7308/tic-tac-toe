(function () {
  const SESSION_KEY = 'tic-tac-toe-player-session';

  function createServerApi({ baseUrl = '/api', fetchImpl = window.fetch.bind(window), storage = window.localStorage } = {}) {
    const endpoint = baseUrl.replace(/\/$/, '');

    function readSession() {
      try {
        return JSON.parse(storage.getItem(SESSION_KEY) || 'null');
      } catch {
        return null;
      }
    }

    function saveSession(state) {
      storage.setItem(SESSION_KEY, JSON.stringify({ gameId: state.gameId, playerToken: state.playerToken }));
    }

    async function request(path, options = {}, token = '') {
      let response;
      try {
        response = await fetchImpl(`${endpoint}${path}`, {
          ...options,
          headers: {
            'Content-Type': 'application/json',
            ...(token ? { Authorization: `Bearer ${token}` } : {}),
            ...options.headers,
          },
        });
      } catch {
        throw new Error('Cannot reach the game server.');
      }

      const body = await response.json().catch(() => null);
      if (!response.ok) {
        if (response.status === 401) storage.removeItem(SESSION_KEY);
        throw new Error(body?.message || `Game request failed (${response.status})`);
      }
      return body;
    }

    async function authenticatedRequest(path, options = {}) {
      const session = readSession();
      if (!session?.gameId || !session?.playerToken) return null;
      try {
        return await request(`/games/${encodeURIComponent(session.gameId)}${path}`, options, session.playerToken);
      } catch (error) {
        if (error.message === 'Game was not found') {
          storage.removeItem(SESSION_KEY);
          return null;
        }
        throw error;
      }
    }

    async function start(path, payload) {
      const state = await request(path, { method: 'POST', body: JSON.stringify(payload) });
      saveSession(state);
      return state;
    }

    return {
      createLocalGame(players) {
        return start('/games/local', { players });
      },
      createGame(name) {
        return start('/games', { name });
      },
      joinGame(joinCode, name) {
        return start('/games/join', { joinCode, name });
      },
      getState() {
        return authenticatedRequest('');
      },
      updateNames(players) {
        return authenticatedRequest('/players', {
          method: 'PATCH',
          body: JSON.stringify({ players }),
        });
      },
      makeMove(row, column) {
        return authenticatedRequest('/moves', {
          method: 'POST',
          body: JSON.stringify({ row, column }),
        });
      },
      restart() {
        return authenticatedRequest('/restart', { method: 'POST', body: '{}' });
      },
      clearSession() {
        storage.removeItem(SESSION_KEY);
      },
    };
  }

  window.TicTacToeApi = { createServerApi };
  window.ticTacToeApi = window.ticTacToeApi || createServerApi({
    baseUrl: window.TIC_TAC_TOE_API_BASE_URL || '/api',
  });
})();
(function () {
  const SESSION_KEY = 'tic-tac-toe-player-session';

  function createServerApi({ baseUrl = '/api', fetchImpl = window.fetch.bind(window), storage = window.localStorage } = {}) {
    const endpoint = baseUrl.replace(/\/$/, '');

    function readSession() {
      try {
        return JSON.parse(storage.getItem(SESSION_KEY) || 'null');
      } catch {
        return null;
      }
    }

    function saveSession(state) {
      storage.setItem(SESSION_KEY, JSON.stringify({ gameId: state.gameId, playerToken: state.playerToken }));
    }

    async function request(path, options = {}, token = '') {
      let response;
      try {
        response = await fetchImpl(`${endpoint}${path}`, {
          ...options,
          headers: {
            'Content-Type': 'application/json',
            ...(token ? { Authorization: `Bearer ${token}` } : {}),
            ...options.headers,
          },
        });
      } catch {
        throw new Error('Cannot reach the game server.');
      }

      const body = await response.json().catch(() => null);
      if (!response.ok) {
        if (response.status === 401) storage.removeItem(SESSION_KEY);
        throw new Error(body?.message || `Game request failed (${response.status})`);
      }
      return body;
    }

    async function authenticatedRequest(path, options = {}) {
      const session = readSession();
      if (!session?.gameId || !session?.playerToken) return null;
      return request(`/games/${encodeURIComponent(session.gameId)}${path}`, options, session.playerToken);
    }

    return {
      async createGame(name) {
        const state = await request('/games', { method: 'POST', body: JSON.stringify({ name }) });
        saveSession(state);
        return state;
      },
      async joinGame(joinCode, name) {
        const state = await request('/games/join', {
          method: 'POST',
          body: JSON.stringify({ joinCode, name }),
        });
        saveSession(state);
        return state;
      },
      getState() {
        return authenticatedRequest('');
      },
      makeMove(row, column) {
        return authenticatedRequest('/moves', {
          method: 'POST',
          body: JSON.stringify({ row, column }),
        });
      },
      restart() {
        return authenticatedRequest('/restart', { method: 'POST', body: '{}' });
      },
      clearSession() {
        storage.removeItem(SESSION_KEY);
      },
    };
  }

  window.TicTacToeApi = { createServerApi };
  const apiBaseUrl = window.TIC_TAC_TOE_API_BASE_URL;
  window.ticTacToeApi = window.ticTacToeApi || (apiBaseUrl
    ? createServerApi({ baseUrl: apiBaseUrl })
    : {
      async createGame() { throw new Error('Game server URL is not configured.'); },
      async joinGame() { throw new Error('Game server URL is not configured.'); },
      async getState() { throw new Error('Game server URL is not configured.'); },
      async makeMove() { throw new Error('Game server URL is not configured.'); },
      async restart() { throw new Error('Game server URL is not configured.'); },
      clearSession() { storage.removeItem(SESSION_KEY); },
    });
})();