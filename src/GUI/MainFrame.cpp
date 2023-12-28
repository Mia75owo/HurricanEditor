#include "MainFrame.hpp"

#include <wx/event.h>
#include <wx/wx.h>

#include "GUI/TileCanvas.hpp"

enum IDs {
  BUTTON_ID = 2,
  SLIDER_ID,
  TEXT_ID,
};

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title) {
  auto panel = new wxPanel(this);

  auto bar = CreateStatusBar();
  bar->SetDoubleBuffered(true);

  auto tileCanvas = new TileCanvas(panel);
}
