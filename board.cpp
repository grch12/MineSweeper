#include "board.h"

#include <cstdlib>
#include <ctime>

#include "customize.h"
#include "utils.h"

#define TFILE <MineSweeper/app.t>
#include <Core/t.h>

#define IMAGECLASS IconImg
#define IMAGEFILE <MineSweeper/icon.iml>
#include <Draw/iml.h>

/**
 * Constructs a new Board with the specified width, height, and mine count.
 *
 * Creates a top-level window with a menu bar and a status bar.
 *
 * Sets the window's size to fit the specified width and height, with a little
 * bit of extra room for the frame.
 *
 * Initializes an array of width * height cells, with all cells initially
 * uncovered and not marked.
 *
 * Randomly selects MINE_COUNT cells to be mines.
 */
Board::Board(int w, int h, int m) : width(w), height(h), MINE_COUNT(m) {
  Title(Upp::t_("MineSweeper"));
  Icon(IconImg::Icon());

  AddFrame(appMenu);
  appMenu.Set(THISBACK(Menu));

  safeCells = width * height - MINE_COUNT;

  AddFrame(sb);
  sb = FormatStatusString();

  Upp::Rect r = GetRect();
  r.SetSize(AddFrameSize(CELL_SIZE * width, CELL_SIZE * height));
  SetRect(r);

  cells = new Cell*[width];
  for (int i = 0; i < width; i++) {
    cells[i] = new Cell[height]{{false, false, false, 0}};
  }

  std::srand(std::time(0));
  for (int i = 0; i < MINE_COUNT; i++) {
    int x, y;
    do {
      x = std::rand() % width;
      y = std::rand() % height;
    } while (cells[x][y].isMine);
    cells[x][y].isMine = true;
  }
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
  int result = UncoverCell(p.x, p.y);
  Update();
  switch (result) {
    case -1: {
      EndGame();
      Upp::PromptOK(Upp::t_("You Win!"));
      break;
    }
    case -2: {
      EndGame();
      Upp::PromptOK(Upp::t_("Game Over"));
      break;
    }
    default: {
      break;
    }
  }
}

void Board::RightDown(Upp::Point p, Upp::dword flags) {
  if (gameOver) return;
  p /= CELL_SIZE;
  MarkCell(p.x, p.y);
  Update();
}

void Board::Update() {
  sb = FormatStatusString();
  Refresh();
}

void Board::EndGame() {
  gameOver = true;
  UncoverAll();
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
 * Draws all cells in the grid, including uncovered cells, marked cells, and
 * cells containing mines.
 *
 * The color of each cell is determined by its state: gray for uncovered cells,
 * red for the cell where the user lost, and white for all other cells.
 *
 * If the cell is a mine and has been uncovered, draws a mine icon in the cell.
 *
 * If the cell is marked, draws a flag icon in the cell. If the user has lost
 * and the cell is not a mine, draws two diagonal lines through the cell to
 * indicate that the mark was incorrect.
 */
void Board::DrawCells(Upp::Draw& w) {
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      Cell cell = cells[i][j];
      Upp::Color cellColor;
      if (!cell.uncovered) {
        cellColor = Upp::SLtGray();
      } else if (explodePoint.x == i && explodePoint.y == j) {
        cellColor = Upp::LtRed();
      } else {
        cellColor = Upp::SWhite();
      }
      w.DrawRect(i * CELL_SIZE, j * CELL_SIZE, CELL_SIZE, CELL_SIZE, cellColor);

      if (!cell.isMine) {
        if (cell.surroundingMines > 0) {
          Upp::Color textColor;
          if (cell.surroundingMines == 1) {
            textColor = Upp::Blue();
          } else if (cell.surroundingMines == 2) {
            textColor = Upp::Green();
          } else if (cell.surroundingMines == 3) {
            textColor = Upp::Red();
          } else {
            textColor = Upp::SBlack();
          }
          Upp::Size textSize = Upp::GetTextSize(
              Upp::AsString(cell.surroundingMines), Upp::StdFontZ(16).Bold());
          w.DrawText(i * CELL_SIZE + (CELL_SIZE - textSize.cx) / 2,
                     j * CELL_SIZE + (CELL_SIZE - textSize.cy) / 2,
                     Upp::AsString(cell.surroundingMines),
                     Upp::StdFontZ(16).Bold(), textColor);
        }
      }

      if (cell.isMine && cell.uncovered) {
        w.DrawImage(i * CELL_SIZE + 3, j * CELL_SIZE + 3, CELL_SIZE - 6,
                    CELL_SIZE - 6, IconImg::Icon());
      }

      if (cell.marked) {
        w.DrawImage(i * CELL_SIZE, j * CELL_SIZE, CELL_SIZE, CELL_SIZE,
                    IconImg::Flag());
        if (gameOver && !cell.isMine) {
          w.DrawLine(i * CELL_SIZE, j * CELL_SIZE, (i + 1) * CELL_SIZE,
                     (j + 1) * CELL_SIZE, 2, Upp::SBlack());
          w.DrawLine((i + 1) * CELL_SIZE, j * CELL_SIZE, i * CELL_SIZE,
                     (j + 1) * CELL_SIZE, 2, Upp::SBlack());
        }
      }
    }
  }
}

/**
 * Uncovers a cell at the given position.
 *
 * If the cell is marked or already uncovered, does nothing.
 *
 * If the cell is a mine, sets the game over flag and returns -2.
 *
 * If the cell is not a mine, decrements the safe cell count and returns 0.
 *
 * If the uncovered cell has no surrounding mines, uncovers all its neighbors
 * recursively.
 *
 * If the safe cell count reaches 0, returns -1 to indicate a win.
 */
int Board::UncoverCell(int x, int y) {
  if (cells[x][y].marked) return 0;
  if (cells[x][y].uncovered) return 0;
  cells[x][y].uncovered = true;
  if (cells[x][y].isMine) {
    if (!gameOver) explodePoint = {x, y};
    return -2;  // Lost
  } else {
    safeCells--;
    CountSurroundingMines(x, y);
    if (cells[x][y].surroundingMines == 0) {
      for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
          if (x + i < 0 || x + i >= width || y + j < 0 || y + j >= height)
            continue;
          UncoverCell(x + i, y + j);
        }
      }
    }
    if (safeCells == 0) {
      return -1;  // Win
    }
  }
  return 0;
}

void Board::UncoverAll() {
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      UncoverCell(i, j);
    }
  }
}

void Board::CountSurroundingMines(int x, int y) {
  int count = 0;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      if (x + i < 0 || x + i >= width || y + j < 0 || y + j >= height) continue;
      if (cells[x + i][y + j].isMine) count++;
    }
  }
  cells[x][y].surroundingMines = count;
}

void Board::MarkCell(int x, int y) {
  if (cells[x][y].uncovered) return;
  cells[x][y].marked = !cells[x][y].marked;
  if (cells[x][y].marked)
    markedCells++;
  else
    markedCells--;
}

Upp::String Board::FormatStatusString() {
  return Upp::Format(Upp::t_("%d mines, %d marked, %d safe cells remaining"),
                     MINE_COUNT, markedCells, safeCells);
}
