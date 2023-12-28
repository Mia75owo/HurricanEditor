#ifndef TILE_CANVAS_HPP_
#define TILE_CANVAS_HPP_

#include <epoxy/gl.h>

#include <wx/glcanvas.h>
#include <wx/wx.h>

class TileCanvas : public wxGLCanvas {
 public:
  TileCanvas(wxPanel* parent);
  void PaintIt(wxPaintEvent&) { Render(); }

private:
  void Render();
  wxGLContext* context;

  wxPoint mousePos;
  bool mouseLeft;
  bool mouseRight;

protected:
  DECLARE_EVENT_TABLE()
};

#endif
