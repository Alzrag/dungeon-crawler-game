#include <cstddef>
#include <cstdint>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vector_relational.hpp>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <sys/types.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cstring>
#include <map>
#include <optional>
#include <set>
#include <fstream>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include "vertex_data.hpp"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;//really dont change window size its hard

const std::string APPLICATIONNAME= "triangle application";
const std::string ENGINENAME="No Engine";

const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};//holds our additional validation layers witch are largly all in a build int one by the used name

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

template<typename T>
T clamp(T value, T min, T max){
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

class HelloTriangleApplication {
public:
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    } 

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
    VkPipelineLayout piplineLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height){
      auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
      app->framebufferResized=true;
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex){
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags=0;
      beginInfo.pInheritanceInfo=nullptr;
      if(vkBeginCommandBuffer(commandBuffer, &beginInfo)!=VK_SUCCESS){
        throw std::runtime_error("failed to begin recoridng command buffer");
      }

      VkClearValue clearColor ={{{0.0f,0.0f,0.0f,1.0f}}};
      VkRenderPassBeginInfo renderPassInfo{};
      renderPassInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassInfo.renderPass=renderPass;
      renderPassInfo.framebuffer=swapChainFramebuffers[imageIndex];
      renderPassInfo.renderArea.offset={0,0};
      renderPassInfo.renderArea.extent=swapChainExtent;
      renderPassInfo.clearValueCount=1;
      renderPassInfo.pClearValues=&clearColor;
      vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
      
      VkViewport viewport{};//stretches
      viewport.x = 0.0f;
      viewport.y=0.0f;
      viewport.width=static_cast<float>(swapChainExtent.width);
      viewport.height=static_cast<float>(swapChainExtent.height);
      viewport.minDepth=0.0f;
      viewport.maxDepth=1.0f;
      vkCmdSetViewport(commandBuffer,0,1,&viewport);

      VkRect2D scissor{};//crops
      scissor.offset={0,0};
      scissor.extent=swapChainExtent;
      vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
      VkBuffer vertexBuffers[] = {vertexBuffer};
      VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(commandBuffer,0,1,vertexBuffers, offsets);
      vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,0,1,&descriptorSets[currentFrame],0,nullptr);
      vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indicies.size()),1,0,0,0);

      vkCmdEndRenderPass(commandBuffer);
      if(vkEndCommandBuffer(commandBuffer)!=VK_SUCCESS){
        throw std::runtime_error("failed tor ecord command buffer");
      }
    }

    struct QueueFamilyIndicies {
      std::optional<uint32_t> graphicsFamily;
      std::optional<uint32_t> presentFamily;
      bool isComplete(){
        return graphicsFamily.has_value() && presentFamily.has_value();
      }
    };
    //queue families dertminte the order of commands sent
    QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device){
      QueueFamilyIndicies indices;
      uint32_t queueFamilyCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);//get a list of queue families
      std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
      vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());//populate the list of quefamilties with data;
      int i=0;
      for (const auto& queueFamily :queueFamilies){
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
          VkBool32 presentSupport = false;
          vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
          if (presentSupport){
            indices.presentFamily = i;
          }
          indices.graphicsFamily = i;
        }
        if(indices.isComplete()){
          break;
        }
        i++;
      }
      return indices  ;
    }

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

    SwarpChainSupportDetails querySwapChainSupport(VkPhysicalDevice device){
      SwarpChainSupportDetails details;

      uint32_t formatCount;
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
      if (formatCount !=0){
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
      }
      
      uint32_t presentModeCount;
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
      if (presentModeCount!=0){
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
      }
      return details;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availbileFormats){
      for (const auto& availbileFormat : availbileFormats) {
        if (availbileFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availbileFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){//industry standard and most common
          return availbileFormat;
        }
      }
      return availbileFormats[0]; //fallback
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availiblePresentModes){
      for (const auto& availiblePresentMode : availiblePresentModes){
        if (availiblePresentMode == VK_PRESENT_MODE_MAILBOX_KHR){//vulkan recomends and i agree as it prevents frame pacing shows the latest frame only and is effectivly uncapped
          return availiblePresentMode;
        }
      }
      return VK_PRESENT_MODE_FIFO_KHR;//garanetied t be availbile so its a good fall back
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities){
      if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){//if not wayland
        return capabilities.currentExtent;
      } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        
        actualExtent.width= clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height=clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
      }
    }

    void pickPhysicalDevice(){
      physicalDevice = VK_NULL_HANDLE;//hold current phsycial device
      uint32_t deviceCount =0;//numbers of availbile vulkan gpu's
      vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);//check how many there are
      if (deviceCount == 0){
        throw std::runtime_error("failed to find GPU's with Vulkan support! (please check you device, drivers, and vulkan instialation[should have been automatic])");//if its 0 there is a issue
      }
      std::vector<VkPhysicalDevice> devices(deviceCount);//now that we know there s no error we can add them to a list
      vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
      std::multimap<int, VkPhysicalDevice> candidates;//sort them with a multimap ordred by their score
      for  (const auto& device : devices){
        int score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
      }
      if (candidates.rbegin()->first>0){//if the best cant support vulkan non can
        physicalDevice = candidates.rbegin()->second;
      } else {
        throw std::runtime_error("failed to find GPU's with Vulkan support! (please check you device, drivers, and vulkan instialation[should have been automatic])");
      }
    }

    int rateDeviceSuitability(VkPhysicalDevice device){
      //start be getting the properties and features of the device
      VkPhysicalDeviceProperties deviceProperties;
      vkGetPhysicalDeviceProperties(device, &deviceProperties);
      VkPhysicalDeviceFeatures deviceFeatures;
      vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
      VkPhysicalDeviceMemoryProperties memoryProperties;
      vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

      int score = 0;
      // if it cant do gemotiry shading it cant do what we need
      if (!deviceFeatures.geometryShader) {
        return 0;
      }
      //check type
      switch (deviceProperties.deviceType){
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
          score += 475;
          break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
          score += 350;
          break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
          score += 250;
          break;
        default:
          score +=0;
          break;
      }
      //raw stat based enchacement
      score += deviceProperties.limits.maxImageDimension2D / 128;
      score += deviceProperties.limits.maxImageDimension3D / 32;
      score += deviceProperties.limits.maxImageDimensionCube / 128;
      score += deviceProperties.limits.maxImageArrayLayers;
      score += deviceProperties.limits.maxUniformBufferRange / 100000;
      score += deviceProperties.limits.maxStorageBufferRange / 100000;
      score += deviceProperties.limits.maxPushConstantsSize;
      score += deviceProperties.limits.maxComputeSharedMemorySize / 750;
      score += deviceProperties.limits.maxComputeWorkGroupInvocations / 2;
      score += deviceProperties.limits.maxDrawIndirectCount > 1? 200 : 0;
      score += deviceProperties.limits.maxDrawIndexedIndexValue > 0xFFFF? 100 : 0;
      score += deviceProperties.limits.maxBoundDescriptorSets * 10;
      score += (deviceProperties.limits.maxDescriptorSetSamplers > 4096 ? 4096u : deviceProperties.limits.maxDescriptorSetSamplers) / 10;//why the fuck do these get so big on modern cards
      score += (deviceProperties.limits.maxDescriptorSetUniformBuffers > 256 ? 256u : deviceProperties.limits.maxDescriptorSetUniformBuffers) * 5;
      score += (deviceProperties.limits.maxDescriptorSetStorageBuffers > 256 ? 256u : deviceProperties.limits.maxDescriptorSetStorageBuffers) * 5;      
      score += deviceProperties.limits.maxVertexInputAttributes * 5;
      score += deviceProperties.limits.maxVertexInputBindings * 5;
      score += deviceProperties.limits.maxFramebufferWidth / 512;
      score += deviceProperties.limits.maxFramebufferHeight / 512;
      score += deviceProperties.limits.maxColorAttachments * 10;
      score += deviceProperties.limits.maxSampleMaskWords * 10;
      score += (int)deviceProperties.limits.maxSamplerAnisotropy * 10;
      score += (int)deviceProperties.limits.maxSamplerLodBias * 5;
      //raw feature based additives
      if (deviceFeatures.tessellationShader) score += 100;
      if (deviceFeatures.multiDrawIndirect) score += 100;
      if (deviceFeatures.samplerAnisotropy) score += 50;
      if (deviceFeatures.textureCompressionBC) score += 50;
      if (deviceFeatures.textureCompressionETC2) score += 30;
      if (deviceFeatures.shaderFloat64) score += 50;
      if (deviceFeatures.shaderInt64) score += 30;
      if (deviceFeatures.shaderInt16) score += 20;
      if (deviceFeatures.multiViewport) score += 50;
      if (deviceFeatures.depthClamp) score += 20;
      if (deviceFeatures.fillModeNonSolid) score += 20;
      if (deviceFeatures.wideLines) score += 10;
      if (deviceFeatures.largePoints) score += 10;
      //vram addtion calcuator biggest component
      VkDeviceSize totalVRAM = 0;
      for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++) {
          if (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
              totalVRAM += memoryProperties.memoryHeaps[i].size;
          }
      }
      if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){//vulkan detects 0 from igpu but they have 128mb for intel 256mb for amd usually
        switch(deviceProperties.vendorID){
          case 0x8086://intel
            totalVRAM+=128*1024*1024;
            break;
          case 0x1002://amd 
            totalVRAM+=256*1024*1024;
            break;
          default:
            totalVRAM+=128*1024*1024;
            break;
        }
      }
      score += (int)(totalVRAM / (1024 * 1024 * 1024)) * 200;

      return score;
    } 

    bool isDeviceSuitable(VkPhysicalDevice device){
      QueueFamilyIndicies indices = findQueueFamilies(device);
      bool extensionsSupported = checkDeviceExtensionSupport(device);
      bool swapChainAdequate = false;
      if (extensionsSupported){
        SwarpChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
      }
      return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device){
      uint32_t extensionCount;
      vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
      
      std::vector<VkExtensionProperties> availibleExtensions(extensionCount);
      vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availibleExtensions.data());

      std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());//why not size?
      for (const auto& extension : availibleExtensions) {
        requiredExtensions.erase(extension.extensionName); 
      }
      return requiredExtensions.empty();
    }

    void createInstance() {
      if (enableValidationLayers && !checkValidationLayerSupport()){
        throw std::runtime_error("Validation layers requested, but not availible!");
      }

      //below is data that while optional is recomended by vulkan for driver specific optimizations
      //here are some notes on this section sType specify the tpy eof the struct 
      //pApplicationName and pEngineName are just names 
      //applicationVersion and engineVersion say was api or encoder vision to use leaving these as defult
      //pNext while not use gives sub strcut behavior
      VkApplicationInfo appInfo{};
      appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pApplicationName = APPLICATIONNAME.c_str();
      appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);//m,ajor minor patch
      appInfo.pEngineName = ENGINENAME.c_str();
      appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);//major minor patch
      appInfo.apiVersion = VK_API_VERSION_1_0;

      //this part now conglomerats the info to one space for jhust genreal info
      VkInstanceCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;// a pointer to the info above 
      createInfo.pApplicationInfo = &appInfo;// pointer actual to whats above ^ is jsut a name for this pointer 

      // this section handles any extenions used by vulkan right now there are not any
      uint32_t glfwExtensionCount = 0;
      const char** glfwExtensions;
      glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
      createInfo.enabledExtensionCount = glfwExtensionCount;//ads the the vkinstance info same with below
      createInfo.ppEnabledExtensionNames = glfwExtensions;
      //part of ehwats above but this adds validation layer names when enabled  
      if (enableValidationLayers) {
        createInfo.enabledLayerCount= static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
      } else {//otherwise leave it as is
        createInfo.enabledLayerCount = 0;
      }

      // update looks like it is just a value for the system to know how many layers we have ie validaiton runtime or other layers to run in our vulkan instance
      // dont need this anymore i think createInfo.enabledLayerCount = 0;
      
      //call our getRequiredExtensions to add debuggig support
      auto extensions = getRequiredExtensions();
      createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
      createInfo.ppEnabledExtensionNames = extensions.data();

      //create a vulkan instance yay we have a up and running vulkan back end at this point
      //VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);//popinter to a struct with creation information a pointer to allocator call backs(advanced topic not in vulkan tutorials), and a pointer to the varaible that will stor our instance

      if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {//runs the same cration check but in a local scope to test if it should suceed
        throw std::runtime_error("failed to create instance!");//if it broke
      }
    }

    bool checkValidationLayerSupport(){
      uint32_t layerCount;
      vkEnumerateInstanceLayerProperties(&layerCount, nullptr);//these are effectivly identifcal to extension properties just for a whole layer
      std::vector<VkLayerProperties> availableLayers(layerCount);
      vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
      for (const char* layerName : validationLayers) {//check if valdiation layers are in availible layers list
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
          if (strcmp(layerName, layerProperties.layerName) == 0) {
            layerFound = true;
            break;
          }
        }
        if (!layerFound) {
          return false;
        }
      }

      return true;
    }
    
    //wow weird way to declare a function but looks likes is a VKAPI_CALL of type VkBool32 of type VKAPI_ATTR like how a vector can be of type int
    //i know i could and will do this with neovim but the paremters are in order:
  //the severity of the messages
  //the type of messages
  //a pointer to a struct with more details about the error 
  //a a pointer to additinal user data
  //in effect we are building our own error statement here
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messsageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData){
      std::cerr << "validation Layer: " << pCallbackData->pMessage << std::endl;
      return VK_FALSE;
    }

    //allows us to handle output of debuggin messages with a addtional extenion yippy more exteions and layers most likley down the line
    std::vector<const char*> getRequiredExtensions() {
      uint32_t glfwExtensionCount = 0;
      const char** glfwExtensions;
      glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
      std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
      if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);//VK_EXT_DEBUG_UTILS_EXTENSION_NAME = VK_EXT_debug_utils
      }
      return extensions;
    }

    void initVulkan() {
      glfwInit();//initialize glfw library
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//dont use opengl use vulkan
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);//prevent resising window because thats hard (update its going to be a long night)
      
      window=glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr); //width height title monitor? openglstuff(share share resorce with anuther window for speed and basically shallow opy them)
      glfwSetWindowUserPointer(window, this);
      glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

      createInstance();
      setupDebugMessenger();
      createSurface();
      pickPhysicalDevice();
      createLogicalDevice();
      createSwapChain();
      createImageViews();
      createRenderPass();
      createDescriptorSetLayout();
      createGraphicsPipeline();
      createFramebuffers();
      createCommandPool();
      createTextureImage();
      createVertexBuffer();
      createIndexBuffer();
      createUniformBuffers();
      createDescriptorPool();
      createDescriptorSets();
      createCommandBuffer();
      createSyncObjects();
    }

    void createTextureImage(){
      int texWidth, texHeight, texChannels;
      stbi_uc* pixels =stbi_load("textures/texture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
      VkDeviceSize imageSize=texWidth*texHeight*4;
      if(!pixels){
        throw std::runtime_error("failed to loard/find texture image");
      }

      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;

      createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
      void * data;
      vkMapMemory(device, stagingBufferMemory,0,imageSize,0,&data);
      memcpy(data, pixels, static_cast<size_t>(imageSize));
      vkUnmapMemory(device, stagingBufferMemory);
      stbi_image_free(pixels);
      
      createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
    
      transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
      transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory){
      VkImageCreateInfo imageInfo{};
      imageInfo.sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      imageInfo.imageType=VK_IMAGE_TYPE_2D;
      imageInfo.extent.width=width;
      imageInfo.extent.height=height;
      imageInfo.extent.depth=1;
      imageInfo.mipLevels=1;
      imageInfo.arrayLayers=1;
      imageInfo.format=format;
      imageInfo.tiling=tiling;
      imageInfo.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
      imageInfo.usage=usage;
      imageInfo.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
      imageInfo.samples=VK_SAMPLE_COUNT_1_BIT;
      
      if (vkCreateImage(device, &imageInfo, nullptr, &image)!=VK_SUCCESS){
        throw std::runtime_error("fialed to create image");
      }
      
      VkMemoryRequirements memRequirements;
      vkGetImageMemoryRequirements(device, textureImage, &memRequirements);
      VkMemoryAllocateInfo allocInfo{};
      allocInfo.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      allocInfo.allocationSize=memRequirements.size;
      allocInfo.memoryTypeIndex=findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      
      if(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory)!=VK_SUCCESS){
        throw std::runtime_error("failed to allocate image memory");
      }
      
      vkBindImageMemory(device, image, imageMemory,0);
    }

    VkCommandBuffer beginSingleTimeCommands(){
      VkCommandBufferAllocateInfo allocInfo{};
      allocInfo.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandPool=commandPool;
      allocInfo.commandBufferCount=1;

      VkCommandBuffer commandBuffer;
      vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

      vkBeginCommandBuffer(commandBuffer, &beginInfo);

      return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer){
      vkEndCommandBuffer(commandBuffer);
      
      VkSubmitInfo submitInfo{};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.commandBufferCount=1;
      submitInfo.pCommandBuffers=&commandBuffer;

      vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
      vkQueueWaitIdle(graphicsQueue);
      vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout){
      VkCommandBuffer commandBuffer=beginSingleTimeCommands(); 

      VkImageMemoryBarrier barrier{};
      barrier.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      barrier.oldLayout=oldLayout;
      barrier.newLayout=newLayout;
      barrier.srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
      barrier.image=image;
      barrier.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
      barrier.subresourceRange.baseMipLevel = 0;
      barrier.subresourceRange.levelCount = 1;
      barrier.subresourceRange.baseArrayLayer = 0;
      barrier.subresourceRange.layerCount = 1;
      
      VkPipelineStageFlags sourceStage;
      VkPipelineStageFlags destinateionStage;

      if(oldLayout==VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
        barrier.srcAccessMask=0;
        barrier.dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinateionStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
        barrier.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinateionStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      } else {
        throw std::invalid_argument("unsupported layout transition");
      }

      //also needs a update
      vkCmdPipelineBarrier(commandBuffer, sourceStage, destinateionStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

      endSingleTimeCommands(commandBuffer);
    }

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height){
      VkCommandBuffer commandBuffer = beginSingleTimeCommands();

      VkBufferImageCopy region{};
      region.bufferOffset=0;
      region.bufferRowLength=0;
      region.bufferImageHeight=0;
      
      region.imageSubresource.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
      region.imageSubresource.mipLevel=0;
      region.imageSubresource.baseArrayLayer=0;
      region.imageSubresource.layerCount=1;
      region.imageOffset ={0,0,0};
      region.imageExtent={width,height,1};
      vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&region);

      endSingleTimeCommands(commandBuffer);
    }

    void createDescriptorPool(){
      VkDescriptorPoolSize poolSize{};
      poolSize.type=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      poolSize.descriptorCount=static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
      
      VkDescriptorPoolCreateInfo poolInfo{};
      poolInfo.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      poolInfo.poolSizeCount=1;
      poolInfo.pPoolSizes=&poolSize;
      poolInfo.maxSets=static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

      if (vkCreateDescriptorPool(device, &poolInfo, nullptr,&descriptorPool)!=VK_SUCCESS){
        throw std::runtime_error("failed to create descirptor pool");
      }
    }

    void createDescriptorSets(){
      std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
      VkDescriptorSetAllocateInfo allocInfo{};
      allocInfo.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.descriptorPool=descriptorPool;
      allocInfo.descriptorSetCount=static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
      allocInfo.pSetLayouts=layouts.data();

      descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
      if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data())!=VK_SUCCESS){
        throw std::runtime_error("failed to allocate descriptor sets");
      }

      for (size_t i =0;i<MAX_FRAMES_IN_FLIGHT;i++){
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer=uniformBuffers[i];
        bufferInfo.offset=0;
        bufferInfo.range=sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet=descriptorSets[i];
        descriptorWrite.dstBinding=0;
        descriptorWrite.dstArrayElement=0;
        descriptorWrite.descriptorType=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount=1;
        descriptorWrite.pBufferInfo=&bufferInfo;
        descriptorWrite.pImageInfo=nullptr;
        descriptorWrite.pTexelBufferView=nullptr;
        
        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
      }
    }

    void createUniformBuffers(){
      VkDeviceSize buffersSize = sizeof(UniformBufferObject);
      uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
      uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
      uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

      for (size_t i = 0;i<MAX_FRAMES_IN_FLIGHT;i++){
        createBuffer(buffersSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        vkMapMemory(device, uniformBuffersMemory[i],0,buffersSize,0,&uniformBuffersMapped[i]);

      }
    }

    void createDescriptorSetLayout(){
      VkDescriptorSetLayoutBinding uboLayoutBinding{};
      uboLayoutBinding.binding=0;
      uboLayoutBinding.descriptorType=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      uboLayoutBinding.descriptorCount=1;
      uboLayoutBinding.stageFlags=VK_SHADER_STAGE_VERTEX_BIT;
      uboLayoutBinding.pImmutableSamplers=nullptr;

      VkDescriptorSetLayoutCreateInfo layoutInfo{};
      layoutInfo.sType= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      layoutInfo.bindingCount=1;
      layoutInfo.pBindings=&uboLayoutBinding;
      
      if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS){
        throw std::runtime_error("failed to create descriptor set layout");
      }
      
      VkPipelineLayoutCreateInfo piplineLayoutInfo{};
      piplineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      piplineLayoutInfo.setLayoutCount=1;
      piplineLayoutInfo.pSetLayouts=&descriptorSetLayout;
    }

    void createIndexBuffer(){
      VkDeviceSize bufferSize = sizeof(indicies[0]) * indicies.size();
      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;
      createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
      void* data;
      vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
      memcpy(data, indicies.data(), (size_t)bufferSize);
      vkUnmapMemory(device, stagingBufferMemory);
      createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

      copyBuffer(stagingBuffer, indexBuffer, bufferSize);

      vkDestroyBuffer(device, stagingBuffer, nullptr);
      vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void createVertexBuffer(){
      VkDeviceSize bufferSize = sizeof(verticies[0])*verticies.size();

      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;
      
      createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
      void* data;
      vkMapMemory(device, stagingBufferMemory, 0, bufferSize,0,&data);
      memcpy(data,verticies.data(),(size_t) bufferSize);
      vkUnmapMemory(device, stagingBufferMemory);
      createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
      
      copyBuffer(stagingBuffer,vertexBuffer,bufferSize);
      vkDestroyBuffer(device, stagingBuffer, nullptr);
      vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    //in order to is maps and better acsess memory we must transfer local memeory from buffer to vertex
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size){
      VkCommandBuffer commandBuffer=beginSingleTimeCommands();
      VkBufferCopy copyRegion{};
      copyRegion.size=size;
      vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
      endSingleTimeCommands(commandBuffer);
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory){
      VkBufferCreateInfo bufferInfo{};
      bufferInfo.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferInfo.size=size;
      bufferInfo.usage=usage;
      bufferInfo.sharingMode=VK_SHARING_MODE_EXCLUSIVE;

      if (vkCreateBuffer(device,&bufferInfo,nullptr,&buffer)!=VK_SUCCESS){
        throw std::runtime_error("failed to create vertex buffer");
      }

      VkMemoryRequirements memRequirements;
      vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
    
      VkMemoryAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      allocInfo.allocationSize=memRequirements.size;
      allocInfo.memoryTypeIndex=findMemoryType(memRequirements.memoryTypeBits, properties);
    
      if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory");
      }

      vkBindBufferMemory(device, buffer,bufferMemory, 0);
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties){
      VkPhysicalDeviceMemoryProperties memProperties;
      vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

      for (uint32_t i = 0;i<memProperties.memoryTypeCount;i++){
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags&properties)==properties) {
          return i;
        }
      }

      throw std::runtime_error("failed to find suitable memory type");
    }

    void createSyncObjects(){
      imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
      renderFinishedSemaphores.resize(swapChainImages.size());
      inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

      VkSemaphoreCreateInfo semaphoreInfo{};
      semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      VkFenceCreateInfo fenceInfo{};
      fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
      
      for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
          vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS){
          throw std::runtime_error("failed to create sync objects");
        }
      }
      for (size_t i = 0; i < swapChainImages.size(); i++){
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS){
          throw std::runtime_error("failed to create render finished semaphores");
        }
      }
    }

    void createCommandBuffer(){
      commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
      VkCommandBufferAllocateInfo allocInfo{};
      allocInfo.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.commandPool=commandPool;
      allocInfo.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandBufferCount=static_cast<uint32_t>(commandBuffers.size());
      if(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data())!=VK_SUCCESS){
        throw std::runtime_error("failed to allocate command buffers");
      }
    }

    void createCommandPool(){
      QueueFamilyIndicies QueueFamilyIndicies = findQueueFamilies(physicalDevice);
      VkCommandPoolCreateInfo poolInfo{};
      poolInfo.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      poolInfo.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
      poolInfo.queueFamilyIndex=QueueFamilyIndicies.graphicsFamily.value();
      if(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool)!=VK_SUCCESS){
        throw std::runtime_error("failed to create command pool");
      }
    }

    void createFramebuffers(){
      swapChainFramebuffers.resize(swapChainImageViews.size());
      for (size_t i = 0; i < swapChainImageViews.size();i++){
        VkImageView attachments []={swapChainImageViews[i]};
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass=renderPass;
        framebufferInfo.attachmentCount=1;
        framebufferInfo.pAttachments=attachments;
        framebufferInfo.width=swapChainExtent.width;
        framebufferInfo.height=swapChainExtent.height;
        framebufferInfo.layers=1;
        if(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i])!=VK_SUCCESS){
          throw std::runtime_error("failed to create frambuffer");
        }
      }
    }

    void createRenderPass(){
      VkAttachmentDescription colorAttachment{};
      colorAttachment.format = swapChainImageFormat;
      colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
      colorAttachment.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;//fresh frame no old
      colorAttachment.storeOp=VK_ATTACHMENT_STORE_OP_STORE;//store whats rendered in memory for later useage
      colorAttachment.stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;//regenerate evry time
      colorAttachment.stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;//no memory saving
      colorAttachment.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
      colorAttachment.finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//output is the swap chain
      
      VkAttachmentReference colorAttachmentRef{};
      colorAttachmentRef.attachment=0;
      colorAttachmentRef.layout=VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//preformance go brrrrrrrr
      
      VkSubpassDescription subpass{};
      subpass.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;//because we ant to actually see what we do
      subpass.colorAttachmentCount=1;
      subpass.pColorAttachments=&colorAttachmentRef;

      VkRenderPassCreateInfo renderPassInfo{};
      renderPassInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      renderPassInfo.attachmentCount=1;
      renderPassInfo.pAttachments=&colorAttachment;
      renderPassInfo.subpassCount=1;
      renderPassInfo.pSubpasses=&subpass;

      VkSubpassDependency dependency{};
      dependency.srcSubpass=VK_SUBPASS_EXTERNAL;
      dependency.dstSubpass=0;
      dependency.srcStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.srcAccessMask=0;
      dependency.dstStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.dstAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      renderPassInfo.dependencyCount=1;
      renderPassInfo.pDependencies=&dependency;
      if(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass)!=VK_SUCCESS){
        throw std::runtime_error("failed to create render pass");
      }
    }
    
    void createGraphicsPipeline(){
      auto vertShaderCode=readFile("shaders/vert.spv");
      auto fragShaderCode = readFile("shaders/frag.spv");

      VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
      VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
  
      VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
      vertShaderStageInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertShaderStageInfo.stage=VK_SHADER_STAGE_VERTEX_BIT;
      vertShaderStageInfo.module=vertShaderModule;
      vertShaderStageInfo.pName="main";
      
      VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
      fragShaderStageInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      fragShaderStageInfo.stage=VK_SHADER_STAGE_FRAGMENT_BIT;
      fragShaderStageInfo.module=fragShaderModule;
      fragShaderStageInfo.pName="main";

      VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
     
      auto bindingDescription = Vertex::getBindingDescription();
      auto attributeDescriptions = Vertex::getAttributeDescriptions();
      VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
      vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertexInputInfo.vertexBindingDescriptionCount = 1;
      vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
      vertexInputInfo.vertexAttributeDescriptionCount= static_cast<uint32_t>(attributeDescriptions.size());
      vertexInputInfo.pVertexAttributeDescriptions=attributeDescriptions.data();

      VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
      inputAssembly.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      inputAssembly.primitiveRestartEnable = VK_FALSE;

      VkViewport viewport{};//stretches
      viewport.x = 0.0f;
      viewport.y=0.0f;
      viewport.width=(float)swapChainExtent.width;
      viewport.height=(float)swapChainExtent.height;
      viewport.minDepth=0.0f;
      viewport.maxDepth=1.0f;

      VkRect2D scissor{};//crops
      scissor.offset={0,0};
      scissor.extent=swapChainExtent;

      std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
      VkPipelineDynamicStateCreateInfo dynamicState{};
      dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
      dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
      dynamicState.pDynamicStates = dynamicStates.data();

      VkPipelineViewportStateCreateInfo viewportState{};
      viewportState.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      viewportState.viewportCount=1;
      viewportState.pViewports=&viewport;
      viewportState.scissorCount=1;
      viewportState.pScissors=&scissor;

      //insert my load and savior a prebuilt rasterizer
      VkPipelineRasterizationStateCreateInfo rasterizer{};
      rasterizer.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      rasterizer.depthClampEnable = VK_FALSE;
      rasterizer.rasterizerDiscardEnable = VK_FALSE; //if this is true then the rest is useless 
      rasterizer.polygonMode=VK_POLYGON_MODE_FILL;//look here when i start using textures
      rasterizer.lineWidth=1.0f;
      rasterizer.cullMode=VK_CULL_MODE_BACK_BIT;
      rasterizer.frontFace=VK_FRONT_FACE_CLOCKWISE;
      rasterizer.depthBiasEnable=VK_FALSE;//basically a deph stretch or shrink effect
      rasterizer.depthBiasConstantFactor=0.0f;
      rasterizer.depthBiasClamp=0.0f;
      rasterizer.depthBiasSlopeFactor=0.0f;
      rasterizer.cullMode=VK_CULL_MODE_BACK_BIT;
      rasterizer.frontFace=VK_FRONT_FACE_COUNTER_CLOCKWISE;

      //anti-aliasing
      VkPipelineMultisampleStateCreateInfo multisampling{};
      multisampling.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisampling.sampleShadingEnable=VK_FALSE;
      multisampling.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;
      multisampling.minSampleShading=1.0f;
      multisampling.pSampleMask=nullptr;//proboly cpuld use soething like a scissor for this
      multisampling.alphaToCoverageEnable=VK_FALSE;
      multisampling.alphaToOneEnable=VK_FALSE;

      //if needed insert depth and stencil testing here

      VkPipelineColorBlendAttachmentState colorBlendAttachment{};
      colorBlendAttachment.colorWriteMask=VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      colorBlendAttachment.blendEnable = VK_TRUE;
      colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
      colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

      VkPipelineColorBlendStateCreateInfo colorBlending{};
      colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      colorBlending.logicOpEnable = VK_FALSE;
      colorBlending.logicOp = VK_LOGIC_OP_COPY;
      colorBlending.attachmentCount = 1;
      colorBlending.pAttachments = &colorBlendAttachment;
      colorBlending.blendConstants[0] = 0.0f;
      colorBlending.blendConstants[1] = 0.0f;
      colorBlending.blendConstants[2] = 0.0f;
      colorBlending.blendConstants[3] = 0.0f;

      VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
      pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipelineLayoutInfo.setLayoutCount=1;
      pipelineLayoutInfo.pSetLayouts=&descriptorSetLayout;
      pipelineLayoutInfo.pushConstantRangeCount=0;
      pipelineLayoutInfo.pPushConstantRanges=nullptr;
      if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout)!=VK_SUCCESS){
        throw std::runtime_error("failed to create pipeline layout!");
      }

      VkGraphicsPipelineCreateInfo pipelineInfo{};
      pipelineInfo.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      pipelineInfo.stageCount=2;
      pipelineInfo.pStages=shaderStages;
      pipelineInfo.pVertexInputState=&vertexInputInfo;
      pipelineInfo.pInputAssemblyState=&inputAssembly;
      pipelineInfo.pViewportState=&viewportState;
      pipelineInfo.pRasterizationState=&rasterizer;
      pipelineInfo.pMultisampleState=&multisampling;
      pipelineInfo.pDepthStencilState=nullptr;//we no do this
      pipelineInfo.pColorBlendState=&colorBlending;
      pipelineInfo.pDynamicState=&dynamicState;
      pipelineInfo.layout=pipelineLayout;
      pipelineInfo.renderPass=renderPass;
      pipelineInfo.subpass=0;//vulkan internally handels this
      pipelineInfo.basePipelineHandle=VK_NULL_HANDLE;//for inheratince
      pipelineInfo.basePipelineIndex=-1;//no

      if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)!=VK_SUCCESS){
        throw std::runtime_error("failed to create graphics pipeline");
      }

      //end of function code
      vkDestroyShaderModule(device, fragShaderModule, nullptr);
      vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    VkShaderModule createShaderModule(const std::vector<char>& code){
      VkShaderModuleCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      createInfo.codeSize=code.size();
      createInfo.pCode=reinterpret_cast<const uint32_t*>(code.data());
      VkShaderModule shaderModule;
      if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS){
        throw std::runtime_error("failed to create shader module");
      }

      return shaderModule;
    }

    void createImageViews(){
      swapChainImageViews.resize(swapChainImages.size());
      for (size_t i = 0;i<swapChainImages.size();i++){
        VkImageViewCreateInfo createinfo{};
        createinfo.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createinfo.image=swapChainImages[i];
        createinfo.viewType=VK_IMAGE_VIEW_TYPE_2D;
        createinfo.format=swapChainImageFormat;
        createinfo.components.r=VK_COMPONENT_SWIZZLE_IDENTITY;
        createinfo.components.g=VK_COMPONENT_SWIZZLE_IDENTITY;
        createinfo.components.b=VK_COMPONENT_SWIZZLE_IDENTITY;
        createinfo.components.a=VK_COMPONENT_SWIZZLE_IDENTITY;
        createinfo.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
        createinfo.subresourceRange.baseMipLevel=0;
        createinfo.subresourceRange.levelCount=1;
        createinfo.subresourceRange.baseArrayLayer=0;
        createinfo.subresourceRange.layerCount=1;
        if (vkCreateImageView(device,&createinfo,nullptr,&swapChainImageViews[i]) != VK_SUCCESS){
          throw std::runtime_error("failed to create image views");
        }
      }
    }

    void createSwapChain() {
      SwarpChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
      VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
      VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
      VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
      uint32_t imageCount = swapChainSupport.capabilities.minImageCount+1;//otherwise we can stall the gpu
      if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount){
        imageCount = swapChainSupport.capabilities.maxImageCount;
      }
      VkSwapchainCreateInfoKHR createInfo{};
      createInfo.sType=VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      createInfo.surface=surface;
      createInfo.minImageCount=imageCount;
      createInfo.imageFormat = surfaceFormat.format;
      createInfo.imageColorSpace = surfaceFormat.colorSpace;
      createInfo.imageExtent=extent;
      createInfo.imageArrayLayers = 1;
      createInfo.imageUsage=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
      createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
      createInfo.presentMode = presentMode;
      createInfo.clipped=VK_TRUE;
      createInfo.oldSwapchain=VK_NULL_HANDLE;

      QueueFamilyIndicies indices = findQueueFamilies(physicalDevice);
      uint32_t queueFamilyIndicies[] {indices.graphicsFamily.value(), indices.presentFamily.value()};
      if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndicies;
      } else {
        createInfo.imageSharingMode=VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
      }

      if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS){
        throw std::runtime_error("failed to create swap chain");
      }
     
      vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
      swapChainImages.resize(imageCount);
      vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

      swapChainImageFormat = surfaceFormat.format;
      swapChainExtent = extent;
    }

    void createSurface() {
      if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
          VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
      }
    }

    void createLogicalDevice() { 
      //back to extension stuff as we set up those queuefamilies from above with how manu ques they are to make
      QueueFamilyIndicies indices = findQueueFamilies(physicalDevice);
      std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
      std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
      float queuePriority = 1.0f;
      for(uint32_t queueFamily : uniqueQueueFamilies){
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
      }   
      VkPhysicalDeviceFeatures deviceFeatures{};
      //now we create the logic device with the above strucutre.
      VkDeviceCreateInfo createInfo{};
      createInfo.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      createInfo.pQueueCreateInfos=queueCreateInfos.data();
      createInfo.pEnabledFeatures=&deviceFeatures;
      createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
      createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
      createInfo.ppEnabledExtensionNames = deviceExtensions.data();
      if (enableValidationLayers){
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
      } else {
        createInfo.enabledLayerCount = 0;
      }
      //try to create the logic device
      if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS){
        throw std::runtime_error("failed to create logical device!");
      }
      vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
      vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    //technically this next function exists in th4 code biut it is faster to write it ourselves then try t find and thenc all it 
  //in effect it does as the anem says creates our vulkan debug utils messenger extenion
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instace, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger){
      auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
      if (func != nullptr){
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
      } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;//returns if issue loading func with this error
      }
    }

    static std::vector<char> readFile(const std::string& filename){
      std::ifstream file(filename,std::ios::ate | std::ios::binary);
      if(!file.is_open()){
        throw std::runtime_error("failed to open file");
      }

      size_t fileSize = (size_t)file.tellg();
      std::vector<char> buffer(fileSize);

      file.seekg(0);
      file.read(buffer.data(), fileSize);
      file.close();
      return buffer;
    }

    void setupDebugMessenger() {
      if (!enableValidationLayers) return;
      //we are creating a new extension here with details about the messenger and its callback based on the debug callback from earlier
      VkDebugUtilsMessengerCreateInfoEXT createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      createInfo.pfnUserCallback = debugCallback;
      createInfo.pUserData = nullptr; // Optional
      if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger");
      }
    }

    void mainLoop() {
      while (!glfwWindowShouldClose(window)){//while window is not closed
        glfwPollEvents();//starts collecting user input
        drawFrame();
        // glfwSetWindowShouldClose(window, GLFW_TRUE); //force close for checking on wayland commented out becuase this is bad
      }

      vkDeviceWaitIdle(device);
    }

    void drawFrame(){
      vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
      
      uint32_t imageIndex;
      VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
      if (result == VK_ERROR_OUT_OF_DATE_KHR){
        recreateSwapChain();
        return;
      } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        throw std::runtime_error("failed to acqueire swap chain image");
      }

      vkResetFences(device, 1, &inFlightFences[currentFrame]);

      vkResetCommandBuffer(commandBuffers[currentFrame], 0);
      recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

      VkSemaphore signalSemaphores[]={renderFinishedSemaphores[imageIndex]};
      VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
      VkPipelineStageFlags waitStages[]={VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
      updateUniformBuffer(currentFrame);
      VkSubmitInfo submitInfo{};
      submitInfo.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.waitSemaphoreCount=1;
      submitInfo.pWaitSemaphores=waitSemaphores;
      submitInfo.pWaitDstStageMask=waitStages;
      submitInfo.commandBufferCount=1;
      submitInfo.pCommandBuffers=&commandBuffers[currentFrame];
      submitInfo.signalSemaphoreCount=1;
      submitInfo.pSignalSemaphores=signalSemaphores;
      if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame])!=VK_SUCCESS){
        throw std::runtime_error("failed to sbmit draw command buffer");
      }

      VkPresentInfoKHR presentInfo{};
      presentInfo.sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
      presentInfo.waitSemaphoreCount=1;
      presentInfo.pWaitSemaphores=signalSemaphores;

      VkSwapchainKHR swapChains[]={swapChain};
      presentInfo.swapchainCount=1;
      presentInfo.pSwapchains=swapChains;
      presentInfo.pImageIndices=&imageIndex;
      presentInfo.pResults=nullptr;

      vkQueuePresentKHR(presentQueue, &presentInfo);
      currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    } 

    //spinning time look at me later for handling rotation of objects or rotation of camera more likley
    void updateUniformBuffer(uint32_t currentImage){
      static auto startTime = std::chrono::high_resolution_clock::now();
      auto currentTime = std::chrono::high_resolution_clock::now();
      float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

      UniformBufferObject ubo{};
      ubo.model=glm::rotate(glm::mat4(1.0f), time*glm::radians(90.0f), glm::vec3(0.0f,0.0f,1.0f));
      
      ubo.view=glm::lookAt(glm::vec3(2.0f,2.0f,2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,0.0f,1.0f));
      ubo.proj=glm::perspective(glm::radians(45.0f), swapChainExtent.width/(float)swapChainExtent.height, 0.1f, 10.0f);
      ubo.proj[1][1]*=-1;
      memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    //this is a functiont o detroy the beguv messagner extion layer basically a destructor with weird iplimention if i understand property 
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator){
      auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");//i used the extension to destroy the extension
      if (func != nullptr){
        func(instance,debugMessenger,pAllocator);
      }
    }

    void cleanupSwapChain(){
      for (auto framebuffer : swapChainFramebuffers){
        vkDestroyFramebuffer(device, framebuffer, nullptr);
      }
      for (auto imageView : swapChainImageViews){
        vkDestroyImageView(device, imageView, nullptr);
      }
      vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    void recreateSwapChain(){
      int width = 0, height = 0;
      glfwGetFramebufferSize(window, &width, &height);
      while (width == 0 || height == 0){ // handle minimization
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
      }
      vkDeviceWaitIdle(device);
      cleanupSwapChain();
      createSwapChain();
      createImageViews();
      createFramebuffers();
    }

    void cleanup() {
      vkDeviceWaitIdle(device); 

      for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
      }
      for (size_t i = 0; i < renderFinishedSemaphores.size(); i++){
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
      }
      vkDestroyCommandPool(device, commandPool, nullptr);

      cleanupSwapChain();

      vkDestroyImage(device, textureImage, nullptr);
      vkFreeMemory(device, textureImageMemory, nullptr);
      
      for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
      }

      vkDestroyDescriptorSetLayout(device, descriptorSetLayout,nullptr);
      vkDestroyBuffer(device, indexBuffer, nullptr);
      vkFreeMemory(device, indexBufferMemory, nullptr);
      vkDestroyBuffer(device, vertexBuffer, nullptr);
      vkFreeMemory(device,vertexBufferMemory,nullptr);
      vkDestroyPipeline(device, graphicsPipeline, nullptr);
      vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
      vkDestroyRenderPass(device, renderPass, nullptr);

      vkDestroyDescriptorPool(device, descriptorPool, nullptr);
      vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
 
      vkDestroyDevice(device, nullptr);
      if (enableValidationLayers){
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
      }
      vkDestroySurfaceKHR(instance, surface, nullptr);
      vkDestroyInstance(instance, nullptr); //i beleive that this destroyed the instance and sets its poitns to nullptr may be wrong?
      glfwDestroyWindow(window);//when window closes destroy its
      glfwTerminate();//stop the glfw library background tasks
    }
};

int main() {
    HelloTriangleApplication app;
    
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

  
    return EXIT_SUCCESS;
}
