#ifndef MAIN_FRAME_HPP_
#define MAIN_FRAME_HPP_

#include <epoxy/gl.h>
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include "GUI/EditMenu.hpp"
#include "GUI/TileCanvas.hpp"

class MainFrame : public wxFrame {
 public:
  MainFrame(const wxString& title);
  void Init();

  TileCanvas* canvas;
  EditMenu* editMenu;

 private:
  wxPanel* panel;
  wxSizer* sizer;
  wxSizer* mainSizer;
};

#endif
