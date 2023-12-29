#ifndef TILE_CANVAS_HPP_
#define TILE_CANVAS_HPP_

#include <epoxy/gl.h>

#include <wx/glcanvas.h>
#include <wx/wx.h>

class TileCanvas : public wxGLCanvas {
 public:
  TileCanvas(wxWindow* parent);
  void PaintIt(wxPaintEvent&) { Update(); Render(); }

  wxPoint GetTileCordsUnderCursor();

private:
  void Update();
  void Render();
  wxGLContext* context;

  wxPoint mousePos;
  bool mouseLeft;
  bool mouseRight;

protected:
  DECLARE_EVENT_TABLE()
};

#endif
