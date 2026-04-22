#include "helpers.h"

//wow weird way to declare a function but looks likes is a VKAPI_CALL of type VkBool32 of type VKAPI_ATTR like how a vector can be of type int
//i know i could and will do this with neovim but the paremters are in order:
//the severity of the messages
//the type of messages
//a pointer to a struct with more details about the error 
//a a pointer to additinal user data
//in effect we are building our own error statement here
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messsageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData){
  std::cerr << "validation Layer: " << pCallbackData->pMessage << std::endl;
  return VK_FALSE;
}

bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers){
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

//allows us to handle output of debuggin messages with a addtional extenion yippy more exteions and layers most likley down the line
std::vector<const char*> getRequiredExtensions(bool enableValidationLayers) {
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);//VK_EXT_DEBUG_UTILS_EXTENSION_NAME = VK_EXT_debug_utils
  }
  return extensions;
}

std::vector<char> readFile(const std::string& filename){
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

bool hasStencilComponent(VkFormat format){
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
