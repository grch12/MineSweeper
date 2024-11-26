#include "board.h"
#include "utils.h"

void NewGame() {
  board->Close();
  board = new Board(w, h, b);
  board->OpenMain();
}