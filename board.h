#ifndef _MineSweeper_board_h_
#define _MineSweeper_board_h_

#include "global.h"

struct Cell {
  bool uncovered;
  bool isBomb;
  bool marked;
  int surroundingBombs;
};

class Board : public Upp::TopWindow {
 public:
  using CLASSNAME = Board;

  Upp::MenuBar appMenu;
  void Menu(Upp::Bar& menu);
  void FileMenu(Upp::Bar& bar);
  void GameMenu(Upp::Bar& bar);

  Upp::StatusBar sb;

  int width, height;
  int BOMB_COUNT;
  static constexpr int CELL_SIZE = 30;

  int safeCells = 0;

  bool gameOver = false;

  Board(int w, int h, int b);
  ~Board();

  virtual void Close() override;

  virtual void Paint(Upp::Draw& w) override;
  virtual void LeftDown(Upp::Point p, Upp::dword flags) override;
  virtual void RightDown(Upp::Point p, Upp::dword flags) override;

 private:
  Cell** cells;

  int markedCells = 0;

  void Update();

  void EndGame();

  void DrawGrid(Upp::Draw& w);
  void DrawCells(Upp::Draw& w);

  int UncoverCell(int x, int y);
  void UncoverAll();
  void CountSurroundingBombs(int x, int y);

  void MarkCell(int x, int y);

  Upp::String FormatStatusString();
};

#endif
