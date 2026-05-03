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
  glm::vec3 selfMin  = aabbMin * Scale + Position;
  glm::vec3 selfMax  = aabbMax * Scale + Position;
  glm::vec3 otherMin = other->aabbMin * other->Scale + other->Position;
  glm::vec3 otherMax = other->aabbMax * other->Scale + other->Position;
  if (selfMax.x < otherMin.x) return false;
  if (selfMin.x > otherMax.x) return false;
  if (selfMax.y < otherMin.y) return false;
  if (selfMin.y > otherMax.y) return false;
  if (selfMax.z < otherMin.z) return false;
  if (selfMin.z > otherMax.z) return false;
  return true;
}

void static_object::onCollision(Gameobject* other){
  if (other->isWall) return;
  if (!this->isWall) return;

  glm::vec3 selfMin  = aabbMin * Scale + Position;
  glm::vec3 selfMax  = aabbMax * Scale + Position;
  glm::vec3 otherMin = other->aabbMin * other->Scale + other->Position;
  glm::vec3 otherMax = other->aabbMax * other->Scale + other->Position;

  glm::vec3 selfCenter  = (selfMin + selfMax) * 0.5f;
  glm::vec3 otherCenter = (otherMin + otherMax) * 0.5f;
  glm::vec3 dir = otherCenter - selfCenter;

  float overlapX  = selfMax.x  - otherMin.x;
  float overlapNX = otherMax.x - selfMin.x;
  float overlapY  = selfMax.y  - otherMin.y;
  float overlapNY = otherMax.y - selfMin.y;
  float overlapZ  = selfMax.z  - otherMin.z;
  float overlapNZ = otherMax.z - selfMin.z;

  float minOverlap = overlapX;
  glm::vec3 correction = glm::vec3(dir.x > 0 ? overlapX : -overlapX, 0, 0);
  if(overlapNX < minOverlap){
    minOverlap = overlapNX;
    correction = glm::vec3(dir.x > 0 ? overlapNX : -overlapNX, 0, 0);
  }
  if(overlapY  < minOverlap){
    minOverlap = overlapY; 
    correction = glm::vec3(0, dir.y > 0 ? overlapY  : -overlapY,  0);
  }
  if(overlapNY < minOverlap){
    minOverlap = overlapNY;
    correction = glm::vec3(0, dir.y > 0 ? overlapNY : -overlapNY, 0);
  }
  if(overlapZ  < minOverlap){
    minOverlap = overlapZ;
    correction = glm::vec3(0, 0, dir.z > 0 ? overlapZ  : -overlapZ );
  }
  if(overlapNZ < minOverlap){
    correction = glm::vec3(0, 0, dir.z > 0 ? overlapNZ : -overlapNZ);
  }

  other->Position += correction;
}

static_object::~static_object() {}
