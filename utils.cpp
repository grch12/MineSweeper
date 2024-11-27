#include "utils.h"

#include "board.h"

void NewGame() {
  explodePoint = {-1, -1};
  board->Close();
  board = new Board(w, h, b);
  board->OpenMain();
}
