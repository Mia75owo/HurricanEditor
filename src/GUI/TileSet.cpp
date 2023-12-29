#include "TileSet.hpp"

#include <wx/wx.h>
#include <wx/filename.h>
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
  auto filename = wxFileName(path).GetFullName();

  bool allreadyLoaded = images.find(filename) != images.end();
  if (allreadyLoaded)
    return true;

  wxImage image;

  bool success = image.LoadFile(path, type);

  if (success) {
    int sizeX = TILESETSIZE_X - static_cast<int>(TILESETSIZE_X) % ORIGINAL_TILE_SIZE_X;
    int sizeY = TILESETSIZE_X - static_cast<int>(TILESETSIZE_Y) % ORIGINAL_TILE_SIZE_Y;
    image = image.GetSubImage(wxRect(0, 0, sizeX, sizeY));

    bool firstLoaded = images.empty();

    images.emplace(filename, image);

    if (firstLoaded) 
      currentImage = filename;
  }

  return success;
}

void TileSet::Select(wxString name) {
  auto idx = images.find(name);
  if (idx == images.end())
    return;

  currentImage = name;

  resized = wxBitmap(images[currentImage].Scale(size, size/*, wxIMAGE_QUALITY_HIGH*/));

  Refresh();
}

void TileSet::Render(wxDC& dc) {
  dc.DrawBitmap(resized, 0, 0, false);
}
void TileSet::Resize(wxDC& dc) {
  int newSize = dc.GetSize().GetWidth();
  if (newSize != size) {
    size = newSize;
    resized = wxBitmap(images[currentImage].Scale(size, size/*, wxIMAGE_QUALITY_HIGH*/));
  }
}
