// FIXME: water displaying causes artifacts when zooming out

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

void LoadMap(int index);

int main() {
  DirectGraphics.Init(640, 480, 32, false);
  TileEngine.LoadSprites();

  KeyBuffer = SDL_GetKeyboardState(&NumberOfKeys);

  TileEngine.LoadLevel(g_storage_ext + "/data/levels/jungle.map");

  Timer.SetMaxFPS(60);
  Timer.update();

  int OldMouseX;
  int OldMouseY;
  int MouseX;
  int MouseY;
  unsigned int MouseMask;

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
        case SDL_KEYDOWN:
        switch (ev.key.keysym.sym) {
            case SDLK_1: LoadMap(1); break;
            case SDLK_2: LoadMap(2); break;
            case SDLK_3: LoadMap(3); break;
            case SDLK_4: LoadMap(4); break;
            case SDLK_5: LoadMap(5); break;
            case SDLK_6: LoadMap(6); break;
            case SDLK_7: LoadMap(7); break;
            case SDLK_8: LoadMap(8); break;
            case SDLK_9: LoadMap(9); break;
            case SDLK_0: LoadMap(10); break;
            case SDLK_s:
              TileEngine.SaveLevel("test.map");
            break;
          }
        default:
          break;
      }
    }


    OldMouseX = MouseX;
    OldMouseY = MouseY;
    MouseMask = SDL_GetMouseState(&MouseX, &MouseY);

    //if (MouseMask & (SDL_BUTTON_LEFT)) {
    if (MouseMask & (SDL_BUTTON(1))) {
      TileEngine.XOffset += OldMouseX - MouseX;
      TileEngine.YOffset += OldMouseY - MouseY;
    }

    //if (MouseMask & (SDL_BUTTON_RIGHT)) {
    if (MouseMask & (SDL_BUTTON(3))) {
        const int x_tile = MouseX / TileEngine.TileSizeX + TileEngine.XOffset / TileEngine.TileSizeX;
        const int y_tile = MouseY / TileEngine.TileSizeY + TileEngine.YOffset / TileEngine.TileSizeY;
        TileEngine.TileAt(x_tile, y_tile).FrontArt = 0;
        TileEngine.TileAt(x_tile, y_tile).BackArt = 0;
        TileEngine.TileAt(x_tile, y_tile).Block = 0;
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

void LoadMap(int index) {
  const std::map<int, std::string> level_map = {
    std::pair(1, g_storage_ext + "/data/levels/jungle.map"),
    std::pair(2, g_storage_ext + "/data/levels/temple.map"),
    std::pair(3, g_storage_ext + "/data/levels/elevator.map"),
    std::pair(4, g_storage_ext + "/data/levels/mpde.map"),
    std::pair(5, g_storage_ext + "/data/levels/spinnenfabrik.map"),
    std::pair(6, g_storage_ext + "/data/levels/tower.map"),
    std::pair(7, g_storage_ext + "/data/levels/eis.map"),
    std::pair(8, g_storage_ext + "/data/levels/cave.map"),
    std::pair(9, g_storage_ext + "/data/levels/endboss.map"),
    std::pair(10, g_storage_ext + "/data/levels/tutorial.map"),
  };

  if (level_map.find(index) != level_map.end()) {
    TileEngine.LoadLevel(level_map.at(index));
  }
}
