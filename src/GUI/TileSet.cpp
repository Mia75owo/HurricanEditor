#include "TileSet.hpp"

#include <wx/wx.h>
#include "Tileengine.hpp"

BEGIN_EVENT_TABLE(TileSet, wxPanel)
EVT_PAINT(TileSet::PaintIt)
END_EVENT_TABLE()

TileSet::TileSet(wxWindow* parent) : wxPanel(parent) {
  Bind(wxEVT_SIZE, [&](wxSizeEvent& evt) {
    Refresh();
    evt.Skip();
  });
}

bool TileSet::LoadTileSet(wxString path, wxBitmapType type) {
  bool allreadyLoaded = std::any_of(images.cbegin(), images.cend(), [&](const TileImage& img){
    return img.Path == path;
  });
  if (allreadyLoaded)
    return true;

  TileImage tileImage;
  bool success = tileImage.Image.LoadFile(path, type);

  if (success) {
    int sizeX = TILESETSIZE_X - static_cast<int>(TILESETSIZE_X) % ORIGINAL_TILE_SIZE_X;
    int sizeY = TILESETSIZE_X - static_cast<int>(TILESETSIZE_Y) % ORIGINAL_TILE_SIZE_Y;
    tileImage.Image = tileImage.Image.GetSubImage(wxRect(0, 0, sizeX, sizeY));
    tileImage.Path = path;
    images.push_back(tileImage);
  }

  return success;
}

void TileSet::Render(wxDC& dc) {
  dc.DrawBitmap(resized, 0, 0, false);
}
void TileSet::Resize(wxDC& dc) {
  int newSize = dc.GetSize().GetWidth();
  if (newSize != size) {
    size = newSize;
    resized = wxBitmap(images[0].Image.Scale(size, size/*, wxIMAGE_QUALITY_HIGH*/));
  }
}
