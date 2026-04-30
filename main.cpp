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

    std::vector<fixed> mapraw = convertMap(mapTxt, app);
    std::cerr << "map converted, size=" << mapraw.size() << std::endl;

    std::vector<std::unique_ptr<fixed>> map;
    map.reserve(mapraw.size());
    for(fixed& block : mapraw){
      map.push_back(std::make_unique<fixed>(std::move(block)));
      app.add(map.back().get());
    }
    std::cerr << "map added to scene" << std::endl;

    fixed enimyF;
    enimyF.init("models/little_robot.obj", "textures/material12.png", app);
    std::cerr << "enimyF init done" << std::endl;

    std::vector<std::unique_ptr<enimy>> enemies;
    enemies.reserve(3);
    for (int i = 0; i < 3; i++) {
      enemies.push_back(std::make_unique<enimy>(&mapTxt, enimyF, &app));
      std::cerr << "enemy " << i << " created" << std::endl;
    }

    fixed player;
    player.init("", "", app);
    std::cerr << "player init done" << std::endl;

    app.loop();
    std::cerr << "loop exited" << std::endl;
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
