/*

SDG                                                                          JJ

                                     Orc Horde
*/

#include <iostream>
#include <stdexcept>

#include "renderer.hh"

int main (int argc, char *argv[]) {
  Renderer renderer;

  try {
	renderer.initWindow();
    renderer.initGraphics();
	//
    while (!renderer.shouldClose()) {
	  renderer.getInput();
	  renderer.drawFrame();
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  renderer.cleanup();

  return EXIT_SUCCESS;
}
