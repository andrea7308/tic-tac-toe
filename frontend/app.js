const boardElement = document.querySelector('#board');
const cells = [...boardElement.querySelectorAll('.cell')];
const playerXInput = document.querySelector('#player-x');
const playerOInput = document.querySelector('#player-o');
const turnIndicator = document.querySelector('#turn-indicator');
const turnText = document.querySelector('#turn-text');
const turnSymbol = document.querySelector('#turn-symbol');
const restartButton = document.querySelector('#restart-button');
const resultDialog = document.querySelector('#result-dialog');
const resultTitle = document.querySelector('#result-title');
const resultDescription = document.querySelector('#result-description');
const resultKicker = document.querySelector('#result-kicker');
const resultMark = document.querySelector('#result-mark');
const playAgainButton = document.querySelector('#play-again-button');
const createGameButton = document.querySelector('#create-game-button');
const joinGameButton = document.querySelector('#join-game-button');
const joinCodeInput = document.querySelector('#join-code-input');
const connectionStatus = document.querySelector('#connection-status');
const shareCode = document.querySelector('#share-code');
const gameApi = window.ticTacToeApi;

const winningLines = [
  [0, 1, 2], [3, 4, 5], [6, 7, 8],
  [0, 3, 6], [1, 4, 7], [2, 5, 8],
  [0, 4, 8], [2, 4, 6],
];

let gameState = null;
let requestPending = false;
let requestError = '';

function playerName(symbol) {
  const input = symbol === 'X' ? playerXInput : playerOInput;
  const fallback = symbol === 'X' ? 'Player X' : 'Player O';
  return input.value.trim() || fallback;
}

function currentPlayers() {
  return { X: playerName('X'), O: playerName('O') };
}

function renderTurn() {
  if (!gameState) return;
  if (!gameState.ready) {
    turnText.textContent = 'Waiting for opponent';
    turnSymbol.textContent = gameState.localPlayer || 'X';
    turnIndicator.classList.toggle('is-o', gameState.localPlayer === 'O');
    return;
  }
  const symbol = gameState.currentPlayer;
  turnText.textContent = requestError || `${playerName(symbol)}'s turn`;
  turnSymbol.textContent = symbol;
  turnIndicator.classList.toggle('is-o', symbol === 'O');
}

function winningLineFor(symbol) {
  return winningLines.find((line) => line.every((index) => gameState.board[index] === symbol));
}

function renderResult() {
  if (gameState.status === 'winner') {
    const winner = typeof gameState.winner === 'string'
      ? { symbol: gameState.winner }
      : gameState.winner;
    const name = winner.name || playerName(winner.symbol);
    resultKicker.textContent = 'THAT\'S A LINE';
    resultTitle.textContent = `Winner: ${name}`;
    resultDescription.textContent = 'Nicely played. Ready for another round?';
    resultMark.textContent = winner.symbol;
    resultMark.className = `result-mark${winner.symbol === 'O' ? ' is-o' : ''}`;
    winningLineFor(winner.symbol)?.forEach((index) => cells[index].classList.add('winning-cell'));
  } else {
    resultKicker.textContent = 'A PERFECT STALEMATE';
    resultTitle.textContent = 'It\'s a draw';
    resultDescription.textContent = 'No spaces left. Shall we go again?';
    resultMark.textContent = '↔';
    resultMark.className = 'result-mark is-draw';
  }
  if (!resultDialog.open) {
    resultDialog.showModal();
    playAgainButton.focus();
  }
}

