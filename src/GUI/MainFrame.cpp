#include "MainFrame.hpp"

#include <wx/event.h>
#include <wx/wx.h>

#include "GUI/TileCanvas.hpp"
#include "Tileengine.hpp"

enum IDs {
};

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title) {

  auto bar = CreateStatusBar();
  bar->SetDoubleBuffered(true);

  panel = new wxPanel(this);


void MainFrame::Init() {
  canvas = new TileCanvas(panel);
}
