#include "EditMenu.hpp"
#include "GUI/TileMap.hpp"

#include <wx/wx.h>

EditMenu::EditMenu(wxPanel* parent) : wxPanel(parent) {
  wxInitAllImageHandlers();

  tileMap = new TileMap(this);
  tileMap->LoadTileMap(wxT("../data/textures/s_Jungle.png"), wxBITMAP_TYPE_PNG);

  sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(tileMap, 1, wxEXPAND);
  this->SetSizerAndFit(sizer);
}
