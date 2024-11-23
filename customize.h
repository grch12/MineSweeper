#ifndef _MineSweeper_customize_h_
#define _MineSweeper_customize_h_

#include <CtrlLib/CtrlLib.h>

#define LAYOUTFILE <MineSweeper/dlg.lay>
#include <CtrlCore/lay.h>

class CustomizeDlg : public WithCustomizeDlgLayout<Upp::TopWindow> {
 public:
  CustomizeDlg();
};

#endif
