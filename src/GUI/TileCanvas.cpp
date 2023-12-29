#include "TileCanvas.hpp"

#include <epoxy/gl.h>
#include <wx/event.h>
#include <wx/glcanvas.h>
#include <wx/window.h>
#include <wx/wx.h>

#include "DX8Graphics.hpp"
#include "ObjectList.hpp"
#include "Tileengine.hpp"
#include "Timer.hpp"

BEGIN_EVENT_TABLE(TileCanvas, wxGLCanvas)
EVT_PAINT(TileCanvas::PaintIt)
END_EVENT_TABLE()

TileCanvas::TileCanvas(wxWindow* parent)
    : wxGLCanvas(parent, wxID_ANY, nullptr, wxDefaultPosition, wxDefaultSize, 0,
                 wxT("GLCanvas")) {
  DirectGraphics.Init();

  context = new wxGLContext(this);
  context->SetCurrent(*this);

  if (!DirectGraphics.SetDeviceInfo()) {
    Protokoll << "\n-> SetDeviceInfo failed!\n" << std::endl;
    GameRunning = false;
  }

  Protokoll << "\n-> OpenGL init successful!\n" << std::endl;

  TileEngine.LoadSprites();
  TileEngine.LoadLevel(g_storage_ext + "/data/levels/jungle.map");

  Bind(wxEVT_SIZE, [&](wxSizeEvent&){
    auto size = this->GetSize();
    DirectGraphics.ResizeToWindow(size.GetWidth(), size.GetHeight());
  });

  mouseLeft = false;
  mouseRight = false;
  Bind(wxEVT_LEFT_DOWN, [&](wxMouseEvent& evt) {
    mouseLeft = true;
    evt.Skip();
  });
  Bind(wxEVT_LEFT_UP, [&](wxMouseEvent& evt) {
    mouseLeft = false;
    evt.Skip();
  });
  Bind(wxEVT_RIGHT_DOWN, [&](wxMouseEvent& evt) {
    mouseRight = true;
    evt.Skip();
  });
  Bind(wxEVT_RIGHT_UP, [&](wxMouseEvent& evt) {
    mouseRight = false;
    evt.Skip();
  });


  Bind(wxEVT_MOTION, [&](wxMouseEvent& evt) {
    if (evt.Dragging() && mouseRight) {
      auto delta = mousePos - evt.GetPosition();

      TileEngine.XOffset += delta.x;
      TileEngine.YOffset += delta.y;
    }

    mousePos = evt.GetPosition();
  });
  Bind(wxEVT_MOUSEWHEEL, [&](wxMouseEvent& evt) {
    if (evt.GetWheelRotation() > 0) {
      TileEngine.ZoomBy(0.1);
    } else {
      TileEngine.ZoomBy(-0.1);
    }
  });
}

wxPoint TileCanvas::GetTileCordsUnderCursor() {
  const wxPoint scaledOffset =
      wxPoint(TileEngine.XOffset / TileEngine.TileSizeX,
              TileEngine.YOffset / TileEngine.TileSizeY);
  const wxPoint scaledMouse = wxPoint(mousePos.x / TileEngine.TileSizeX,
                                      mousePos.y / TileEngine.TileSizeY);

  const wxPoint relative = scaledMouse + scaledOffset;

return relative;
}

void TileCanvas::Update() {
  Timer.update();
  TileEngine.UpdateLevel();
  TileEngine.CalcRenderRange();
}

void TileCanvas::Render() {
  context->SetCurrent(*this);

  DirectGraphics.ClearBackBuffer();
  DirectGraphics.SetColorKeyMode();

  TileEngine.DrawBackground();

  TileEngine.DrawBackLevel();
  TileEngine.DrawFrontLevel();

  ObjectList.DrawAllObjects(TileEngine.XOffset, TileEngine.YOffset, TileEngine.Scale);

  DirectGraphics.SetColorKeyMode();

  TileEngine.DrawWater();
  TileEngine.DrawBackLevelOverlay();
  TileEngine.DrawOverlayLevel();
  //TileEngine.DrawShadow();

  glFlush();
  SwapBuffers();
}
