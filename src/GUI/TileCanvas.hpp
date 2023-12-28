#ifndef TILE_CANVAS_HPP_
#define TILE_CANVAS_HPP_

#include <epoxy/gl.h>

#include <wx/glcanvas.h>
#include <wx/wx.h>

class TileCanvas : public wxGLCanvas {
  void Render();

  wxGLContext* context;

 public:
  TileCanvas(wxPanel* parent);
  void PaintIt(wxPaintEvent&) { Render(); }
};

#endif
