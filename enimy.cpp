#include "enimy.h"
#include <random>

enimy::enimy(std::vector<std::vector<char>>* mapIn, fixed object, Engine* game){
  app = game;
  level=1;
  health = (((level-1)*10)+20);
  damage = (((level-1)*10)+20);
  map = mapIn;
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> pos(1,mapIn->size());
  position = {pos(mt), pos(mt), 1.0f};
  while (mapIn->at(position.x+1).at(position.y+1)!=' '){
    position = {pos(mt), pos(mt), 1.0f};
  }
}

void enimy::takeDamage(int amount){
  health=health+=amount;
}

void enimy::hurt(){
  app->playerHealth+=damage;
}

void enimy::move(){
  if (state == "wander"){
    glm::vec3 target;
    position = {pos(mt), pos(mt), 1.0f};
    while (mapIn->at(position.x+1).at(position.y+1)!=' '){
      position = {pos(mt), pos(mt), 1.0f};
    }
  }
}

void stateTransition(){

}


