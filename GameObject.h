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

  bool isWall = false;

  /**
   * @brief per frame update tick
   *
   * @param deltaTime eleapstd time in secound since last frame
   */
  virtual void update(float deltaTime)=0;

  /**
   * @brief issues render commands
   */
  virtual void render()=0;

  /**
   * @brief what to do on a collsiion 
   *
   * @param other the other object to preforn actions to
   */
  virtual void onCollision(Gameobject* other) = 0;

  /**
   * @brief a initalization and set up function for more customization that the constructor
   */
  virtual void init()=0;

  /**
   * @brief virtual distrcutor for later 
   */
  virtual ~Gameobject();

  /**
   * @brief default connstructor initalizes transform and state members to safe defaults
   */
  Gameobject();

  /**
   * @brief preforms a world space model matrix caluclation based on scale and roation value and th emth for a 4x4 matirx that vulkan uses in rendering
   */
  glm::mat4 getModelMatrix() const;
};
