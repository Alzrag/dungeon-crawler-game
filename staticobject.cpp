#include "staticobject.h"
#include "GameObject.h"

void static_object::update(float deltaTime, std::vector<Gameobject*>& sceneObjects){
  if (!hasCollider){
    return;
  }

  for (Gameobject* object: sceneObjects){
    if (object==this){
      continue;
    } else if (!object->hasCollider){
      continue;
    } else if(checkAABB(object)){
      onCollision(object);
    }
  }
}

bool static_object::checkAABB(Gameobject* other){
  if (this->aabbMax.x+Position.x < other->aabbMin.x+other->Position.x) return false;
  if (this->aabbMin.x+Position.x > other->aabbMax.x+other->Position.x) return false;
  if (this->aabbMax.y+Position.y < other->aabbMin.y+other->Position.y) return false;
  if (this->aabbMin.y+Position.y > other->aabbMax.y+other->Position.y) return false;
  if (this->aabbMax.z+Position.z < other->aabbMin.z+other->Position.z) return false;
  if (this->aabbMin.z+Position.z > other->aabbMax.z+other->Position.z) return false;
  return true;
}
