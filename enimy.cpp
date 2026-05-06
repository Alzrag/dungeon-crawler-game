#include "enimy.h"
#include <glm/geometric.hpp>
#include <random>
#include "map.h"

std::random_device rd;
std::mt19937 mt(rd());

/**
 * @brief constrcuts an enimy placing it at a random open spot in the maze 
 *
 * @param vector the map of the ascii grid for logic later 
 * @param object the object to use as a base 
 * @param game the engine to hook into
 */
enimy::enimy(std::vector<std::vector<char>>* mapIn, const fixed& object, Engine* game){
  app = game;
  level=1;
  health = (((level-1)*10)+20);
  damage = -1*(((level-1)*10)+20);
  map = mapIn;
  self = object;
  std::uniform_int_distribution<int> posRow(0, (int)map->size()-1);
  std::uniform_int_distribution<int> posCol(0, (int)(*map)[0].size()-1);
  newPosition = {posRow(mt), posCol(mt), 1.0f};
  while (mapIn->at(static_cast<size_t>(newPosition.x)).at(static_cast<size_t>(newPosition.y))!=' '){
    newPosition = {posRow(mt), posCol(mt), 1.0f};
  }
  position=newPosition;
  currentPos=newPosition;
  self.Position = {newPosition.x, newPosition.y, 0.0f};
  position = newPosition;
  currentPos = newPosition;
  app->add(&self); 
  state="wander";
}

/**
 * @brief reduces the enimys healthy by a givin negative amount
 *
 * @param amount the amount to change the helath by negitive to take damgae positive to heal
 */
void enimy::takeDamage(int amount){
  health+=amount;
}

/**
 * @brief applies the enimys damage to the player
 */
void enimy::hurt(){
  app->playerHealth+=damage;
  std::cout<<"you have been hurt your health is now: "<<app->playerHealth<<std::endl;
}

/**
 * @class node
 * @brief a structure used to hold data for prcoessing the maze durring A* navigation and enimy path finding
 *
 */
struct node {
  int x,y;
  float f,g,h;
  bool open;
  node* parrent=nullptr;
  bool closed=false;

  node(){
    x=0;
    y=0;
    f=0;
    g=0;
    h=0;
    open=false;
  }
  void set(int xI,int yI,float gI,float hI, node* p){
    x=xI;
    y=yI;
    g=gI;
    h=hI;
    f=g+h;
    open=true;
    parrent=p;
  }
};

/**
 * @brief a distance calculator calcuating along the maze between to points
 *
 * @param x1 x cordinate 1 
 * @param x2  x cordianate 2 
 * @param y1 y cordinate 1 
 * @param y2 y cordianate 2
 * @return returns a float of the distance along the baze between to points
 */
float distance(int x1, int x2, int y1, int y2){
  return static_cast<float>(abs(x1-x2) + abs(y1-y2));
}

/**
 * @brief checks if the cadinatal positions form a node are valid or invalud
 *
 * @param x x positon of target node 
 * @param y y poisiotn of target node 
 * @param Maze the maze ascii 
 * @return bool if it is a wall or not
 */
bool isValid(int x, int y, const std::vector<std::vector<char>>& Maze){
  if(y<0 || static_cast<size_t>(y)>=Maze.size() || x<0 || static_cast<size_t>(x)>=Maze[0].size()) return false;
  if(Maze[static_cast<size_t>(y)][static_cast<size_t>(x)]=='#') return false;
  return true;
}

/**
 * @brief a fucntion to revese a vecotr of anything givin its start and end 
 *
 * @tparam Iterator the type of the vecotr 
 * @param begin the begining index 
 * @param end  the ending index 
 */
template <typename Iterator>
void reverse(Iterator begin, Iterator end) {
  --end;
  while (begin < end) {
    auto temp = *begin;
    *begin = *end;
    *end = temp;
    begin++;
    end--;
  }
}

/**
 * @brief finds the path between two points with A*
 *
 * @param Maze ascii of the maze 
 * @return a vector of posiotns to deque through
 */
