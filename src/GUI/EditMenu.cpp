#include "EditMenu.hpp"
#include "GUI/App.hpp"
#include "GUI/IDs.hpp"
#include "GUI/TileSet.hpp"
#include "Tileengine.hpp"

#include <wx/wx.h>

EditMenu::EditMenu(wxWindow* parent) : wxPanel(parent) {
  tileSet = new TileSet(this);
  sizer = new wxBoxSizer(wxVERTICAL);
  //tileSetSizer = new wxBoxSizer(wxVERTICAL);
  //controlsSizer = new wxBoxSizer(wxVERTICAL);

  controls = new wxPanel(this);
  positionText = new wxStaticText(controls, wxID_ANY, "", wxPoint(0, 40));;
  setsChoice = new wxChoice(controls, ID_TILESET_CHOICE, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT);
}

void EditMenu::Init() {
  tileSet->SetBackgroundColour(wxColor(0, 0, 0));
  controls->SetBackgroundColour(wxColor(200, 100, 100));

  for (auto& tileSetName : TileEngine.LoadedTilesetPaths) {
    // TODO: make non hardcoded
    tileSet->LoadTileSet(wxString::Format("../data/textures/%s", tileSetName), wxBITMAP_TYPE_PNG);
    setsChoice->Append(tileSetName);
  }
  setsChoice->Select(0);

  sizer->Add(tileSet, 0, wxSHAPED|wxEXPAND);
  sizer->Add(controls, 1);

  sizer->SetSizeHints(this);

  this->SetSizerAndFit(sizer);

  // =====
  // BINDS
  // =====

  setsChoice->Bind(wxEVT_CHOICE, [&](wxCommandEvent&) {
    int selected = setsChoice->GetCurrentSelection();
    wxString name = setsChoice->GetString(selected);
    tileSet->Select(name);
  });
  frame->canvas->Bind(wxEVT_MOTION, [&](wxMouseEvent& evt) {
    auto cords = frame->canvas->GetTileCordsUnderCursor();
    auto str = wxString::Format("Tile: %ix%i", cords.x, cords.y);
    positionText->SetLabel(str);
    evt.Skip();
  });
}
