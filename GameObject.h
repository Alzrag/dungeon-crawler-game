#pragma once

#include "vulkan/vulkan_core.h"
#include <array>
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Gameobject {
public:
  glm::vec3 Position;
  glm::vec3 Scale;
  float yaw,pitch,roll;
  glm::vec3 aabbMin;
  glm::vec3 aabbMax;
  VkImage textureImage;
  VkImageView textureImageView;
  VkSampler textureSampler;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;
  uint32_t indexCount;

  bool hasModel;
  bool hasTexture;
  bool hasCollider;

  virtual void update(float deltaTime)=0;

  virtual void render()=0;

  virtual void onCollision(Gameobject* other) = 0;

  virtual void init()=0;

  virtual ~Gameobject();

  Gameobject();

  glm::mat4 getModelMatrix() const;
};
