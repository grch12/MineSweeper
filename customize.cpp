#include "customize.h"

#include "board.h"
#include "global.h"
#include "utils.h"

#define TFILE <MineSweeper/app.t>
#include <Core/t.h>

CustomizeDlg::CustomizeDlg() {
  CtrlLayout(*this);
  inputWidth <<= w;
  inputHeight <<= h;
  inputMines <<= m;
  okBtn.Ok() << [&] {
    if (inputWidth <= 0 || inputHeight <= 0 || inputMines <= 0) return;
    w = (int)inputWidth;
    h = (int)inputHeight;
    m = (int)inputMines;
    NewGame();
  };
  cancelBtn.Cancel() << [&] { Close(); };
}
