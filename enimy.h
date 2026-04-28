#pragma once

#include "Engine.h"
#include "GameObject.h"
#include "fixed.h"
#include <glm/ext/vector_float3.hpp>
#include <vector>
#include <random>
#include <string>

class enimy {
  public:
    enimy(std::vector<std::vector<char>>* map, fixed object, Engine* game);
    void takeDamage(int amount);
    void move();
    void hurt();
  private:
    glm::vec3 position;
    std::string state;
    int health;
    int damage;
    int level;
    fixed self;
    Engine* app;
    std::vector<std::vector<char>>* map;
};

