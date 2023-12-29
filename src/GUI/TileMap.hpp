#ifndef TILE_MAP_HPP_
#define TILE_MAP_HPP_

#include <wx/wx.h>

class TileMap : public wxPanel {
 public:
  TileMap(wxPanel* parent);
  bool LoadTileMap(wxString path, wxBitmapType type);

  void PaintIt(wxPaintEvent&) {
    Update();
    Render();
  }

 private:
  wxBitmap image;

  void Update();
  void Render();

 protected:
  DECLARE_EVENT_TABLE()
};

#endif
