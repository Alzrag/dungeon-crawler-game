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


/**
 * @brief checks weather all of the requested validtiaon layers are aailible on this system.
 *
 * enumerates the instance layer properties reported by the vulkan runtime and check if eervy name in validation layers actually is here
 *
 * @param validationLayers list of all null-terminatd layer names as strings to check
 * @return true if every requested layer is here false if not
 */
bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers);

//allows us to handle output of debuggin messages with a addtional extenion yippy more exteions and layers most likley down the line
std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);

/**
 * @brief reads an entire bionary file into a vector char buffer for use by the hasStencilComponent
 *
 * determines the size with std::ios::ate allocates a buffer for that size goes to the beggining reads all bytes and loads the SPIR-V shader bionaries 
 *
 * @param filename path to the file to read
 * @return a vecotr contiagning all bytes of the file
 * @throws std::runtime_error if the file can be opened
 */
std::vector<char> readFile(const std::string& filename);

/**
 * @brief selects the prefered swa chain surface format from a availible list
 *
 *preferes VK_FORMAT_B8G8R8A8_SRGB with VK_COLOR_SPACE_SRGB_NONLINEAR_KHR for visual resons. other wise it falls back to the first abilible format
 *
 * @param availbileFormats lsit of surfac formats reported byt he device to be availible 
 * @return the chosen VkSurfaceFormatKHR
 */
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availbileFormats);

/**
 * @brief Selects the prefered swap-chain presentatio mode from a avilible list
 *
 * @param availiblePresentModes list of present modes reported by the gpu to be availible 
 * @return the chosen VkPresentModeKHR
 */
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availiblePresentModes);

/**
 * @brief determines if the vulkan dpeth format includes a stencil component
 *
 * @param format the VKFormat
 * @return true if format is VK_FORMAT_D32_SFLOAT_S8_UINT or VK_FORMAT_D24_UNORM_S8_UINT, flase otherwise
 */
bool hasStencilComponent(VkFormat format);
