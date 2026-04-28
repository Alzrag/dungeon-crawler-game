#pragma once

#include "Engine.h"
#include "GameObject.h"
#include "fixed.h"
#include <vector>

class enimy {
  public:
    std::vector<std::vector<char>> spawn(std::vector<std::vector<char>> map, fixed object, Engine& game);
    void takeDamage();
    std::vector<std::vector<char>> move(std::vector<std::vector<char>> map);
  private:
    int state;
    int health = (((level-1)*10)+20);
    int damage = (((level-1)*10)+20);
    int level;
    fixed self;
    Engine& app;
};

