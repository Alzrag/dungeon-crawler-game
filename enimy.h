#pragma once

#include "Engine.h"
#include "GameObject.h"
#include "fixed.h"
#include <glm/ext/vector_float3.hpp>
#include <vector>
#include <random>
#include <string>
#include <deque>

class enimy {
  public:
    enimy(std::vector<std::vector<char>>* map, const fixed& object, Engine* game);
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
    void stateTransition();
    glm::vec3 newPosition;
    std::deque<glm::vec3> pathQueue;
    glm::vec3 currentPos;
    glm::vec3 targetPos;
    float moveSpeed = 5.0f;
    bool moving = false; 
};

