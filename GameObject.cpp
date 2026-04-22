#include "GameObject.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

Gameobject::Gameobject(){
  Position=glm::vec3(0,0,0);
  Scale=glm::vec3(1,1,1);
  yaw=0.0f;
  pitch=0.0f;
  roll=0.0f;

  aabbMin=glm::vec3(0,0,0);
  aabbMax=glm::vec3(0,0,0);

  indexCount=0;

  hasCollider=false;
  hasModel=false;
  hasTexture=false;
  textureImage=VK_NULL_HANDLE;
  textureImageView=VK_NULL_HANDLE;
  textureSampler=VK_NULL_HANDLE;
  vertexBuffer=VK_NULL_HANDLE;
  vertexBufferMemory=VK_NULL_HANDLE;
  indexBuffer=VK_NULL_HANDLE;
  indexBufferMemory=VK_NULL_HANDLE;
}

glm::mat4 Gameobject::getModelMatrix() const{
  glm::mat4 model = glm::mat4(1.0f);

  model=glm::translate(model, Position);

  model=glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
  model=glm::rotate(model, glm::radians(pitch), glm::vec3(0.0f, 1.0f, 0.0f));
  model=glm::rotate(model, glm::radians(roll), glm::vec3(1.0f, 0.0f, 0.0f));

  model=glm::scale(model,Scale);

  return model;
};

