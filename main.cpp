#include "board.h"
#include "global.h"

Board* board = nullptr;
int w = 10;
int h = 10;
int b = 10;

Upp::Point explodePoint = {-1, -1};

GUI_APP_MAIN {
  Upp::SetLanguage(Upp::GetSystemLNG());
  board = new Board(w, h, b);
  board->OpenMain();
  Upp::Ctrl::EventLoop();
}
