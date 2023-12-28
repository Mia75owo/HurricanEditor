#include "App.hpp"

#include <wx/wx.h>

#include "DX8Graphics.hpp"
#include "DX8Texture.hpp"
#include "Logdatei.hpp"
#include "MainFrame.hpp"
#include "ObjectList.hpp"
#include "Tileengine.hpp"
#include "Timer.hpp"

wxIMPLEMENT_APP(App);

bool GameRunning = true;
std::string g_storage_ext = "/home/mia/code/Editor";

Logdatei Protokoll("logdatei.txt");
DirectGraphicsClass DirectGraphics;
TexturesystemClass Textures;
TimerClass Timer;
TileEngineClass TileEngine;
ObjectListClass ObjectList;

const Uint8* KeyBuffer;
int NumberOfKeys;

bool App::OnInit() {
  frame = new MainFrame("Hurrican Editor");
  frame->SetClientSize(800, 600);
  frame->Center();
  frame->Show();

  frame->Init();
  return true;
}