function renderState() {
  if (!gameState) {
    cells.forEach((cell) => { cell.disabled = true; });
    restartButton.disabled = true;
    createGameButton.disabled = requestPending;
    joinGameButton.disabled = requestPending;
    return;
  }
  gameState.board.forEach((symbol, index) => {
    const cell = cells[index];
    cell.className = 'cell';
    cell.replaceChildren();
    cell.disabled = requestPending || !gameState.ready || Boolean(symbol) || gameState.status !== 'in_progress';
    cell.setAttribute('aria-label', `Row ${Math.floor(index / 3) + 1}, column ${(index % 3) + 1}, ${symbol || 'empty'}`);
    if (symbol) {
      cell.classList.add(symbol === 'X' ? 'mark-x' : 'mark-o');
      const mark = document.createElement('span');
      mark.className = 'cell-mark';
      mark.setAttribute('aria-hidden', 'true');
      mark.textContent = symbol;
      cell.append(mark);
    }
  });

  if (gameState.players) {
    playerXInput.value = gameState.players.X || playerXInput.value;
    playerOInput.value = gameState.players.O || playerOInput.value;
  }
  restartButton.disabled = requestPending;
  createGameButton.disabled = requestPending;
  joinGameButton.disabled = requestPending;
  connectionStatus.textContent = gameState.ready
    ? `Connected as Player ${gameState.localPlayer}. Game ${gameState.joinCode}.`
    : `Game ${gameState.joinCode} is ready. Share the code with your opponent.`;
  shareCode.textContent = gameState.joinCode;
  shareCode.hidden = !gameState.joinCode;
  renderTurn();
  if (gameState.status === 'winner' || gameState.status === 'draw') renderResult();
}

function showRequestError(error) {
  requestError = error.message || 'Could not update the game. Try again.';
  connectionStatus.textContent = requestError;
  renderTurn();
}

async function placeMark(index) {
  if (!gameState || !gameState.ready || requestPending || gameState.status !== 'in_progress' || gameState.board[index]) return;
  requestError = '';
  requestPending = true;
  renderState();
  try {
    gameState = await gameApi.makeMove(Math.floor(index / 3), index % 3, currentPlayers());
    renderState();
  } catch (error) {
    showRequestError(error);
  } finally {
    requestPending = false;
    renderState();
  }
}

async function restartGame() {
  if (!gameState || requestPending) return;
  requestError = '';
  requestPending = true;
  renderState();
  try {
    gameState = await gameApi.restart(currentPlayers());
    if (resultDialog.open) resultDialog.close();
    renderState();
  } catch (error) {
    showRequestError(error);
  } finally {
    requestPending = false;
    renderState();
  }
}

async function connectGame(action) {
  if (requestPending) return;
  requestPending = true;
  requestError = '';
  connectionStatus.textContent = action === 'create' ? 'Creating game...' : 'Joining game...';
  renderState();
  try {
    gameState = action === 'create'
      ? await gameApi.createGame(playerName('X'))
      : await gameApi.joinGame(joinCodeInput.value.trim(), playerName('O'));
    if (resultDialog.open) resultDialog.close();
    joinCodeInput.value = '';
    renderState();
  } catch (error) {
    connectionStatus.textContent = error.message || 'Could not connect to the game.';
  } finally {
    requestPending = false;
    renderState();
  }
}

async function refreshGameState() {
  if (!gameState || requestPending) return;
  try {
    const latest = await gameApi.getState();
    if (latest) {
      requestError = '';
      gameState = latest;
      renderState();
    }
  } catch (error) {
    showRequestError(error);
  }
}

cells.forEach((cell, index) => cell.addEventListener('click', () => placeMark(index)));
restartButton.addEventListener('click', restartGame);
playAgainButton.addEventListener('click', restartGame);
createGameButton.addEventListener('click', () => connectGame('create'));
joinGameButton.addEventListener('click', () => connectGame('join'));
joinCodeInput.addEventListener('keydown', (event) => {
  if (event.key === 'Enter') connectGame('join');
});
playerXInput.addEventListener('input', renderTurn);
playerOInput.addEventListener('input', renderTurn);
resultDialog.addEventListener('cancel', (event) => event.preventDefault());

gameApi.getState()
  .then((state) => {
    gameState = state;
    if (gameState) {
      renderState();
    } else {
      connectionStatus.textContent = 'Create a game or join with a code.';
      renderState();
    }
  })
  .catch(showRequestError);

window.setInterval(refreshGameState, 1500);