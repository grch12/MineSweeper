#include "board.h"

#include <cstdlib>
#include <ctime>

#include "customize.h"
#include "utils.h"

#define TFILE <MineSweeper/app.t>
#include <Core/t.h>

void Board::Menu(Upp::Bar& bar) {
  bar.Add(Upp::t_("File"), THISBACK(FileMenu));
  bar.Add(Upp::t_("Game"), THISBACK(GameMenu));
}

void Board::FileMenu(Upp::Bar& bar) {
  bar.Add(Upp::t_("About"), [] {
    Upp::PromptOK(
        "[4 MineSweeper v0.2.0]&Author: grch12&GitHub repo: "
        "[^https://github.com/grch12/MineSweeper^ grch12/MineSweeper]&"
        "Icon: [^https://openclipart.org/detail/20846^ "
        "cartoon sea mine by rg1024]&Flag Image: "
        "[^https://www.svgrepo.com/svg/251968/flag-maps-and-flags^ "
        "Flag Maps And Flags SVG Vector] from SVG Repo&"
        "Made with [^https://www.ultimatepp.org^ U`+`+]. "
        "See LICENSE for details");
  });
  bar.Add(Upp::t_("Exit"), [&] { Close(); });
};

/**
 * Configures the game menu options in the application's menu bar.
 *
 * Adds a "New Game" option to start a new game immediately.
 *
 * Adds a "Difficulty" submenu with predefined difficulty levels:
 * "Beginner", "Intermediate", "Expert", each setting the
 * width, height, and bomb count for the game, and starting a new game.
 *
 * Adds a "Customize" option that opens a dialog for custom game settings.
 *
 * @param bar The Upp::Bar object that represents the menu bar to which
 * the game menu options are added.
 */
void Board::GameMenu(Upp::Bar& bar) {
  bar.Add(Upp::t_("New Game"), [] { NewGame(); });
  bar.Sub(Upp::t_("Difficulty"), [](Upp::Bar& bar) {
    bar.Add(Upp::t_("Beginner"), [] {
      w = 9;
      h = 9;
      b = 10;
      NewGame();
    });
    bar.Add(Upp::t_("Intermediate"), [] {
      w = 16;
      h = 16;
      b = 40;
      NewGame();
    });
    bar.Add(Upp::t_("Expert"), [] {
      w = 30;
      h = 16;
      b = 99;
      NewGame();
    });
    bar.Add(Upp::t_("Customize"), [] {
      CustomizeDlg dlg;
      dlg.Run();
    });
  });
}

#define IMAGECLASS IconImg
#define IMAGEFILE <MineSweeper/icon.iml>
#include <Draw/iml.h>

/**
 * Constructs a new Board with the specified width, height, and bomb count.
 *
 * Creates a top-level window with a menu bar and a status bar.
 *
 * Sets the window's size to fit the specified width and height, with a little
 * bit of extra room for the frame.
 *
 * Initializes an array of width * height cells, with all cells initially
 * uncovered and not marked.
 *
 * Randomly selects BOMB_COUNT cells to be bombs.
 */
Board::Board(int w, int h, int b) : width(w), height(h), BOMB_COUNT(b) {
  Title(Upp::t_("MineSweeper"));
  Icon(IconImg::Icon());

  AddFrame(appMenu);
  appMenu.Set(THISBACK(Menu));

  safeCells = width * height - BOMB_COUNT;

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
  for (int i = 0; i < BOMB_COUNT; i++) {
    int x, y;
    do {
      x = std::rand() % width;
      y = std::rand() % height;
    } while (cells[x][y].isBomb);
    cells[x][y].isBomb = true;
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
  UncoverCell(p.x, p.y);
  Update();
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
      w.DrawRect(i * CELL_SIZE, j * CELL_SIZE, CELL_SIZE, CELL_SIZE, cellColor);

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
        w.DrawImage(i * CELL_SIZE, j * CELL_SIZE, CELL_SIZE, CELL_SIZE,
                    IconImg::Flag());
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
    Upp::PromptOK(Upp::t_("Game Over"));
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
      Upp::PromptOK(Upp::t_("You Win!"));
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
  if (cells[x][y].marked)
    markedCells++;
  else
    markedCells--;
}

Upp::String Board::FormatStatusString() {
  return Upp::Format(Upp::t_("%d bombs, %d marked, %d safe cells remaining"),
                     BOMB_COUNT, markedCells, safeCells);
}
