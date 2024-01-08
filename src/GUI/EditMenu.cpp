#include "EditMenu.hpp"

#include <wx/wx.h>

#include "GUI/App.hpp"
#include "GUI/IDs.hpp"
#include "GUI/TileSet.hpp"
#include "Tileengine.hpp"

EditMenu::EditMenu(wxWindow* parent) : wxPanel(parent) {
  tileSet = new TileSet(this);
  sizer = new wxBoxSizer(wxVERTICAL);

  controls = new wxPanel(this);

  setsChoice = new wxChoice(controls, ID_TILESET_CHOICE, wxDefaultPosition,
                            wxDefaultSize, 0, NULL, wxCB_SORT);
  setsChoiceSizer = new wxBoxSizer(wxVERTICAL);

  tileConfigBox = new wxStaticBox(controls, wxID_ANY, "Tile Config:");
  tileConfigBoxSizer = new wxStaticBoxSizer(tileConfigBox, wxVERTICAL);
  tileConfigSizer = new wxWrapSizer(wxHORIZONTAL, wxREMOVE_LEADING_SPACES);

  // clang-format off
  wall = new wxCheckBox(tileConfigBox, wxID_ANY, "wall", wxDefaultPosition, wxSize(100, 20));
  enemyWall = new wxCheckBox(tileConfigBox, wxID_ANY, "enemy wall", wxDefaultPosition, wxSize(100, 20));
  platform = new wxCheckBox(tileConfigBox, wxID_ANY, "platform", wxDefaultPosition, wxSize(100, 20));
  light = new wxCheckBox(tileConfigBox, wxID_ANY, "light", wxDefaultPosition, wxSize(100, 20));
  overlay = new wxCheckBox(tileConfigBox, wxID_ANY, "overlay", wxDefaultPosition, wxSize(100, 20));
  animatedBack = new wxCheckBox(tileConfigBox, wxID_ANY, "anim back", wxDefaultPosition, wxSize(100, 20));
  animatedFront = new wxCheckBox(tileConfigBox, wxID_ANY, "anim front", wxDefaultPosition, wxSize(100, 20));
  water = new wxCheckBox(tileConfigBox, wxID_ANY, "water", wxDefaultPosition, wxSize(100, 20));
  damage = new wxCheckBox(tileConfigBox, wxID_ANY, "damage", wxDefaultPosition, wxSize(100, 20));
  conveyorL = new wxCheckBox(tileConfigBox, wxID_ANY, "conveyor L", wxDefaultPosition, wxSize(100, 20));
  conveyorR = new wxCheckBox(tileConfigBox, wxID_ANY, "conveyor R", wxDefaultPosition, wxSize(100, 20));
  turn = new wxCheckBox(tileConfigBox, wxID_ANY, "turn", wxDefaultPosition, wxSize(100, 20));
  destroyable = new wxCheckBox(tileConfigBox, wxID_ANY, "destroyable", wxDefaultPosition, wxSize(100, 20));
  moveL = new wxCheckBox(tileConfigBox, wxID_ANY, "move L", wxDefaultPosition, wxSize(100, 20));
  overlayLight = new wxCheckBox(tileConfigBox, wxID_ANY, "overlay light", wxDefaultPosition, wxSize(100, 20));
  swamp = new wxCheckBox(tileConfigBox, wxID_ANY, "swamp", wxDefaultPosition, wxSize(100, 20));
  ice = new wxCheckBox(tileConfigBox, wxID_ANY, "ice", wxDefaultPosition, wxSize(100, 20));
  moveD = new wxCheckBox(tileConfigBox, wxID_ANY, "move D", wxDefaultPosition, wxSize(100, 20));
  waterfall = new wxCheckBox(tileConfigBox, wxID_ANY, "waterfall", wxDefaultPosition, wxSize(100, 20));
  moveR = new wxCheckBox(tileConfigBox, wxID_ANY, "move R", wxDefaultPosition, wxSize(100, 20));
  slopeL = new wxCheckBox(tileConfigBox, wxID_ANY, "slope L", wxDefaultPosition, wxSize(100, 20));
  slopeR = new wxCheckBox(tileConfigBox, wxID_ANY, "slope R", wxDefaultPosition, wxSize(100, 20));
  liquid = new wxCheckBox(tileConfigBox, wxID_ANY, "liquid", wxDefaultPosition, wxSize(100, 20));
  // clang-format on

  tileInfoBox = new wxStaticBox(controls, wxID_ANY, "Tile Info:");
  tileInfoBoxSizer = new wxStaticBoxSizer(tileInfoBox, wxVERTICAL);
  positionText = new wxStaticText(tileInfoBox, wxID_ANY, "");
}

