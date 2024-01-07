#ifndef EDIT_MENU_HPP_
#define EDIT_MENU_HPP_

#include <wx/wrapsizer.h>
#include <wx/wx.h>

#include <cstdint>

#include "GUI/TileSet.hpp"

class EditMenu : public wxPanel {
 public:
  EditMenu(wxWindow* parent);
  void Init();

  TileSet* tileSet;

 private:
  wxPanel* controls;
  wxChoice* setsChoice;

  wxStaticBox* tileConfigBox;
  wxStaticBoxSizer* tileConfigBoxSizer;
  wxWrapSizer* tileConfigSizer;

  wxCheckBox* wall;
  wxCheckBox* enemyWall;
  wxCheckBox* platform;
  wxCheckBox* light;
  wxCheckBox* overlay;
  wxCheckBox* animatedBack;
  wxCheckBox* animatedFront;
  wxCheckBox* water;
  wxCheckBox* damage;
  wxCheckBox* conveyorL;
  wxCheckBox* conveyorR;
  wxCheckBox* turn;
  wxCheckBox* destroyable;
  wxCheckBox* moveL;
  wxCheckBox* overlayLight;
  wxCheckBox* swamp;
  wxCheckBox* ice;
  wxCheckBox* moveD;
  wxCheckBox* waterfall;
  wxCheckBox* moveR;
  wxCheckBox* slopeL;
  wxCheckBox* slopeR;
  wxCheckBox* liquid;

  wxStaticBox* tileInfoBox;
  wxStaticText* positionText;

  wxBoxSizer* sizer;
  wxBoxSizer* setsChoiceSizer;
  wxStaticBoxSizer* tileInfoBoxSizer;
};

#endif