std::vector<glm::vec3> solveMaze(std::vector<std::vector<char>>& Maze){ 
  size_t rows = Maze.size();
  size_t cols = Maze[0].size();
  node* start=nullptr;
  node* end(nullptr);
  std::vector<std::vector<node>> grid(rows, std::vector<node>(cols));

  //build grid
  for (size_t y = 0; y < rows; y++){
    for (size_t x = 0; x < cols; x++){
      grid[y][x].y = static_cast<int>(y);
      grid[y][x].x = static_cast<int>(x);
      if (Maze[y][x]=='E') start=&grid[y][x];
      if (Maze[y][x]=='S') end=&grid[y][x];
    }
  }
  if (start == nullptr || end == nullptr) return {};
  std::vector<node*> open;
  start->set(start->x,start->y,0,distance(start->x, end->x, start->y, end->y), nullptr);
  open.push_back(start);

  while(!open.empty()){
    int bestOp=0;
    for (int i = 1; static_cast<size_t>(i) < open.size(); i++){
      if(open[static_cast<size_t>(i)]->f < open[static_cast<size_t>(bestOp)]->f){
        bestOp = i;
      }
    }
    node* current = open[static_cast<size_t>(bestOp)];

    if(current->x==end->x&&current->y==end->y){ 
      std::vector<glm::vec3> path;
      node* temp = current->parrent;
      while(temp!=nullptr && Maze[static_cast<size_t>(temp->y)][static_cast<size_t>(temp->x)]!='E'){
        path.push_back({(float)temp->y, (float)temp->x, 1.0f});
        temp=temp->parrent;
      }
      if (!path.empty()) reverse(path.begin(), path.end());
      return path;
    }

    open.erase(open.begin()+bestOp);
    current->closed=true;
    if(Maze[static_cast<size_t>(current->y)][static_cast<size_t>(current->x)]!='E'&&Maze[static_cast<size_t>(current->y)][static_cast<size_t>(current->x)]!='S')Maze[static_cast<size_t>(current->y)][static_cast<size_t>(current->x)]='V';

    int dx[]={0,0,-1,1};
    int dy[]={-1,1,0,0};

    for (int i=0;i<4;i++){
      int nx=current->x+dx[i];
      int ny=current->y+dy[i];

      if (isValid(nx, ny, Maze)){
        node* neighbor = &grid[static_cast<size_t>(ny)][static_cast<size_t>(nx)]; 
        if (neighbor->closed){
          continue;
        }
        float g = current->g+1.0f;
        if(!neighbor->open||g<neighbor->g){
          neighbor->set(nx,ny,g,distance(nx, end->x, ny, end->y), current);
          bool allreadyOpened=false;
          for (node* n: open){
            if(n==neighbor){
              allreadyOpened=true;
              break;
            }
          }
          if(!allreadyOpened){
            open.push_back(neighbor);
            if(Maze[static_cast<size_t>(neighbor->y)][static_cast<size_t>(neighbor->x)]!='E'&& Maze[static_cast<size_t>(neighbor->y)][static_cast<size_t>(neighbor->x)]!='S') Maze[static_cast<size_t>(neighbor->y)][static_cast<size_t>(neighbor->x)]='N';
          }
        }
      }
    }
  }
  return {};
}

/**
 * @brief same as aboce but he target position is allways the players positon 
 *
 * @param Maze the ascii of the maze for logic 
 * @return the vector to be dequed through to reach players position
 */
std::vector<glm::vec3> playerDistance(std::vector<std::vector<char>>& Maze){
  size_t rows = Maze.size();
  size_t cols = Maze[0].size();
  node* start=nullptr;
  node* end(nullptr);
  std::vector<std::vector<node>> grid(rows, std::vector<node>(cols));

  //build grid
  for (size_t y = 0; y < rows; y++){
    for (size_t x = 0; x < cols; x++){
      grid[y][x].y = static_cast<int>(y);
      grid[y][x].x = static_cast<int>(x);
      if (Maze[static_cast<size_t>(y)][static_cast<size_t>(x)]=='E') start=&grid[static_cast<size_t>(y)][static_cast<size_t>(x)];
      if (Maze[static_cast<size_t>(y)][static_cast<size_t>(x)]=='S') end=&grid[static_cast<size_t>(y)][static_cast<size_t>(x)];
    }
  }
  if (start == nullptr || end == nullptr) return {};
  std::vector<node*> open;
  start->set(start->x,start->y,0,distance(start->x, end->x, start->y, end->y), nullptr);
  open.push_back(start);

  while(!open.empty()){
    size_t bestOp = 0;
    for (size_t i = 1; i < open.size(); i++){
      if(open[i]->f < open[bestOp]->f){
        bestOp=i;
      }
    }
    node* current = open[bestOp];

    if(current->x==end->x&&current->y==end->y){ 
      std::vector<glm::vec3> path;
      node* temp = current->parrent;
      while(temp!=nullptr&&Maze[static_cast<size_t>(temp->y)][static_cast<size_t>(temp->x)]!='E'){
        path.push_back({(float)temp->y, (float)temp->x, 1.0f});
        temp=temp->parrent;
      }
      if (!path.empty()) reverse(path.begin(), path.end());
      return path;
    }

    open.erase(open.begin()+static_cast<int>(bestOp));
    current->closed=true;
    if(Maze[static_cast<size_t>(current->y)][static_cast<size_t>(current->x)]!='E'&&Maze[static_cast<size_t>(current->y)][static_cast<size_t>(current->x)]!='S')Maze[static_cast<size_t>(current->y)][static_cast<size_t>(current->x)]='V';

    int dx[]={0,0,-1,1};
    int dy[]={-1,1,0,0};

    for (int i=0;i<4;i++){
      int nx=current->x+dx[i];
      int ny=current->y+dy[i];

      if (isValid(nx, ny, Maze)){
        node* neighbor = &grid[static_cast<size_t>(ny)][static_cast<size_t>(nx)]; 
        if (neighbor->closed){
          continue;
        }
        float g = current->g+1.0f;
        if(!neighbor->open||g<neighbor->g){
          neighbor->set(nx,ny,g,distance(nx, end->x, ny, end->y), current);
          bool allreadyOpened=false;
          for (node* n: open){
            if(n==neighbor){
              allreadyOpened=true;
              break;
            }
          }
          if(!allreadyOpened){
            open.push_back(neighbor);
            if(Maze[static_cast<size_t>(neighbor->y)][static_cast<size_t>(neighbor->x)]!='E'&&Maze[static_cast<size_t>(neighbor->y)][static_cast<size_t>(neighbor->x)]!='S')Maze[static_cast<size_t>(neighbor->y)][static_cast<size_t>(neighbor->x)]='N';
          }
        }
      }
    }
  }
  return {};
}

