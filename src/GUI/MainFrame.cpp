#include "MainFrame.hpp"

#include <wx/event.h>
#include <wx/wx.h>

#include "GUI/TileCanvas.hpp"
#include "Tileengine.hpp"

enum IDs {
};

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title) {

  panel = new wxPanel(this);

  sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(panel, 1, wxEXPAND);

  this->SetSizerAndFit(sizer);
}

void MainFrame::Init() {
  canvas = new TileCanvas(panel);
  canvasSizer = new wxBoxSizer(wxVERTICAL);
  canvasSizer->Add(canvas, 1, wxEXPAND);
  panel->SetSizerAndFit(canvasSizer);
}
