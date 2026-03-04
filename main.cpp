#include "first_app.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
int main() {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  lve::FirstApp app{};

  try {
    app.run();
  } catch (const std::exception &e){
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
