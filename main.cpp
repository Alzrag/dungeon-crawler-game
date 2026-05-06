#include "Engine.h"
#include "fixed.h"
#include "map.h"
#include "enimy.h"


//a strucutre to hold some state of the game data durring update
struct UpdateState {
  enimy* enemies[3];
  int count;
  int* playerHealth;
} gUpdateState;

/**
 * @brief a loop to happens every fram of the gam that acts as a logic call back used in the enginer main loop function
 *
 * currently it is used to itterate over all enimies and trigger their updates ie state transitions;
 * additioanly it preforms some checks like checking player health
 *
 * @param e a reference to the running engine instance
 */
void gameUpdate(Engine& e) {
  e.playerHealth+=0;//i prevent warnings as the main loop needs a engine but this implimentation in main does not
  for (int i = 0; i < gUpdateState.count; i++) {
    gUpdateState.enemies[i]->stateTransition();
  }
  if (*gUpdateState.playerHealth <= 0) {
    std::cout << "game over" << std::endl;
    exit(0);
  }
}

/**
 * @brief the entry/start point of the program
 *
 * inistalizies the game engine, creates the map, populates the scene, registed the update callback and enters the render loop with clean up logic at the end
 *
 * @return EXIT_SUCCESS on clena shutdown, EXIT_FAILURE if an except is thrown
 */
int main() {
  Engine app;
  enimy* enemyPtrs[3];
  try {
    app.init();
    std::cerr << "init done" << std::endl;

    std::vector<std::vector<char>> mapTxt = generate_map(10, 10, 12);
    std::cerr << "map generated" << std::endl;

    std::vector<fixed*> map = convertMap(mapTxt, app);
    for (fixed* obj : map){
      app.add(obj);
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

    app.playerHealth=1000;
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