void EditMenu::Init() {
  tileSet->SetBackgroundColour(wxColor(0, 0, 0));
  controls->SetBackgroundColour(wxColor(200, 100, 100));

  for (auto& loadedTileSet : TileEngine.LoadedTilesetPathsWithID) {
    // TODO: make non hardcoded
    tileSet->LoadTileSet(
        wxString::Format("../data/textures/%s", loadedTileSet.first),
        loadedTileSet.second, wxBITMAP_TYPE_PNG);
    setsChoice->Append(loadedTileSet.first);
  }
  setsChoice->Select(0);

  auto font =
      wxFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  positionText->SetFont(font);

  tileConfigSizer->Add(wall, 0, wxEXPAND);
  tileConfigSizer->Add(enemyWall, 0, wxEXPAND);
  tileConfigSizer->Add(platform, 0, wxEXPAND);
  tileConfigSizer->Add(light, 0, wxEXPAND);
  tileConfigSizer->Add(overlay, 0, wxEXPAND);
  tileConfigSizer->Add(animatedBack, 0, wxEXPAND);
  tileConfigSizer->Add(animatedFront, 0, wxEXPAND);
  tileConfigSizer->Add(water, 0, wxEXPAND);
  tileConfigSizer->Add(damage, 0, wxEXPAND);
  tileConfigSizer->Add(conveyorL, 0, wxEXPAND);
  tileConfigSizer->Add(conveyorR, 0, wxEXPAND);
  tileConfigSizer->Add(turn, 0, wxEXPAND);
  tileConfigSizer->Add(destroyable, 0, wxEXPAND);
  tileConfigSizer->Add(moveL, 0, wxEXPAND);
  tileConfigSizer->Add(overlayLight, 0, wxEXPAND);
  tileConfigSizer->Add(swamp, 0, wxEXPAND);
  tileConfigSizer->Add(ice, 0, wxEXPAND);
  tileConfigSizer->Add(moveD, 0, wxEXPAND);
  tileConfigSizer->Add(waterfall, 0, wxEXPAND);
  tileConfigSizer->Add(moveR, 0, wxEXPAND);
  tileConfigSizer->Add(slopeL, 0, wxEXPAND);
  tileConfigSizer->Add(slopeR, 0, wxEXPAND);
  tileConfigSizer->Add(liquid, 0, wxEXPAND);

  tileConfigBoxSizer->Add(tileConfigSizer, 0, wxEXPAND);

  tileInfoBox->SetFont(font);
  tileInfoBoxSizer->Add(positionText, 0, wxEXPAND);

  setsChoiceSizer->Add(setsChoice, 0, wxEXPAND);
  setsChoiceSizer->Add(tileInfoBoxSizer, 0, wxEXPAND, 10);
  setsChoiceSizer->Add(tileConfigBoxSizer, 0, wxEXPAND, 10);
  controls->SetSizerAndFit(setsChoiceSizer);

  sizer->Add(tileSet, 0, wxSHAPED | wxEXPAND);
  sizer->Add(controls, 1, wxEXPAND);

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

uint32_t EditMenu::getBlockFlags() {
  uint32_t flags = 0;

  auto toFull = [](bool in) {
    if (in) {
      return 0xFFFFFFFF;
    } else {
      return (unsigned int)0x0;
    }
  };

  flags |= BLOCKWERT_WAND & toFull(wall->IsChecked());
  flags |= BLOCKWERT_GEGNERWAND & toFull(enemyWall->IsChecked());
  flags |= BLOCKWERT_PLATTFORM & toFull(platform->IsChecked());
  flags |= BLOCKWERT_LIGHT & toFull(light->IsChecked());
  flags |= BLOCKWERT_VERDECKEN & toFull(overlay->IsChecked());
  flags |= BLOCKWERT_ANIMIERT_BACK & toFull(animatedBack->IsChecked());
  flags |= BLOCKWERT_ANIMIERT_FRONT & toFull(animatedFront->IsChecked());
  flags |= BLOCKWERT_WASSER & toFull(water->IsChecked());
  flags |= BLOCKWERT_SCHADEN & toFull(damage->IsChecked());
  flags |= BLOCKWERT_FLIESSBANDL & toFull(conveyorL->IsChecked());
  flags |= BLOCKWERT_FLIESSBANDR & toFull(conveyorR->IsChecked());
  flags |= BLOCKWERT_WENDEPUNKT & toFull(turn->IsChecked());
  flags |= BLOCKWERT_DESTRUCTIBLE & toFull(destroyable->IsChecked());
  flags |= BLOCKWERT_MOVELINKS & toFull(moveL->IsChecked());
  flags |= BLOCKWERT_OVERLAY_LIGHT & toFull(overlayLight->IsChecked());
  flags |= BLOCKWERT_SUMPF & toFull(swamp->IsChecked());
  flags |= BLOCKWERT_EIS & toFull(ice->IsChecked());
  flags |= BLOCKWERT_MOVEVERTICAL & toFull(moveD->IsChecked());
  flags |= BLOCKWERT_WASSERFALL & toFull(waterfall->IsChecked());
  flags |= BLOCKWERT_MOVERECHTS & toFull(moveR->IsChecked());
  flags |= BLOCKWERT_SCHRAEGE_L & toFull(slopeL->IsChecked());
  flags |= BLOCKWERT_SCHRAEGE_R & toFull(slopeR->IsChecked());
  flags |= BLOCKWERT_LIQUID & toFull(liquid->IsChecked());

  return flags;
}
