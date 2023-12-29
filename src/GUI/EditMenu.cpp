#include "EditMenu.hpp"
#include "GUI/TileSet.hpp"
#include "Tileengine.hpp"

#include <wx/wx.h>

EditMenu::EditMenu(wxWindow* parent) : wxPanel(parent) {
  tileSet = new TileSet(this);
  sizer = new wxBoxSizer(wxVERTICAL);
  controls = new wxPanel(this);
  positionText = new wxStaticText(controls, wxID_ANY, "", wxPoint(0, 40));;
}

void EditMenu::Init() {
  for (auto& tileSetPath : TileEngine.LoadedTilesetPaths) {
    // TODO: make non hardcoded
    tileSet->LoadTileSet(wxString::Format("../data/textures/%s", tileSetPath), wxBITMAP_TYPE_PNG);
  }
  tileSet->SetBackgroundColour(wxColor(0, 0, 0));

  sizer->Add(tileSet, 1, wxSHAPED);
  sizer->Add(controls);
  this->SetSizerAndFit(sizer);

  // =====
  // BINDS
  // =====

  frame->canvas->Bind(wxEVT_MOTION, [&](wxMouseEvent& evt) {
    auto cords = frame->canvas->GetTileCordsUnderCursor();
    auto str = wxString::Format("Tile: %ix%i", cords.x, cords.y);
    positionText->SetLabel(str);
    evt.Skip();
  });
}
