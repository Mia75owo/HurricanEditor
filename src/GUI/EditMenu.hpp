#ifndef EDIT_MENU_HPP_
#define EDIT_MENU_HPP_

#include <wx/wx.h>
#include "GUI/TileMap.hpp"

class EditMenu : public wxPanel {
public:
  EditMenu(wxWindow* parent);
  void Init();

private:
  TileMap* tileMap;

  wxBoxSizer* sizer;
};

#endif
