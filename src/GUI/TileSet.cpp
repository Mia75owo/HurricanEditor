#include "TileSet.hpp"

#include <wx/wx.h>
#include <wx/filename.h>
#include "Tileengine.hpp"

BEGIN_EVENT_TABLE(TileSet, wxPanel)
EVT_PAINT(TileSet::PaintIt)
END_EVENT_TABLE()

TileSet::TileSet(wxWindow* parent) : wxPanel(parent) {
  selectedTile = 0;
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

  // Render grid
  const int tilesX = floor(TILESETSIZE_X / ORIGINAL_TILE_SIZE_X);
  const int tilesY = floor(TILESETSIZE_Y / ORIGINAL_TILE_SIZE_Y);

  const float tileSizeX = size / static_cast<float>(tilesX);
  const float tileSizeY = size / static_cast<float>(tilesY);

  const int width = 3;

  for (int i = 1; i < tilesX; i++) {
    dc.DrawRectangle(i * tileSizeX - width / 2.0f, 0, width, size);
  }
  for (int i = 1; i < tilesY; i++) {
    dc.DrawRectangle(0, i * tileSizeY - width / 2.0f, size, width);
  }

  // Render selected highlight
  const int selectedX = selectedTile % tilesX;
  const int selectedY = selectedTile % tilesY;

  dc.SetBrush(wxBrush(wxColor(255, 255, 255, 100)));
  dc.DrawRectangle(selectedX - width / 2.0f, selectedY - width / 2.0f, tileSizeX + width, tileSizeY + width);
}
void TileSet::Resize(wxDC& dc) {
  int newSize = dc.GetSize().GetWidth();
  if (newSize != size) {
    size = newSize;
    resized = wxBitmap(images[currentImage].Scale(size, size/*, wxIMAGE_QUALITY_HIGH*/));
  }
}
