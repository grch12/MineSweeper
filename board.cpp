#include "board.h"

#include <cstdlib>
#include <ctime>

#include "customize.h"

void Board::Menu(Upp::Bar& bar) {
  bar.Add("File", THISBACK(FileMenu));
  bar.Add("Game", THISBACK(GameMenu));
}

void Board::FileMenu(Upp::Bar& bar) {
  bar.Add("About", [] {
    Upp::PromptOK(
        "[4 MineSweeper v0.1.0]&Author: grch12&Made with [^https://ultimatepp.org^ U`+`+]. "
        "See LICENSE for details");
  });
  bar.Add("Exit", [&] { Close(); });
};

void Board::GameMenu(Upp::Bar& bar) {
  bar.Add("New Game", [] {
    board->Close();
    board = new Board(w, h, b);
    board->OpenMain();
  });
  bar.Add("Customize", [] {
    CustomizeDlg dlg;
    dlg.Run();
  });
}

/**
 * Creates a new game board of the given size with the given number
 * of bombs.
 *
 * The board is created with a title of "MineSweeper" and a menu
 * with options for a new game and to exit the application.
 *
 * The board is then initialized by setting the size of the window
 * and creating an array of cells. The cells are initialized to
 * false, false, false, and 0, which means that they are not uncovered,
 * are not bombs, are not marked, and have 0 surrounding bombs.
 *
 * The given number of bombs is then randomly distributed across the
 * board by setting the isBomb member of the appropriate cells to true.
 *
 * Finally, the number of safe cells is calculated by subtracting the
 * number of bombs from the total number of cells on the board.
 */
Board::Board(int w, int h, int b) : width(w), height(h), BOMB_COUNT(b) {
  Title("MineSweeper");

  AddFrame(appMenu);
  appMenu.Set(THISBACK(Menu));

  Upp::Rect r = GetRect();
  r.SetSize(AddFrameSize(CELL_SIZE * width, CELL_SIZE * height));
  SetRect(r);

  cells = new Cell*[width];
  for (int i = 0; i < width; i++) {
    cells[i] = new Cell[height]{false, false, false, 0};
  }

  std::srand(std::time(0));
  for (int i = 0; i < BOMB_COUNT; i++) {
    int x, y;
    do {
      x = std::rand() % width;
      y = std::rand() % height;
    } while (cells[x][y].isBomb);
    cells[x][y].isBomb = true;
  }

  safeCells = width * height - BOMB_COUNT;
}

Board::~Board() {
  for (int i = 0; i < width; i++) {
    delete[] cells[i];
  }
  delete[] cells;
}

void Board::Close() { delete this; }

void Board::Paint(Upp::Draw& w) {
  w.DrawRect(GetSize(), Upp::SWhite());

  DrawCells(w);
  DrawGrid(w);
}

void Board::LeftDown(Upp::Point p, Upp::dword flags) {
  if (gameOver) return;
  p /= CELL_SIZE;
  UncoverCell(p.x, p.y);
  Refresh();
}

void Board::RightDown(Upp::Point p, Upp::dword flags) {
  if (gameOver) return;
  p /= CELL_SIZE;
  MarkCell(p.x, p.y);
  Refresh();
}

void Board::DrawGrid(Upp::Draw& w) {
  for (int i = 1; i < width; i++) {
    w.DrawLine(i * CELL_SIZE, 0, i * CELL_SIZE, height * CELL_SIZE, 0,
               Upp::SBlack());
  }
  for (int i = 1; i < height; i++) {
    w.DrawLine(0, i * CELL_SIZE, width * CELL_SIZE, i * CELL_SIZE, 0,
               Upp::SBlack());
  }
}

