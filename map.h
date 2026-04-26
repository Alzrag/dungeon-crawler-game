#include <vector>
#include <stack>
#include <random>
#include "Engine.h"

std::vector<std::vector<char>> generate_map(int width, int height, unsigned int seed);

void print_map(const std::vector<std::vector<char>>& room);

std::vector<fixed> convertMap(const std::vector<std::vector<char>>& room, Engine& app);
