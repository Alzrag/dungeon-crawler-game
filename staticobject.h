#pragma once
#include <vector>
#include "GameObject.h"

class static_object : public Gameobject{
public:
  void update(float deltaTime, std::vector<Gameobject*>& sceneObjects);
  void update(float deltaTime) override;
  void onCollision(Gameobject* other) override;
  virtual ~static_object();

protected:
  bool checkAABB(Gameobject* other);
};