/**
 * Draws all cells on the board.
 *
 * If the cell is not uncovered, fills the cell with a light gray color.
 *
 * If the cell is a bomb, fills the cell with a light red color.
 *
 * If the cell is not a bomb and has surrounding bombs, draws the number of
 * surrounding bombs in the center of the cell. The color of the text is
 * determined by the number of surrounding bombs: blue if there is 1, green if
 * there are 2, red if there are 3, and black if there are more.
 *
 * If the cell is marked, draws two black lines crossing the center of the cell.
 */
void Board::DrawCells(Upp::Draw& w) {
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      Cell cell = cells[i][j];
      Upp::Color cellColor;
      if (!cell.uncovered) {
        cellColor = Upp::SLtGray();
      } else if (cell.isBomb) {
        cellColor = Upp::LtRed();
      } else {
        cellColor = Upp::SWhite();
      }
      w.DrawRect(i * CELL_SIZE, j * CELL_SIZE, (i + 1) * CELL_SIZE,
                 (j + 1) * CELL_SIZE, cellColor);

      if (!cell.isBomb) {
        if (cell.surroundingBombs > 0) {
          Upp::Color textColor;
          if (cell.surroundingBombs == 1) {
            textColor = Upp::Blue();
          } else if (cell.surroundingBombs == 2) {
            textColor = Upp::Green();
          } else if (cell.surroundingBombs == 3) {
            textColor = Upp::Red();
          } else {
            textColor = Upp::SBlack();
          }
          Upp::Size textSize = Upp::GetTextSize(
              Upp::AsString(cell.surroundingBombs), Upp::StdFontZ(16).Bold());
          w.DrawText(i * CELL_SIZE + (CELL_SIZE - textSize.cx) / 2,
                     j * CELL_SIZE + (CELL_SIZE - textSize.cy) / 2,
                     Upp::AsString(cell.surroundingBombs),
                     Upp::StdFontZ(16).Bold(), textColor);
        }
      }

      if (cell.marked) {
        w.DrawLine(i * CELL_SIZE, j * CELL_SIZE, (i + 1) * CELL_SIZE,
                   (j + 1) * CELL_SIZE, 2, Upp::SBlack());
        w.DrawLine((i + 1) * CELL_SIZE, j * CELL_SIZE, i * CELL_SIZE,
                   (j + 1) * CELL_SIZE, 2, Upp::SBlack());
      }
    }
  }
}

/**
 * Uncovers a cell at the given position.
 *
 * If the cell is marked, or is already uncovered, does nothing.
 *
 * If the cell is a bomb, ends the game and displays a "Game Over" message.
 *
 * If the cell is not a bomb, decrements the number of safe cells and counts
 * the number of surrounding bombs.
 *
 * If the number of surrounding bombs is 0, recursively uncovers all
 * surrounding cells.
 *
 * If the number of safe cells reaches 0, ends the game and displays a
 * "You Win!" message.
 */
void Board::UncoverCell(int x, int y) {
  if (cells[x][y].marked) return;
  if (cells[x][y].uncovered) return;
  cells[x][y].uncovered = true;
  if (cells[x][y].isBomb) {
    gameOver = true;
    Upp::PromptOK("Game Over");
  } else {
    safeCells--;
    CountSurroundingBombs(x, y);
    if (cells[x][y].surroundingBombs == 0) {
      for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
          if (x + i < 0 || x + i >= width || y + j < 0 || y + j >= height)
            continue;
          if (cells[x + i][y + j].uncovered) continue;
          UncoverCell(x + i, y + j);
        }
      }
    }
    if (safeCells == 0 && !gameOver) {
      gameOver = true;
      Upp::PromptOK("You Win!");
    }
  }
}

void Board::CountSurroundingBombs(int x, int y) {
  int count = 0;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      if (x + i < 0 || x + i >= width || y + j < 0 || y + j >= height) continue;
      if (cells[x + i][y + j].isBomb) count++;
    }
  }
  cells[x][y].surroundingBombs = count;
}

void Board::MarkCell(int x, int y) {
  if (cells[x][y].uncovered) return;
  cells[x][y].marked = !cells[x][y].marked;
}
