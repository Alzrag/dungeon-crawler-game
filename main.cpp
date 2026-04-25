#include "Engine.h"
#include "fixed.h"

int main() {
  Engine app; 

  try {
    app.init();

    fixed room2;
    room2.init("models/viking_room.obj", "textures/viking_room.png", app);
    room2.Position={2.0f,0.0f,0.0f};
    app.add(&room2);

    fixed room;
    room.init("models/model.obj", "textures/small_tower_albedo.png", app);
    room.pitch=90;
    room.Scale={0.0001f, 0.0001f,0.0001f};
    app.add(&room);



    app.loop();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }


  return EXIT_SUCCESS;
}
