#include "MainFrame.hpp"

#include <wx/event.h>
#include <wx/wx.h>

#include "GUI/TileCanvas.hpp"
#include "Tileengine.hpp"

enum IDs {
  ID_LOAD = 2,
};

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title) {
  auto menuFile = new wxMenu;
  menuFile->Append(ID_LOAD, "&Load Map", "Opens a Hurrican map file");
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  auto menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
  SetMenuBar(menuBar);

  Bind(wxEVT_MENU, [&](wxCommandEvent&) {
    wxFileDialog fileDialog(this, _("Open map File"), "", "",
                       "map files (*.map)|*.map", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (fileDialog.ShowModal() == wxID_CANCEL)
      return;
    Protokoll << fileDialog.GetPath().ToStdString() << std::endl;
    TileEngine.LoadLevel(fileDialog.GetPath().ToStdString());
  }, ID_LOAD);
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
