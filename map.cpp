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
    auto [r,c] = cellStack.top();
    vector<pair<int,int>> neighbors;
    for (auto [dr,dc]: dirs) {
      int nr = r + dr, nc = c + dc;
      if (nr > 0 && nr < rows-1 && nc > 0 && nc < cols-1 && room[nr][nc]=='#') neighbors.push_back({nr,nc});
    }
    if (!neighbors.empty()) {
      uniform_int_distribution<int> dist(0, neighbors.size()-1);
      auto [nr,nc] = neighbors[dist(gen)];
      room[(r+nr)/2][(c+nc)/2] = ' ';
      room[nr][nc] = ' ';
      cellStack.push({nr,nc});
    } else cellStack.pop();
  }

  room[1][1] = '!';

  while(true) {
    uniform_int_distribution<int> row_dist(0,(int)room.size()-1);
    int r=row_dist(gen);
    uniform_int_distribution<int> col_dist(0,(int)room[0].size()-1);
    int c=col_dist(gen);
    if(room[r][c]==' '){ room[r][c]='X'; break; }
  }

  for(int i=0;i<3;i++){
    while(true){
      uniform_int_distribution<int> row_dist(0,(int)room.size()-1);
      int r=row_dist(gen);
      uniform_int_distribution<int> col_dist(0,(int)room[0].size()-1);
      int c=col_dist(gen);
      if(room[r][c]==' '){ room[r][c]='?'; break; }
    }
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

std::vector<std::unique_ptr<fixed>> convertMap(const std::vector<std::vector<char>>& room, Engine& app){
  std::vector<std::unique_ptr<fixed>> map3d;
  map3d.reserve(1 + room.size() * room[0].size());

  float x = room.size();
  float y = room[0].size();

  map3d.push_back(std::make_unique<fixed>());
  map3d.back()->init("models/wall.obj", "textures/brick_Wall.png", app);
  map3d.back()->isWall = true;
  map3d.back()->Scale    = {x, y, 1.0f};
  map3d.back()->Position = {x/2, y/2, -1.0f};

  for (size_t i = 0; i < room.size(); i++){
    for (size_t j = 0; j < room[0].size(); j++){
      if (room[i][j] == '#'){
        map3d.push_back(std::make_unique<fixed>());
        map3d.back()->init("models/wall.obj", "textures/brick_Wall.png", app);
        map3d.back()->Position = {static_cast<float>(i), static_cast<float>(j), 0.0f};
      }
    }
  }
  return map3d;
}
