#include "TileCanvas.hpp"

#include <epoxy/gl.h>
#include <wx/glcanvas.h>
#include <wx/wx.h>

#include "DX8Graphics.hpp"
#include "Tileengine.hpp"
BEGIN_EVENT_TABLE(TileCanvas, wxGLCanvas)
EVT_PAINT(TileCanvas::PaintIt)
END_EVENT_TABLE()

TileCanvas::TileCanvas(wxPanel* parent)
    : wxGLCanvas(parent, wxID_ANY, nullptr, wxDefaultPosition, wxDefaultSize, 0,
                 wxT("GLCanvas")) {
  DirectGraphics.Init(640, 480, 32, false);

  context = new wxGLContext(this);
  context->SetCurrent(*this);

  if (!DirectGraphics.SetDeviceInfo()) {
    Protokoll << "\n-> SetDeviceInfo failed!\n" << std::endl;
    GameRunning = false;
  }

  Protokoll << "\n-> OpenGL init successful!\n" << std::endl;

  TileEngine.LoadSprites();
}

void TileCanvas::Render() {}
