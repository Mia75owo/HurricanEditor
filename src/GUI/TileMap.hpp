#ifndef TILE_MAP_HPP_
#define TILE_MAP_HPP_

#include <wx/wx.h>

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
  wxImage image;
  wxBitmap resized;

  void Render(wxDC& dc);
  void Resize(wxDC& dc);

  wxSize size;

 protected:
  DECLARE_EVENT_TABLE()
};

#endif
