#pragma once
#include <vector>
#include "GameObject.h"

class static_object : public Gameobject{
public:
  void update(float deltaTime, std::vector<Gameobject*>& sceneObjects);
  virtual void onCollision(Gameobject* other)=0;
  virtual ~static_object();

protected:
  bool checkAABB(Gameobject* other);
};
