#pragma once 
#include "lve_window.hpp"
#include "lve_pipeline.hpp"
namespace lve {
  class FirstApp{
    private:
      LveWindow LveWindow{WIDTH,HEIGHT, "Hello Vulkan!"};
      LvePipeline LvePipeline{"shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv"};
      
    public:
      static constexpr int WIDTH = 800;
      static constexpr int HEIGHT = 600;

      void run();
  };
  } // namespace lve
