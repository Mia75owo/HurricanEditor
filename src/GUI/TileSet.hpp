#ifndef TILE_SET_HPP_
#define TILE_SET_HPP_

#include <wx/bmpbndl.h>
#include <wx/wx.h>
#include <map>

struct LoadedTileSet {
  wxImage image;
  int tileSetID;
};

class TileSet : public wxPanel {
 public:
  TileSet(wxWindow* parent);
  bool LoadTileSet(wxString path, int tileSetID, wxBitmapType type);

  void PaintIt(wxPaintEvent&) {
    wxPaintDC dc(this);
    Resize(dc);
    Render(dc);
  }

  void Select(wxString name);
  void SetSelectedTileID(int tileID);
  int GetSelectedTileID();
  int GetSelectedTileSetID();

 private:
  std::map<wxString, LoadedTileSet> images;
  wxString currentImage;

  wxBitmap resized;

  void Render(wxDC& dc);
  void Resize(wxDC& dc);
  int GetTileUnderCursor(wxPoint cursor);

  int size;
  int selectedTile;

  wxPoint mousePos;

 protected:
  DECLARE_EVENT_TABLE()
};

#endif
