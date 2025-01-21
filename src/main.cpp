/*

SDG                                                                                               JJ

                                               Orc Horde

									A Game Made (almost) from scratch
*/

#include <iostream>
#include <stdexcept>
#include <chrono>

#include "game_object.hh"

using namespace std::literals;

std::chrono::duration MIN_FRAME_TIME = 1ms;

struct GameState {
  AssetStore &assetStore;
  std::vector<RigidBody> gameObjects;
};

GameState initGameState(Renderer &renderer) {;
  AssetStore *assetStore = new AssetStore(renderer);
  auto position = skyVec3{0.0,0.0,0.0};
  auto rotation = skyVec3{0.0,0.0,0.0};
  auto scale = 1.0f;
  RigidBody house = RigidBody(position, rotation, scale, "", "viking_room1234", *assetStore);

  auto position2 = skyVec3{1.0,1.0,1.0};
  auto rotation2 = skyVec3{0.0,0.0,0.0};
  auto scale2 = 2.0f;

  RigidBody house2 = RigidBody(position, rotation, scale, "", "viking_room1234", *assetStore);

  GameState gameState { .assetStore = *assetStore,
						.gameObjects {house, house2},      };


  // TODO(caleb): This should probabaly be assigned to a job queue somewhere
  // when we get to that point
  while (!house.load()) {
	std::printf("Loading house...");
  }

  return gameState;

} 

void drawDemoFrame(Renderer &renderer, GameState gameState, int generation) {
  RenderState renderState = {};

  for (auto& obj : gameState.gameObjects) {
	//obj.update();
	obj.display(renderState);
  }

  auto ops = renderState.getRenderOps(renderer);
  renderer.drawFrame(ops);

  renderState.cleanup(renderer);
}


int main (int argc, char *argv[]) {
  Renderer renderer;

  try {
	renderer.initWindow();
    renderer.initGraphics();

	auto prev_frame = std::chrono::high_resolution_clock::now();
	GameState gameState = initGameState(renderer);

	int generation = 0;
    while (!renderer.shouldClose()) {
	  auto current_frame = std::chrono::high_resolution_clock::now();
	  if ((current_frame - prev_frame) > MIN_FRAME_TIME) {
		renderer.getInput();
		drawDemoFrame(renderer, gameState, generation++);
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
