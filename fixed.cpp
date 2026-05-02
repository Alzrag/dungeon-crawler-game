#include "fixed.h"
#include "Engine.h"
#include "fstream"
#include "vertex_data.hpp"
#include "vulkan/vulkan_core.h"

void fixed::init(const std::string& modelPath, const std::string& texturePath, Engine& gameEngine){
  engineDevice = gameEngine.device;
  enginePool    = gameEngine.descriptorPool;
  hasTexture  = false;
  this->texturePath=texturePath;
  engine=&gameEngine;

  if (std::ifstream(modelPath)){
    hasModel    = true;
    hasCollider = true;
    gameEngine.loadModel(modelPath, vertices, indices);
    gameEngine.createVertexBuffer(vertices, localVertexBuffer, localVertexBufferMemory);
    gameEngine.createIndexBuffer(indices, localIndexBuffer, localIndexBufferMemory);
    if (hasModel) {
      aabbMin = vertices[0].pos;
      aabbMax = vertices[0].pos;
      for (const auto& v : vertices) {
        aabbMin = glm::min(aabbMin, v.pos);
        aabbMax = glm::max(aabbMax, v.pos);
      }
    }
    if (std::ifstream(texturePath)) {
      gameEngine.loadTextureFromPath(texturePath, localTextureImage, localTextureMemory, localTextureImageView, localTextureSampler);
      hasTexture = true;
    } else {
      hasTexture = false;
    }  
  } else {
    hasModel = false;
    hasCollider = false;
  }
  indexCount = static_cast<uint32_t>(indices.size());

  std::vector<VkDescriptorSetLayout> layouts(gameEngine.MAX_FRAMES_IN_FLIGHT, gameEngine.descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = gameEngine.descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(gameEngine.MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(gameEngine.MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(gameEngine.device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
      throw std::runtime_error("failed to allocate per-object descreptor sets");

    for (int i = 0; i < gameEngine.MAX_FRAMES_IN_FLIGHT; i++) {
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = gameEngine.uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(Engine::UniformBufferObject);

      VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = hasTexture ? localTextureImageView : gameEngine.textureImageView;
      imageInfo.sampler = gameEngine.textureSampler;

      std::array<VkWriteDescriptorSet, 2> writes{};
      writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writes[0].dstSet = descriptorSets[i];
      writes[0].dstBinding = 0;
      writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      writes[0].descriptorCount = 1;
      writes[0].pBufferInfo = &bufferInfo;

      writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writes[1].dstSet = descriptorSets[i];
      writes[1].dstBinding = 1;
      writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      writes[1].descriptorCount = 1;
      writes[1].pImageInfo = &imageInfo;
 
      vkUpdateDescriptorSets(gameEngine.device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }
}

void fixed::render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int frameIndex){ 
  if (!hasModel) return;

  VkBuffer vertexBuffers[] = { localVertexBuffer };
  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

  vkCmdBindIndexBuffer(commandBuffer, localIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout, 0, 1, &descriptorSets[frameIndex], 0, nullptr);

  glm::mat4 model = getModelMatrix();
  vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);

  vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void fixed::update(float) {}
void fixed::render() {}
void fixed::init() {}

fixed::~fixed(){
  if(engineDevice==VK_NULL_HANDLE) return;
  if (!descriptorSets.empty() && enginePool != VK_NULL_HANDLE){
    vkFreeDescriptorSets(engineDevice, enginePool, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data());
  }
  if (localIndexBuffer!=VK_NULL_HANDLE){
    vkDestroyBuffer(engineDevice, localIndexBuffer, nullptr);
    vkFreeMemory(engineDevice, localIndexBufferMemory, nullptr);
  }
  if (localVertexBuffer!=VK_NULL_HANDLE){
    vkDestroyBuffer(engineDevice, localVertexBuffer, nullptr);
    vkFreeMemory(engineDevice, localVertexBufferMemory, nullptr);
  }
  if (localTextureImage != VK_NULL_HANDLE){
    vkDestroyImage(engineDevice, localTextureImage, nullptr);
    vkFreeMemory(engineDevice, localTextureMemory, nullptr);
    vkDestroyImageView(engineDevice, localTextureImageView, nullptr);
  }
}

fixed::fixed(const fixed& other) {
  *this = other;
}

fixed& fixed::operator=(const fixed& other){
  if (this == &other) return *this;
  if(engineDevice!=VK_NULL_HANDLE){
    if (!descriptorSets.empty() && enginePool != VK_NULL_HANDLE){
      vkFreeDescriptorSets(engineDevice, enginePool, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data());
    }
    if (localIndexBuffer!=VK_NULL_HANDLE){
      vkDestroyBuffer(engineDevice, localIndexBuffer, nullptr);
      vkFreeMemory(engineDevice, localIndexBufferMemory, nullptr);
    }
    if (localVertexBuffer!=VK_NULL_HANDLE){
      vkDestroyBuffer(engineDevice, localVertexBuffer, nullptr);
      vkFreeMemory(engineDevice, localVertexBufferMemory, nullptr);
    }
    if (localTextureImage != VK_NULL_HANDLE){
      vkDestroyImage(engineDevice, localTextureImage, nullptr);
      vkFreeMemory(engineDevice, localTextureMemory, nullptr);
      vkDestroyImageView(engineDevice, localTextureImageView, nullptr);
    }
  }
  Position = other.Position;
  Scale = other.Scale;
  yaw = other.yaw;
  pitch = other.pitch;
  roll = other.roll;
  aabbMin = other.aabbMin;
  aabbMax = other.aabbMax;
  hasModel = other.hasModel;
  hasTexture = other.hasTexture;
  hasCollider = other.hasCollider;
  vertices = other.vertices;
  indices = other.indices;
  indexCount = other.indexCount;
  localVerts = other.localVerts;
  localInds = other.localInds;
  engineDevice = other.engineDevice;
  enginePool = other.enginePool;
  engine = other.engine;
  texturePath = other.texturePath;

  if (engineDevice == VK_NULL_HANDLE || !other.hasModel) return *this;

  engine->createVertexBuffer(vertices, localVertexBuffer, localVertexBufferMemory);
  engine->createIndexBuffer(indices, localIndexBuffer, localIndexBufferMemory);

  if (other.hasTexture && !other.texturePath.empty()) {
    engine->loadTextureFromPath(texturePath, localTextureImage,
    localTextureMemory, localTextureImageView, localTextureSampler);
  } else {
    localTextureImage    = VK_NULL_HANDLE;
    localTextureMemory   = VK_NULL_HANDLE;
    localTextureImageView= VK_NULL_HANDLE;
    localTextureSampler  = VK_NULL_HANDLE;
  }
  std::vector<VkDescriptorSetLayout> layouts(engine->MAX_FRAMES_IN_FLIGHT, engine->descriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = enginePool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(engine->MAX_FRAMES_IN_FLIGHT);
  allocInfo.pSetLayouts = layouts.data();
  descriptorSets.resize(engine->MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(engineDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate descriptor sets in copy");
  for (int i = 0; i < engine->MAX_FRAMES_IN_FLIGHT; i++) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = engine->uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(Engine::UniformBufferObject);
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = hasTexture ? localTextureImageView : engine->textureImageView;
    imageInfo.sampler = hasTexture ? localTextureSampler : engine->textureSampler;
    std::array<VkWriteDescriptorSet, 2> writes{};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = descriptorSets[i];
    writes[0].dstBinding = 0;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].descriptorCount = 1;
    writes[0].pBufferInfo = &bufferInfo;
    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = descriptorSets[i];
    writes[1].dstBinding = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].descriptorCount = 1;
    writes[1].pImageInfo = &imageInfo;
    vkUpdateDescriptorSets(engineDevice, 2, writes.data(), 0, nullptr);
  }
  return *this;
}
