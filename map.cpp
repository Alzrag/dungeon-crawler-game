#include "map.h"
#include "fixed.h"
#include "Engine.h"

/**
 * @brief generates a 2-D maze grid using a random wlk and recusive back tracer effectivly a dpeth first search system then returns a grid of #'s' and then spaces for empy areas
 *
 * @param width number of cells along x axis(half of output)
 * @param height number of cells along y axis (half of ourput)
 * @param seed RNG seed for reproducable generation or random 
 * @return themaze as a vector vecotr of chars
 */
std::vector<std::vector<char>> generate_map(int width, int height, unsigned int seed) {
  using namespace std;
  size_t rows = static_cast<size_t>(height) * 2 + 1;
  /**
   * @brief 
   */
  size_t cols = static_cast<size_t>(width) * 2 + 1;
  vector<vector<char>> room(rows, vector<char>(cols, '#'));
  mt19937 gen(seed);

  size_t start_row = 1, start_col = 1;
  room[start_row][start_col] = ' ';
  stack<pair<int,int>> cellStack;
  cellStack.push({start_row, start_col});

  vector<pair<int,int>> dirs = {{-2,0},{2,0},{0,-2},{0,2}};
  while (!cellStack.empty()) {
    std::pair<int,int> rc = cellStack.top();
    int r = rc.first, c = rc.second;
    vector<pair<int,int>> neighbors;
    for (size_t d = 0; d < dirs.size(); d++) {
      int dr = dirs[d].first, dc = dirs[d].second;
      int nr = r + dr, nc = c + dc;
      if (nr > 0 && nr < (int)rows-1 && nc > 0 && nc < (int)cols-1 && room[static_cast<size_t>(nr)][static_cast<size_t>(nc)]=='#') neighbors.push_back({nr,nc});
    }
    if (!neighbors.empty()) {
      uniform_int_distribution<int> dist(0, static_cast<int>(neighbors.size())-1);
      std::pair<int,int> nrnc = neighbors[static_cast<size_t>(dist(gen))];
      int nr = nrnc.first, nc = nrnc.second;
      room[static_cast<size_t>((r+nr)/2)][static_cast<size_t>((c+nc)/2)] = ' ';
      room[static_cast<size_t>(nr)][static_cast<size_t>(nc)] = ' ';
      cellStack.push({nr,nc});
    } else cellStack.pop();
  }

  return room;
}

/**
 * @brief prints the mazxe as viewed from top down in ASCII
 *
 * @param room the maze to print
 */
void print_map(const std::vector<std::vector<char>>& room){
  std::cout << "=== Top-Down Map ===\n";
  for(const auto& row:room){
      for(char c:row) std::cout << c;
      std::cout << '\n';
  }
}

/**
 * @brief converts the 2d maze generated to a list of heap-aloacted fixed objects and creates a floor tile
 *
 * @param room the maze 
 * @param app the engine to render into 
 * @return the vector of pointers to the new heap-aloacted fixed objects.
 */
std::vector<fixed*> convertMap(const std::vector<std::vector<char>>& room, Engine& app){
  std::vector<fixed*> map3d;
  map3d.reserve(1 + room.size() * room[0].size());

  float x = static_cast<float>(room.size());
  float y = static_cast<float>(room[0].size());

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
