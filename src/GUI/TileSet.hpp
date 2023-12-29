#ifndef TILE_SET_HPP_
#define TILE_SET_HPP_

#include <wx/bmpbndl.h>
#include <wx/wx.h>
#include <map>

class TileSet : public wxPanel {
 public:
  TileSet(wxWindow* parent);
  bool LoadTileSet(wxString path, wxBitmapType type);

  void PaintIt(wxPaintEvent&) {
    wxPaintDC dc(this);
    Resize(dc);
    Render(dc);
  }

  void Select(wxString name);

 private:
  std::map<wxString, wxImage> images;
  wxString currentImage;

  wxBitmap resized;

  void Render(wxDC& dc);
  void Resize(wxDC& dc);

  int size;

 protected:
  DECLARE_EVENT_TABLE()
};

#endif