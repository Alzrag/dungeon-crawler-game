#include "Engine.h"
#include "fixed.h"
#include "map.h"
#include <memory>
#include "enimy.h"

int main() {
  Engine app;  
  try {
    app.init();

    std::vector<std::vector<char>> mapTxt = generate_map(10, 10, 12);
    std::vector<fixed> mapraw = convertMap(mapTxt, app);
    std::vector<std::unique_ptr<fixed>> map;
    map.reserve(mapraw.size());
    for(fixed& block : mapraw){
      map.push_back(std::make_unique<fixed>(std::move(block)));
      app.add(map.back().get());
    }

    fixed enimyF;
    enimyF.init("models/little_robot.obj", "textures/material12.png", app);
    std::vector<std::unique_ptr<enimy>> enemies;
    enemies.reserve(100);
    for (int i = 0; i < 100; i++) {
      enemies.push_back(std::make_unique<enimy>(&mapTxt, enimyF, &app));
    }
    fixed player;
    player.init("", "", app);
    player.aabbMin = {-0.3f, -0.3f, -0.9f};
    player.aabbMax = { 0.3f,  0.3f,  0.9f};
    player.hasCollider = true;
    app.player = &player;
    app.add(&player);

    app.setUpdateCallback([&player](Engine& app){
      player.Position = app.cameraPos; 
    });

    app.loop();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
