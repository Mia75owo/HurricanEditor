#ifndef EDIT_MENU_HPP_
#define EDIT_MENU_HPP_

#include <wx/wx.h>
#include "GUI/TileSet.hpp"

class EditMenu : public wxPanel {
public:
  EditMenu(wxWindow* parent);
  void Init();

  TileSet* tileSet;
private:

  wxPanel* controls;
  wxChoice* setsChoice;

  wxStaticBox* tileInfoBox;
  wxStaticText* positionText;

  wxBoxSizer* sizer;
  wxBoxSizer* setsChoiceSizer;
  wxStaticBoxSizer* tileInfoBoxSizer;
};

#endif
