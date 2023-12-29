#include "EditMenu.hpp"
#include "GUI/TileMap.hpp"

#include <wx/wx.h>

EditMenu::EditMenu(wxWindow* parent) : wxPanel(parent) {
  wxInitAllImageHandlers();

  tileMap = new TileMap(this);
  sizer = new wxBoxSizer(wxVERTICAL);
}

void EditMenu::Init() {
  tileMap->LoadTileMap(wxT("../data/textures/s_Jungle.png"), wxBITMAP_TYPE_PNG);
  tileMap->SetBackgroundColour(wxColor(0, 0, 0));

  sizer->Add(tileMap, 1, wxSHAPED);
  this->SetSizerAndFit(sizer);
}
