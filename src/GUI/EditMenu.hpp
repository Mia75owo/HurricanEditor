#ifndef EDIT_MENU_HPP_
#define EDIT_MENU_HPP_

#include <wx/wx.h>
#include "GUI/TileMap.hpp"

class EditMenu : public wxPanel {
public:
  EditMenu(wxPanel* parent);

private:
  TileMap* tileMap;
};

#endif
