#include "customize.h"

#include "board.h"
#include "global.h"

#define TFILE <MineSweeper/app.t>
#include <Core/t.h>

CustomizeDlg::CustomizeDlg() {
  CtrlLayout(*this);
  inputWidth <<= w;
  inputHeight <<= h;
  inputBombs <<= b;
  okBtn.Ok() << [&] {
    if (inputWidth <= 0 || inputHeight <= 0 || inputBombs <= 0) return;
    w = (int)inputWidth;
    h = (int)inputHeight;
    b = (int)inputBombs;
    Close();
    board->Close();
    board = new Board(w, h, b);
    board->OpenMain();
  };
  cancelBtn.Cancel() << [&] { Close(); };
}
