#include "Engine.h"
#include "fixed.h"
#include "map.h"
#include <memory>
#include "enimy.h"

struct UpdateState {
  enimy* enemies[3];
  int count;
  int* playerHealth;
} gUpdateState;

void gameUpdate(Engine& e) {
  for (int i = 0; i < gUpdateState.count; i++) {
    gUpdateState.enemies[i]->stateTransition();
  }
  if (*gUpdateState.playerHealth <= 0) {
    std::cout << "game over" << std::endl;
    exit(0);
  }
}

int main() {
  Engine app;
  enimy* enemyPtrs[3];
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
    enimyF.Scale={0.5,0.5,0.5};
    enimyF.init("models/little_robot.obj", "textures/material12.png", app);
    std::cerr << "enimyF init done" << std::endl;

    fixed player;
    player.init("", "", app);
    player.aabbMin = {-0.3f, -0.3f, -0.9f};
    player.aabbMax = { 0.3f,  0.3f,  0.9f};
    player.Position = glm::vec3(1.0f, 1.0f, 0.0f);//debug mode fly above the map
    player.hasCollider = true;
    app.player = &player;
    app.add(&player);
    int* playerhealth=&app.playerHealth;

    for (int i = 0; i < 3; i++) {
      enemyPtrs[i] = new enimy(&mapTxt, enimyF, &app);
      std::cerr << "enemy " << i << " created" << std::endl;
    }

    gUpdateState.enemies[0] = enemyPtrs[0];
    gUpdateState.enemies[1] = enemyPtrs[1];
    gUpdateState.enemies[2] = enemyPtrs[2];
    gUpdateState.count = 3;
    gUpdateState.playerHealth = &app.playerHealth;

    app.setUpdateCallback(gameUpdate);

    app.loop();
    std::cerr << "loop exited" << std::endl;
    for (int i = 0; i < 3; i++) delete enemyPtrs[i];
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    for (int i = 0; i < 3; i++) delete enemyPtrs[i];
    return EXIT_FAILURE;
  }
  for (int i = 0; i < 3; i++) delete enemyPtrs[i];
  return EXIT_SUCCESS;
};
