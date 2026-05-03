#include "enimy.h"
#include <glm/geometric.hpp>
#include <random>
#include "map.h"

std::random_device rd;
std::mt19937 mt(rd());

enimy::enimy(std::vector<std::vector<char>>* mapIn, const fixed& object, Engine* game){
  app = game;
  level=1;
  health = (((level-1)*10)+20);
  damage = (((level-1)*10)+20);
  map = mapIn;
  self = object;
  std::uniform_int_distribution<int> posRow(0, (int)map->size()-1);
  std::uniform_int_distribution<int> posCol(0, (int)(*map)[0].size()-1);
  newPosition = {posRow(mt), posCol(mt), 1.0f};
  while (mapIn->at((int)newPosition.x).at((int)newPosition.y)!=' '){
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

void enimy::takeDamage(int amount){
  health+=amount;
}

void enimy::hurt(){
  app->playerHealth+=damage; 
}

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

float distance(float x1,float x2,float y1, float y2){
  return abs(x1-x2)+abs(y1-y2);
}

bool isValid(int x,int y, const std::vector<std::vector<char>>& Maze){
  if(y<0||y>=Maze.size()||x<0||x>=Maze[0].size()) return false;
  if(Maze[y][x]=='#') return false;
  return true;
}

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

std::vector<glm::vec3> solveMaze(std::vector<std::vector<char>>& Maze){ 
  int rows = Maze.size();
  int cols=Maze[0].size();
  node* start=nullptr;
  node* end(nullptr);
  std::vector<std::vector<node>> grid(rows, std::vector<node>(cols));

  //build grid
  for (int y = 0; y<rows;y++){
    for (int x = 0;x<cols;x++){
      grid[y][x].y=y;
      grid[y][x].x=x;
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
    for (int i=1;i<(int)open.size();i++){
      if(open[i]->f<open[bestOp]->f){
        bestOp=i;
      }
    }
    node* current = open[bestOp];

    if(current->x==end->x&&current->y==end->y){ 
      std::vector<glm::vec3> path;
      node* temp = current->parrent;
      while(temp!=nullptr&&Maze[temp->y][temp->x]!='E'){
        path.push_back({(float)temp->y, (float)temp->x, 1.0f});
        temp=temp->parrent;
      }
      if (!path.empty()) reverse(path.begin(), path.end());
      return path;
    }

    open.erase(open.begin()+bestOp);
    current->closed=true;
    if(Maze[current->y][current->x]!='E'&&Maze[current->y][current->x]!='S')Maze[current->y][current->x]='V';

    int dx[]={0,0,-1,1};
    int dy[]={-1,1,0,0};

    for (int i=0;i<4;i++){
      int nx=current->x+dx[i];
      int ny=current->y+dy[i];

      if (isValid(nx, ny, Maze)){
        node* neighbor =&grid[ny][nx]; 
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
            if(Maze[neighbor->y][neighbor->x]!='E'&&Maze[neighbor->y][neighbor->x]!='S')Maze[neighbor->y][neighbor->x]='N';
          }
        }
      }
    }
  }
  return {};
}

std::vector<glm::vec3> playerDistance(std::vector<std::vector<char>>& Maze){
  int rows = Maze.size();
  int cols=Maze[0].size();
  node* start=nullptr;
  node* end(nullptr);
  std::vector<std::vector<node>> grid(rows, std::vector<node>(cols));

  //build grid
  for (int y = 0; y<rows;y++){
    for (int x = 0;x<cols;x++){
      grid[y][x].y=y;
      grid[y][x].x=x;
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
    for (int i=1;i<(int)open.size();i++){
      if(open[i]->f<open[bestOp]->f){
        bestOp=i;
      }
    }
    node* current = open[bestOp];

    if(current->x==end->x&&current->y==end->y){ 
      std::vector<glm::vec3> path;
      node* temp = current->parrent;
      while(temp!=nullptr&&Maze[temp->y][temp->x]!='E'){
        path.push_back({(float)temp->y, (float)temp->x, 1.0f});
        temp=temp->parrent;
      }
      if (!path.empty()) reverse(path.begin(), path.end());
      return path;
    }

    open.erase(open.begin()+bestOp);
    current->closed=true;
    if(Maze[current->y][current->x]!='E'&&Maze[current->y][current->x]!='S')Maze[current->y][current->x]='V';

    int dx[]={0,0,-1,1};
    int dy[]={-1,1,0,0};

    for (int i=0;i<4;i++){
      int nx=current->x+dx[i];
      int ny=current->y+dy[i];

      if (isValid(nx, ny, Maze)){
        node* neighbor =&grid[ny][nx]; 
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
            if(Maze[neighbor->y][neighbor->x]!='E'&&Maze[neighbor->y][neighbor->x]!='S')Maze[neighbor->y][neighbor->x]='N';
          }
        }
      }
    }
  }
  return {};
}

void enimy::move(){
  if (state == "wander"){
    float dt=app->dt;
    if (!moving) {
      if (pathQueue.empty()) {
        glm::vec3 target = newPosition;
        std::uniform_int_distribution<int> posRow(1, (int)map->size()-1);
        std::uniform_int_distribution<int> posCol(1, (int)(*map)[0].size()-1);
        glm::vec3 newDest = {(float)posRow(mt), (float)posCol(mt), 1.0f};
        while (map->at((int)newDest.x).at((int)newDest.y) != ' '){
          newDest = {(float)posRow(mt), (float)posCol(mt), 1.0f};
        }
        newPosition = newDest;
        std::vector<std::vector<char>> tempMap = *map;
        tempMap.at((int)position.x).at((int)position.y) = 'E';
        tempMap.at((int)newDest.x).at((int)newDest.y) = 'S';

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
        tempMap.at((int)position.x).at((int)position.y) = 'E'; 
        tempMap.at(static_cast<int>(app->cameraPos.x)).at(static_cast<int>(app->cameraPos.y)) = 'S';
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

void enimy::stateTransition(){
  std::vector<std::vector<char>> tempMap = *map;
  //print_map(tempMap);
  tempMap.at((int)position.x).at((int)position.y) = 'E';
  tempMap.at(static_cast<int>(app->cameraPos.x)).at(static_cast<int>(app->cameraPos.y)) = 'S';
  int pdis = (int)playerDistance(tempMap).size();
  std::cout<<"pdis is: "<<pdis<<std::endl;
  if (pdis<=5){
    state="chase";
    std::cout<<"chaseing"<<std::endl;
    if (pdis <=1){
      hurt();
    }
  } else {
    std::cout<<"wandering"<<std::endl;
    state="wander";
  }
  move();
}



