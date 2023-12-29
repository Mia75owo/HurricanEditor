#include "EditMenu.hpp"
#include "GUI/TileMap.hpp"
#include "Tileengine.hpp"

#include <wx/wx.h>

EditMenu::EditMenu(wxWindow* parent) : wxPanel(parent) {
  tileMap = new TileMap(this);
  sizer = new wxBoxSizer(wxVERTICAL);
}

void EditMenu::Init() {
  for (auto& tileMapPath : TileEngine.LoadedTilesetPaths) {
    // TODO: make non hardcoded
    tileMap->LoadTileMap(wxString::Format("../data/textures/%s", tileMapPath), wxBITMAP_TYPE_PNG);
  }
  tileMap->SetBackgroundColour(wxColor(0, 0, 0));

  sizer->Add(tileMap, 1, wxSHAPED);
  this->SetSizerAndFit(sizer);
}
