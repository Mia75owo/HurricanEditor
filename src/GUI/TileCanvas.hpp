#ifndef TILE_CANVAS_HPP_
#define TILE_CANVAS_HPP_

#include <epoxy/gl.h>

#include <wx/glcanvas.h>
#include <wx/wx.h>
#include "Tileengine.hpp"

enum EditMode {
  EDIT_MODE_FRONT,
  EDIT_MODE_BACK,
  EDIT_MODE_OBJECTS,
  EDIT_MODE_VIEW,
};

class TileCanvas : public wxGLCanvas {
 public:
  TileCanvas(wxWindow* parent);
  void PaintIt(wxPaintEvent&) { Update(); Render(); }

  wxPoint GetTileCordsUnderCursor();

  EditMode editMode;

private:
  void Update();
  void Render();

  void PlaceBlock(wxPoint pos, LevelTileStruct tile);
  void PlaceTileFront(wxPoint pos, unsigned char art, unsigned char tileSet, uint32_t flags);
  void PlaceTileBack(wxPoint pos, unsigned char art, unsigned char tileSet, uint32_t flags);

  wxGLContext* context;

  wxPoint mousePos;
  bool mouseLeft;
  bool mouseRight;

protected:
  DECLARE_EVENT_TABLE()
};

#endif
