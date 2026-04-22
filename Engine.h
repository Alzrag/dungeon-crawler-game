#pragma once

//Core
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//std
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cstring>
#include <map>
#include <optional>
#include <set>
#include <fstream>
#include <chrono>
#include <unordered_map>

//GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//External
#include <stb_image.h>
#include <tiny_obj_loader.h>

//Project Files
#include "vertex_data.hpp"
#include "helpers.h"


inline const uint32_t WIDTH = 800;
inline const uint32_t HEIGHT = 600;//really dont change window size its hard

inline const std::string APPLICATIONNAME= "triangle application";
inline const std::string ENGINENAME="DM Engine";

inline const std::string MODEL_PATH="models/viking_room.obj";
inline const std::string TEXTURE_PATH="textures/viking_room.png";

inline const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};//holds our additional validation layers witch are largly all in a build int one by the used name

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

class Engine {
  public:
    void run();
  private:
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkDevice device;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
    VkQueue presentQueue;
    const std::vector<const char*> deviceExtensions = { //used for the set of nessisary extension for your gpu to be valid
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;//ferb we did it
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    //dancing aroudn emmeory acsess issues
    const int MAX_FRAMES_IN_FLIGHT = 2;
    int currentFrame = 0;
    std::vector<VkSemaphore> imageAvailableSemaphores;//a semaphore as i unserstanding it a memory and event sensitive sleep statement
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;//a signaller for when something is done
    bool framebufferResized = false;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    struct QueueFamilyIndicies {
      std::optional<uint32_t> graphicsFamily;
      std::optional<uint32_t> presentFamily;
      bool isComplete(){
        return graphicsFamily.has_value() && presentFamily.has_value();
      }
    }; 

    struct SwarpChainSupportDetails {//figure out max and min texture and data sizes presentation modes and color spaces
      VkSurfaceCapabilitiesKHR capabilities;
      std::vector<VkSurfaceFormatKHR> formats;
      std::vector<VkPresentModeKHR> presentModes;
    };

    struct UniformBufferObject {
      alignas(16) glm::mat4 model;
      alignas(16) glm::mat4 view;
      alignas(16) glm::mat4 proj;
    };

    // GLFW
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    // Core init and loop
    void initVulkan();
    void mainLoop();
    void drawFrame();
    void cleanup();

    // Instance and device setup
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    int rateDeviceSuitability(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device);

    // Swapchain
    void createSwapChain();
    void recreateSwapChain();
    void cleanupSwapChain();
    void createImageViews();
    SwarpChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    // Pipeline and render pass
    void createRenderPass();
    void createGraphicsPipeline();
    void createDescriptorSetLayout();
    VkShaderModule createShaderModule(const std::vector<char>& code);

    // Framebuffers and commands
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffer();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    // Buffers
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void updateUniformBuffer(uint32_t currentImage);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // Descriptors
    void createDescriptorPool();
    void createDescriptorSets();

    // Images and textures
    void createTextureImage();
    void createTextureImageView();
    void createTextureSampler();
    void createDepthResources();
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instace, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();

    // Single time commands
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    // Sync
    void createSyncObjects();

    // Model
    void loadModel();

    // Debug
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
};
