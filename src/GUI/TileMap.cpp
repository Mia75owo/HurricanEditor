#include "TileMap.hpp"

#include <wx/wx.h>

BEGIN_EVENT_TABLE(TileMap, wxPanel)
EVT_PAINT(TileMap::PaintIt)
END_EVENT_TABLE()

TileMap::TileMap(wxPanel* parent) : wxPanel(parent) {}

bool TileMap::LoadTileMap(wxString path, wxBitmapType type) {
  return image.LoadFile(path, type);
}

void TileMap::Update() {}
void TileMap::Render() {
  wxPaintDC dc(this);
  dc.DrawBitmap(image, 0, 0, false);
}
