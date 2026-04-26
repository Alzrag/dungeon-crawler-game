#include "Engine.h"
#include "fixed.h"
#include "map.h"
#include <memory>
#include <map>

void update(Engine& app){
  if(!app.sceneObjects.empty()){
    app.sceneObjects[0]->pitch+=0.5f;
  }
}


int main() {
  Engine app;  

  try {
    app.init();
    app.setUpdateCallback(update);

    std::vector<std::vector<char>> mapTxt = generate_map(10, 10, 12);
    std::vector<fixed> mapraw = convertMap(mapTxt, app);
    std::vector<std::unique_ptr<fixed>> map;
    map.reserve(mapraw.size());
    for(fixed block : mapraw){
      map.push_back(std::make_unique<fixed>(std::move(block)));
      app.add(map.back().get());
    }

    app.loop();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }


  return EXIT_SUCCESS;
}