/**
 * @brief moves tghe enimy one set at a time so it doesnt go through walls movmeent is broken up by delta time and calls the correct functiont o identify target path 
 */
void enimy::move(){
  if (state == "wander"){
    float dt=app->dt;
    if (!moving) {
      if (pathQueue.empty()) {
        std::uniform_int_distribution<int> posRow(1, (int)map->size()-1);
        std::uniform_int_distribution<int> posCol(1, (int)(*map)[0].size()-1);
        glm::vec3 newDest = {(float)posRow(mt), (float)posCol(mt), 1.0f};
        while (map->at(static_cast<size_t>(newDest.x)).at(static_cast<size_t>(newDest.y)) != ' '){
          newDest = {(float)posRow(mt), (float)posCol(mt), 1.0f};
        }
        newPosition = newDest;
        std::vector<std::vector<char>> tempMap = *map;
        tempMap.at(static_cast<size_t>(position.x)).at(static_cast<size_t>(position.y)) = 'E';
        tempMap.at(static_cast<size_t>(newDest.x)).at(static_cast<size_t>(newDest.y)) = 'S';

        std::vector<glm::vec3> newPath = solveMaze(tempMap);
        for (auto& step: newPath){
          pathQueue.push_back(step);
        }
      }
      if (!pathQueue.empty()){
        targetPos = pathQueue.front();
        pathQueue.pop_front();
        moving=true;
      }
    } else {
      glm::vec3 diff = targetPos - currentPos;
      if (glm::length(diff) > 0.0001f) {
        glm::vec3 dir = glm::normalize(diff);
        currentPos += dir * moveSpeed * dt;
      }
      if (glm::distance(currentPos, targetPos) < moveSpeed * dt){
        currentPos = targetPos;
        moving = false;
        position = {std::round(currentPos.x), std::round(currentPos.y), 1.0f};
      }
    }
    position = {std::round(currentPos.x), std::round(currentPos.y), 1.0f};
    self.Position = {currentPos.x, currentPos.y, 0.0f};
  } else if (state == "chase"){
    float dt=app->dt;
    if (!moving) {
      if (pathQueue.empty()) {
        std::vector<std::vector<char>> tempMap=*map;
        tempMap.at(static_cast<size_t>(position.x)).at(static_cast<size_t>(position.y)) = 'E'; 
        tempMap.at(static_cast<size_t>(app->cameraPos.x)).at(static_cast<size_t>(app->cameraPos.y)) = 'S';
        std::vector<glm::vec3> newPath = playerDistance(tempMap);
        for (auto& step: newPath){
          pathQueue.push_back(step);
        }
      }
      if (!pathQueue.empty()){
        targetPos = pathQueue.front();
        pathQueue.pop_front();
        moving=true;
      }
    } else {
      glm::vec3 diff = targetPos - currentPos;
      if (glm::length(diff) > 0.0001f) {
        glm::vec3 dir = glm::normalize(diff);
        currentPos += dir * moveSpeed * dt;
      }
      if (glm::distance(currentPos, targetPos) < moveSpeed * dt){
        currentPos = targetPos;
        moving = false;
        position = {std::round(currentPos.x), std::round(currentPos.y), 1.0f};
      }
    }
    position = {std::round(currentPos.x), std::round(currentPos.y), 1.0f};
    self.Position = {currentPos.x, currentPos.y, 0.0f};
  }
}

/**
 * @brief evalautes the ais distance from the player to detrmine its stae form wander to chase anf inaly damge as it gets closer to the player.
 */
void enimy::stateTransition(){
  std::string oldState = state;

  std::vector<std::vector<char>> tempMap = *map;
  tempMap.at(static_cast<size_t>(position.x)).at(static_cast<size_t>(position.y)) = 'E';
  tempMap.at(static_cast<size_t>(app->cameraPos.x)).at(static_cast<size_t>(app->cameraPos.y)) = 'S';
  int pdis = (int)playerDistance(tempMap).size();

  if (pdis <= 5){
    state = "chase";
    if (pdis <= 1){
      hurt();
    }
  } else {
    state = "wander";
  }

  if (state != oldState){
    pathQueue.clear();
    moving = false;
  }

  move();
}



