#include <string>

#include "DX8Graphics.hpp"
#include "DX8Texture.hpp"
#include "Logdatei.hpp"
#include "Tileengine.hpp"
#include "Timer.hpp"
#include "ObjectList.hpp"

bool GameRunning = true;
std::string g_storage_ext = "/home/mia/code/Editor";

Logdatei Protokoll("logdatei.txt");
DirectGraphicsClass DirectGraphics;
TexturesystemClass Textures;
TimerClass Timer;
TileEngineClass TileEngine;
ObjectListClass ObjectList;

const Uint8 *KeyBuffer;
int NumberOfKeys;
#define KeyDown(key) (KeyBuffer[SDL_GetScancodeFromKey(key)] > 0)

int main() {
  DirectGraphics.Init(640, 480, 32, false);
  KeyBuffer = SDL_GetKeyboardState(&NumberOfKeys);

  TileEngine.LoadLevel(g_storage_ext + "/data/levels/temple.map");

  Timer.SetMaxFPS(60);
  Timer.update();

  int OldMouseX;
  int OldMouseY;
  int MouseX;
  int MouseY;
  unsigned int MouseMask;

  TileEngine.Scale = 1.0;

  while (GameRunning) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
        case SDL_QUIT:
          GameRunning = false;
          break;
        case SDL_WINDOWEVENT:
          if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
            DirectGraphics.ResizeToWindow();
          }
          break;
        case SDL_MOUSEWHEEL:
          if (ev.wheel.y > 0) {
            // Scroll up
            TileEngine.Zoom(1.1);
          } else if (ev.wheel.y < 0) {
            // Scroll down
            TileEngine.Zoom(0.9);
          }
        default:
          break;
      }
    }

    OldMouseX = MouseX;
    OldMouseY = MouseY;
    MouseMask = SDL_GetMouseState(&MouseX, &MouseY);

    if (MouseMask & (SDL_BUTTON_LEFT)) {
      TileEngine.XOffset += OldMouseX - MouseX;
      TileEngine.YOffset += OldMouseY - MouseY;
    }

    if (KeyDown(SDLK_LEFT)) {
      TileEngine.XOffset -= 20;
    }
    if (KeyDown(SDLK_RIGHT)) {
      TileEngine.XOffset += 20;
    }
    if (KeyDown(SDLK_UP)) {
      TileEngine.YOffset -= 20;
    }
    if (KeyDown(SDLK_DOWN)) {
      TileEngine.YOffset += 20;
    }

    Timer.update();

    TileEngine.UpdateLevel();

    DirectGraphics.ClearBackBuffer();

    DirectGraphics.SetColorKeyMode();

    TileEngine.CalcRenderRange();
    TileEngine.DrawBackground();

    TileEngine.DrawBackLevel();
    TileEngine.DrawFrontLevel();

    ObjectList.DrawAllObjects(TileEngine.XOffset, TileEngine.YOffset, TileEngine.Scale);

    DirectGraphics.SetColorKeyMode();

    TileEngine.DrawWater();
    TileEngine.DrawBackLevelOverlay();
    TileEngine.DrawOverlayLevel();
    //TileEngine.DrawShadow();

    DirectGraphics.DisplayBuffer();

    Timer.wait();
  }
}
