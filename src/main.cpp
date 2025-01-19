/*

SDG                                                                          JJ

                                     Orc Horde
*/

#include <iostream>
#include <stdexcept>
#include <chrono>

#include "renderer.hh"
#include "game_object.hh"

using namespace std::literals;

std::chrono::duration MIN_FRAME_TIME = 1ms;

void drawDemoFrame(Renderer &renderer) {
  AssetStore *assetStore = new AssetStore(renderer);

}


int main (int argc, char *argv[]) {
  Renderer renderer;

  try {
	renderer.initWindow();
    renderer.initGraphics();

	auto prev_frame = std::chrono::high_resolution_clock::now();

    while (!renderer.shouldClose()) {
	  auto current_frame = std::chrono::high_resolution_clock::now();
	  if ((current_frame - prev_frame) > MIN_FRAME_TIME) {
		renderer.getInput();
		// TODO(caleb): replace with drawFrame with renderOps
		//renderer.drawFrame(); 
		prev_frame = current_frame;
	  }
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  renderer.cleanup();

  return EXIT_SUCCESS;
}
