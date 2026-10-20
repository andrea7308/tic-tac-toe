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
  window.ticTacToeApi = window.ticTacToeApi || createServerApi({
    baseUrl: window.TIC_TAC_TOE_API_BASE_URL || '/api',
  });
})();