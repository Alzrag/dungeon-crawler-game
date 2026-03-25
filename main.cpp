#include <cstdint>
#include <stdexcept>
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
      return indices.isComplete();
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
      glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//prevent resising window because thats hard 
      
      window=glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr); //width height title monitor? openglstuff(share share resorce with anuther window for speed and basically shallow opy them)
      
      createInstance();
      setupDebugMessenger();
      createSurface();
      pickPhysicalDevice();
      createLogicalDevice();
    }

    void createSurface(){
      if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS){
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
      createInfo.enabledExtensionCount = 0;//ignored
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
        // glfwSetWindowShouldClose(window, GLFW_TRUE); //force close for checking on wayland commented out becuase this is bad
      }
    }

    //this is a functiont o detroy the beguv messagner extion layer basically a destructor with weird iplimention if i understand property 
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator){
      auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");//i used the extension to destroy the extension
      if (func != nullptr){
        func(instance,debugMessenger,pAllocator);
      }
    }

    void cleanup() {
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
