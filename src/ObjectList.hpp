#ifndef OBJECT_LIST_HPP_
#define OBJECT_LIST_HPP_

#include <array>
#include "DX8Sprite.hpp"
#include "Gegner.hpp"
#include "Tileengine.hpp"

class Object {
public:
  uint32_t ObjectID;
  int32_t XPos;
  int32_t YPos;
  int32_t Value1;
  int32_t Value2;
};

class ObjectListClass {
public:
  void PushObject(uint32_t ID, int32_t x, int32_t y, int32_t Value1, int32_t Value2);

  void LoadObjectGraphic(int index);
  void LoadAllGraphics();

  void DrawObject(int index, TileEngineClass& TE);
  void DrawAllObjects(TileEngineClass& TE);

  unsigned int ObjectIndex;

  std::array<Object, MAX_GEGNER> Objects;
  std::array<DirectGraphicsSprite*, MAX_GEGNERGFX> ObjectGraphics;

  ObjectListClass();
  ~ObjectListClass();
};

#endif
