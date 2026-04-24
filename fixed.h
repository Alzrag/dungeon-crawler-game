#pragma once

#include "GameObject.h"
#include "staticobject.h"
#include <tiny_obj_loader.h>
#include <unordered_map>
#include "Engine.h"
#include "vertex_data.hpp"
#include <vulkan/vulkan.h>

class fixed : public static_object{
  public:
    void init(const std::string& modelPath, const std::string& texturePath, Engine& gameEngine);
    void render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet); 
    ~fixed() override;
  private:
    VkDevice engineDevice= VK_NULL_HANDLE;
    int localVerts;
    int localInds;
    VkBuffer localVertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory localVertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer localIndexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory localIndexBufferMemory  = VK_NULL_HANDLE;
    VkImage localTextureImage = VK_NULL_HANDLE;
    VkDeviceMemory localTextureMemory = VK_NULL_HANDLE;
    VkImageView localTextureImageView = VK_NULL_HANDLE;
    VkSampler localTextureSampler = VK_NULL_HANDLE;
};

