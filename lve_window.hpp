#pragma once 
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <string>
namespace lve{

  class LveWindow {
    private:

      void initWindow();

      int width;
      int height;

      std::string windowName;
      GLFWwindow *window;

    public:
      LveWindow(int w, int h, std::string name);
      ~LveWindow();
  
      LveWindow(const LveWindow &) = delete;
      LveWindow &operator=(const LveWindow &) = delete;

      bool shouldClose () { return glfwWindowShouldClose(window); }
  };
}
