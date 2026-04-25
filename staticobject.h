#pragma once
#include <vector>
#include "GameObject.h"

class static_object : public Gameobject{
public:
  void update(float deltaTime, std::vector<Gameobject*>& sceneObjects);
  void onCollision(Gameobject* other);
  virtual ~static_object();

protected:
  bool checkAABB(Gameobject* other);
};
