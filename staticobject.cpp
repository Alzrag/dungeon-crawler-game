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

void static_object::onCollision(Gameobject* other){
  glm::vec3 selfCenter  = Position + (aabbMin + aabbMax) * 0.5f;
  glm::vec3 otherCenter = other->Position + (other->aabbMin + other->aabbMax) * 0.5f;

  float overlapX = (aabbMax.x + Position.x) - (other->aabbMin.x + other->Position.x);
  float overlapNX = (other->aabbMax.x + other->Position.x) - (aabbMin.x + Position.x);
  float overlapY = (aabbMax.y + Position.y) - (other->aabbMin.y + other->Position.y);
  float overlapNY = (other->aabbMax.y + other->Position.y) - (aabbMin.y + Position.y);
  float overlapZ = (aabbMax.z + Position.z) - (other->aabbMin.z + other->Position.z);
  float overlapNZ = (other->aabbMax.z + other->Position.z) - (aabbMin.z + Position.z);

  float minOverlap = overlapX;
  glm::vec3 correction = glm::vec3(overlapX,0,0);
  if(overlapNX<minOverlap){
    minOverlap=overlapNX;
    correction=glm::vec3(-overlapNX,0,0);
  }
  if(overlapY<minOverlap){
    minOverlap=overlapY;
    correction=glm::vec3(0,overlapY,0);
  }
  if(overlapNY<minOverlap){
    minOverlap=overlapNY;
    correction=glm::vec3(0,-overlapNY,0);
  }
  if(overlapZ<minOverlap){
    minOverlap=overlapZ;
    correction=glm::vec3(0,0,overlapZ);
  }
  if(overlapNZ<minOverlap){
    correction=glm::vec3(0,0,-overlapNZ);
  }

  Position-=correction;
}
