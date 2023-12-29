#include "EditMenu.hpp"
#include "GUI/TileSet.hpp"
#include "Tileengine.hpp"

#include <wx/wx.h>

EditMenu::EditMenu(wxWindow* parent) : wxPanel(parent) {
  tileSet = new TileSet(this);
  sizer = new wxBoxSizer(wxVERTICAL);
}

void EditMenu::Init() {
  for (auto& tileSetPath : TileEngine.LoadedTilesetPaths) {
    // TODO: make non hardcoded
    tileSet->LoadTileSet(wxString::Format("../data/textures/%s", tileSetPath), wxBITMAP_TYPE_PNG);
  }
  tileSet->SetBackgroundColour(wxColor(0, 0, 0));

  sizer->Add(tileSet, 1, wxSHAPED);
  this->SetSizerAndFit(sizer);
}
