#include "TileMap.hpp"

#include <wx/wx.h>
#include "Tileengine.hpp"

BEGIN_EVENT_TABLE(TileMap, wxPanel)
EVT_PAINT(TileMap::PaintIt)
END_EVENT_TABLE()

TileMap::TileMap(wxWindow* parent) : wxPanel(parent) {
  Bind(wxEVT_SIZE, [&](wxSizeEvent& evt) {
    Refresh();
    evt.Skip();
  });
}

bool TileMap::LoadTileMap(wxString path, wxBitmapType type) {
  bool success = image.LoadFile(path, type);

  if (success) {
    int sizeX = TILESETSIZE_X - static_cast<int>(TILESETSIZE_X) % ORIGINAL_TILE_SIZE_X;
    int sizeY = TILESETSIZE_X - static_cast<int>(TILESETSIZE_Y) % ORIGINAL_TILE_SIZE_Y;
    image = image.GetSubImage(wxRect(0, 0, sizeX, sizeY));
  }

  return success;
}

void TileMap::Render(wxDC& dc) {
  dc.DrawBitmap(resized, 0, 0, false);
}
void TileMap::Resize(wxDC& dc) {
  int newSize = dc.GetSize().GetWidth();
  if (newSize != size) {
    size = newSize;
    resized = wxBitmap(image.Scale(size, size/*, wxIMAGE_QUALITY_HIGH*/));
  }
}
