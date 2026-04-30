#include "Engine.h"
#include "fixed.h"
#include "map.h"
#include <memory>
#include "enimy.h"

int main() {
  Engine app;  
  try {
    app.init();
    std::cerr << "init done" << std::endl;

    std::vector<std::vector<char>> mapTxt = generate_map(10, 10, 12);
    std::cerr << "map generated" << std::endl;

    std::vector<std::unique_ptr<fixed>> map = convertMap(mapTxt, app);
    for (auto& block : map){
      app.add(block.get());
    }
    std::cerr << "map added to scene" << std::endl;

    fixed enimyF;
    enimyF.init("models/little_robot.obj", "textures/material12.png", app);
    std::cerr << "enimyF init done" << std::endl;

    std::vector<std::unique_ptr<enimy>> enemies;
    enemies.reserve(100);
    for (int i = 0; i < 100; i++) {
      enemies.push_back(std::make_unique<enimy>(&mapTxt, enimyF, &app));
      std::cerr << "enemy " << i << " created" << std::endl;
    }

    fixed player;
    player.init("", "", app);
    player.aabbMin = {-0.3f, -0.3f, -0.9f};
    player.aabbMax = { 0.3f,  0.3f,  0.9f};
    player.Position = glm::vec3(1.0f, 1.0f, 10.0f);//debug mode fly above the map
    player.hasCollider = true;
    app.player = &player;
    app.add(&player);

    app.loop();
    std::cerr << "loop exited" << std::endl;
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
