#include "TileMap.hpp"

#include <wx/wx.h>

BEGIN_EVENT_TABLE(TileMap, wxPanel)
EVT_PAINT(TileMap::PaintIt)
END_EVENT_TABLE()

TileMap::TileMap(wxPanel* parent) : wxPanel(parent) {
  Bind(wxEVT_SIZE, [&](wxSizeEvent& evt) {
    Refresh();
    evt.Skip();
  });
}

bool TileMap::LoadTileMap(wxString path, wxBitmapType type) {
  return image.LoadFile(path, type);
}

void TileMap::Render(wxDC& dc) {
  dc.DrawBitmap(resized, 0, 0, false);
}
void TileMap::Resize(wxDC& dc) {
  auto newSize = dc.GetSize();
  if (newSize != size) {
    size.SetWidth(newSize.GetWidth());
    size.SetHeight(newSize.GetWidth());
    resized = wxBitmap(image.Scale(size.GetWidth(), size.GetHeight()/*, wxIMAGE_QUALITY_HIGH*/));
  }
}
