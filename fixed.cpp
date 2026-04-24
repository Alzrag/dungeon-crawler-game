#include "fixed.h"
#include "Engine.h"
#include "fstream"
#include "vertex_data.hpp"
#include "vulkan/vulkan_core.h"

void fixed::init(const std::string& modelPath, const std::string& texturePath, Engine& gameEngine){
  engineDevice = gameEngine.device; 
  gameEngine.createVertexBuffer(vertices, localVertexBuffer, localVertexBufferMemory);
  gameEngine.createIndexBuffer(indices, localIndexBuffer, localIndexBufferMemory); 
  hasTexture  = false;

  if (std::ifstream(modelPath)){
    hasModel    = true;
    hasCollider = true;
    gameEngine.loadModel(modelPath, vertices, indices);
    if (std::ifstream(texturePath)) {
      gameEngine.loadTextureFromPath(texturePath, localTextureImage,
                                      localTextureMemory, localTextureImageView,
                                      localTextureSampler);
      hasTexture = true;
    } else {
      hasTexture = false;
    }  
  } else {
    hasModel = false;
    hasCollider = false;
  }
  indexCount = static_cast<uint32_t>(indices.size());
}

void fixed::render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet){ 
  if (!hasModel) return;

  VkBuffer vertexBuffers[] = { localVertexBuffer };
  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

  vkCmdBindIndexBuffer(commandBuffer, localIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

  glm::mat4 model = getModelMatrix();
  vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);

  vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

fixed::~fixed(){
  if(engineDevice==VK_NULL_HANDLE) return;
  if (localIndexBuffer!=VK_NULL_HANDLE){
    vkDestroyBuffer(engineDevice, localVertexBuffer, nullptr);
    vkFreeMemory(engineDevice, localIndexBufferMemory, nullptr);
  }
  if (localVertexBuffer!=VK_NULL_HANDLE){
    vkDestroyBuffer(engineDevice, localVertexBuffer, nullptr);
    vkFreeMemory(engineDevice, localVertexBufferMemory, nullptr);
  }
}
