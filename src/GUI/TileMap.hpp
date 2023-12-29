#ifndef TILE_MAP_HPP_
#define TILE_MAP_HPP_

#include <wx/bmpbndl.h>
#include <wx/wx.h>

struct TileImage {
  wxString Path;
  wxImage Image;
};

class TileMap : public wxPanel {
 public:
  TileMap(wxWindow* parent);
  bool LoadTileMap(wxString path, wxBitmapType type);

  void PaintIt(wxPaintEvent&) {
    wxPaintDC dc(this);
    Resize(dc);
    Render(dc);
  }

 private:
  std::vector<TileImage> images;

  wxBitmap resized;

  void Render(wxDC& dc);
  void Resize(wxDC& dc);

  int size;

 protected:
  DECLARE_EVENT_TABLE()
};

#endif
