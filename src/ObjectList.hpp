#ifndef OBJECT_LIST_HPP_
#define OBJECT_LIST_HPP_

#include <array>
#include "DX8Sprite.hpp"
#include "Gegner.hpp"

class Object {
public:
  uint32_t ObjectID;
  int32_t XPos;
  int32_t YPos;
  uint8_t ChangeLight;
  uint8_t Skill;
  int32_t Value1;
  int32_t Value2;
};

class ObjectListClass {
public:
  void PushObject(Object object);

  void LoadObjectGraphic(int index);
  void LoadAllGraphics();

  void DrawObject(int index, float xoff, float yoff, float scale);
  void DrawAllObjects(float xoff, float yoff, float scale);

  void ClearObjects();

  std::array<Object, MAX_GEGNER> Objects;
  std::array<DirectGraphicsSprite*, MAX_GEGNERGFX> ObjectGraphics;

  ObjectListClass();
  ~ObjectListClass();

  unsigned int ObjectCount;
private:
};

#endif
