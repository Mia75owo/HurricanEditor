#ifndef APP_HPP_
#define APP_HPP_

#include <epoxy/gl.h>
#include <wx/wx.h>
#include "GUI/MainFrame.hpp"

class App : public wxApp {
 public:
  bool OnInit();

 private:
  void OnIdle(wxIdleEvent& evt);
};

extern MainFrame* frame;

#endif
