#include "board.h"
#include "customize.h"
#include "utils.h"

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
        "cartoon sea mine] by rg1024&Flag Image: "
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
