#include "map.h"
#include "fixed.h"
#include "Engine.h"

std::vector<std::vector<char>> generate_map(int width, int height, unsigned int seed) {
  using namespace std;
  int rows = height * 2 + 1;
  int cols = width * 2 + 1;
  vector<vector<char>> room(rows, vector<char>(cols, '#'));
  mt19937 gen(seed);

  int start_row = 1, start_col = 1;
  room[start_row][start_col] = ' ';
  stack<pair<int,int>> cellStack;
  cellStack.push({start_row, start_col});

  vector<pair<int,int>> dirs = {{-2,0},{2,0},{0,-2},{0,2}};
  while (!cellStack.empty()) {
    std::pair<int,int> rc = cellStack.top();
    int r = rc.first, c = rc.second;
    vector<pair<int,int>> neighbors;
    for (int d = 0; d < (int)dirs.size(); d++) {
      int dr = dirs[d].first, dc = dirs[d].second;
      int nr = r + dr, nc = c + dc;
      if (nr > 0 && nr < rows-1 && nc > 0 && nc < cols-1 && room[nr][nc]=='#') neighbors.push_back({nr,nc});
    }
    if (!neighbors.empty()) {
      uniform_int_distribution<int> dist(0, neighbors.size()-1);
      std::pair<int,int> nrnc = neighbors[dist(gen)];
      int nr = nrnc.first, nc = nrnc.second;
      room[(r+nr)/2][(c+nc)/2] = ' ';
      room[nr][nc] = ' ';
      cellStack.push({nr,nc});
    } else cellStack.pop();
  }

  return room;
}

void print_map(const std::vector<std::vector<char>>& room){
  std::cout << "=== Top-Down Map ===\n";
  for(const auto& row:room){
      for(char c:row) std::cout << c;
      std::cout << '\n';
  }
}

std::vector<fixed*> convertMap(const std::vector<std::vector<char>>& room, Engine& app){
  std::vector<fixed*> map3d;
  map3d.reserve(1 + room.size() * room[0].size());

  float x = room.size();
  float y = room[0].size();

  fixed* floor = new fixed();
  floor->init("models/wall.obj", "textures/brick_Wall.png", app);
  floor->isWall = true;
  floor->hasCollider = false;
  floor->Scale    = {x, y, 1.0f};
  floor->Position = {x/2, y/2, -1.0f};
  map3d.push_back(floor);

  for (size_t i = 0; i < room.size(); i++){
    for (size_t j = 0; j < room[0].size(); j++){
      if (room[i][j] == '#'){
        fixed* wall = new fixed();
        wall->init("models/wall.obj", "textures/brick_Wall.png", app);
        wall->Position = {static_cast<float>(i), static_cast<float>(j), 0.0f};
        wall->isWall = true;
        map3d.push_back(wall);
      }
    }
  }
  return map3d;
}
