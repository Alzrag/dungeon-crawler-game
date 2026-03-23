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
 
    void pickPhysicalDevice(){
      VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;//hold current phsycial device
      uint32_t deviceCount =0;//numbers of availbile vulkan gpu's
      vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);//check how many there are
      if (deviceCount == 0){
        throw std::runtime_error("failed to find GPU's with Vulkan support! (please check you device, drivers, and vulkan instialation[should have been automatic])");//if its 0 there is a issue
      }
      std::vector<VkPhysicalDevice> devices(deviceCount);//now that we know there s no error we can add them to a list
      vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
      for (const auto& device: devices){//first check for dedicated gpu
        if (isDeviceSuitable(device, true)){
          physicalDevice = device;
          break;
        }
      }
      if (physicalDevice == VK_NULL_HANDLE){
        for (const auto& device: devices){//if not dediated check intigrated
          if (isDeviceSuitable(device, false)){
            physicalDevice = device;
            break;
          }
        }
      }
      if (physicalDevice == VK_NULL_HANDLE){//if neither there is a issue
        throw std::runtime_error("failed to find a suitable GPU! (please check you device, drivers, and vulkan instialation[should have been automatic])");
      }
    }

    bool isDeviceSuitable(VkPhysicalDevice device, bool dedTintF){
      //start be getting the properties and features of the device
      VkPhysicalDeviceProperties deviceProperties;
      vkGetPhysicalDeviceProperties(device, &deviceProperties);
      VkPhysicalDeviceFeatures deviceFeatures;
      vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
      return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader && dedTintF == true || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && deviceFeatures.geometryShader && dedTintF == false;
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
      if (enableValidationLayers){
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
      }
      
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
