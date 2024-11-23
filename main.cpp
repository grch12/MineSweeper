#include "board.h"
#include "global.h"

Board* board = nullptr;
int w = 10;
int h = 10;
int b = 10;

GUI_APP_MAIN {
  board = new Board(w, h, b);
  board->OpenMain();
  Upp::Ctrl::EventLoop();
}
