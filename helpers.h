#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstring>

template<typename T>
T clamp(T value, T min, T max){
  if (value < min) return min;
  if (value > max) return max;
  return value;
}
//wow weird way to declare a function but looks likes is a VKAPI_CALL of type VkBool32 of type VKAPI_ATTR like how a vector can be of type int
//i know i could and will do this with neovim but the paremters are in order:
//the severity of the messages
//the type of messages
//a pointer to a struct with more details about the error 
//a a pointer to additinal user data
//in effect we are building our own error statement here
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messsageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers);

//allows us to handle output of debuggin messages with a addtional extenion yippy more exteions and layers most likley down the line
std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);

std::vector<char> readFile(const std::string& filename);

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availbileFormats);

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availiblePresentModes);

bool hasStencilComponent(VkFormat format);
