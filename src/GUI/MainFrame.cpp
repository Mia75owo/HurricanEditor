#include "MainFrame.hpp"

#include <wx/wx.h>

#include "GUI/EditMenu.hpp"
#include "GUI/IDs.hpp"
#include "GUI/TileCanvas.hpp"
#include "Tileengine.hpp"

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title) {
  auto menuFile = new wxMenu;
  menuFile->Append(ID_LOAD, "&Load Map", "Opens a Hurrican map file");
  menuFile->AppendSeparator();
  menuFile->Append(ID_SAVE, "&Save Map", "Saves a Hurrican map file to a path");
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);
  auto menuEditor = new wxMenu;
  menuEditor->Append(ID_RESET_ZOOM, "&Reset Zoom",
                     "Sets the Zoom of the Level back to 1.0");
  menuEditor->Append(ID_EDITOR_MODE_FRONT, "&EM: front",
                     "Changes the editor mode to 'front'");
  menuEditor->Append(ID_EDITOR_MODE_BACK, "&EM: back",
                     "Changes the editor mode to 'back'");
  menuEditor->Append(ID_EDITOR_MODE_OBJECTS, "&EM: objects",
                     "Changes the editor mode to 'objects'");
  menuEditor->Append(ID_EDITOR_MODE_VIEW, "&EM: view",
                     "Changes the editor mode to 'view'");

  auto menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuEditor, "&Editor");
  SetMenuBar(menuBar);

  // clang-format off
  Bind(wxEVT_MENU, [&](auto&) { LoadLevel(); }, ID_LOAD);
  Bind(wxEVT_MENU, [&](auto&) { SaveLevel(); }, ID_SAVE);

  Bind(wxEVT_MENU, [&](auto&) { ResetZoom(); }, ID_RESET_ZOOM);
  Bind(wxEVT_MENU, [&](auto&) { canvas->editMode = EDIT_MODE_FRONT; },
      ID_EDITOR_MODE_FRONT);
  Bind(wxEVT_MENU, [&](auto&) { canvas->editMode = EDIT_MODE_BACK; },
      ID_EDITOR_MODE_BACK);
  Bind(wxEVT_MENU, [&](auto&) { canvas->editMode = EDIT_MODE_OBJECTS; },
      ID_EDITOR_MODE_OBJECTS);
  Bind(wxEVT_MENU, [&](auto&) { canvas->editMode = EDIT_MODE_VIEW; },
      ID_EDITOR_MODE_VIEW);
  // clang-format on

  mainSplitter =
      new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                           wxSP_BORDER | wxSP_LIVE_UPDATE);

  sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(mainSplitter, 1, wxEXPAND);
  this->SetSizerAndFit(sizer);
}

void MainFrame::LoadLevel() {
  wxFileDialog fileDialog(this, _("Open map File"), "../data/levels", "",
                          "map files (*.map)|*.map",
                          wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (fileDialog.ShowModal() == wxID_CANCEL) return;
  Protokoll << fileDialog.GetPath().ToStdString() << std::endl;
  TileEngine.LoadLevel(fileDialog.GetPath().ToStdString());
}

void MainFrame::SaveLevel() {
  wxFileDialog fileDialog(this, _("Save map File"), "../data/levels", "",
                          "map files (*.map)|*.map",
                          wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  if (fileDialog.ShowModal() == wxID_CANCEL) return;
  TileEngine.SaveLevel(fileDialog.GetPath().ToStdString());
}

void MainFrame::ResetZoom() { TileEngine.ZoomBy(1.0f - TileEngine.Scale); }

void MainFrame::Init() {
  canvas = new TileCanvas(mainSplitter);

  editMenu = new EditMenu(mainSplitter);
  editMenu->SetBackgroundColour(wxColor(100, 200, 100));

  mainSplitter->SplitVertically(canvas, editMenu);
  mainSplitter->SetMinimumPaneSize(200);
  mainSplitter->SetSashGravity(1.0);

  editMenu->Init();
}
