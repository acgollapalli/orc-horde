/*

SDG                                                                                               JJ

                                               Orc Horde

									A Game Made (almost) from scratch
*/

#include <iostream>
#include <stdexcept>
#include <chrono>

#include "game_object.hh"

#ifndef PI
#define PI 3.14159
#endif

using namespace std::literals;

std::chrono::duration MIN_FRAME_TIME = 1ms;

struct GameState {
  AssetStore &assetStore;
  std::vector<RigidBody> gameObjects;
};

GameState initGameState(Renderer &renderer) {;
  AssetStore *assetStore = new AssetStore(renderer);
  skyVec3 position (0.0,0.0,0.0);
  skyQuat rotation = skyQuat::fromAngle(skyVec3(0.0, 1.0, 0.0), PI);
  float scale = 1.0f;
  RigidBody house = RigidBody(position, rotation, scale, "", "viking_room1234", *assetStore);

  skyVec3 position2 (0.5,0.5,0.5);
  skyQuat rotation2 = skyQuat::fromAngle(skyVec3(0.0, 1.0, 0.0), PI);
  float scale2 = 1.0f;

  RigidBody house2 = RigidBody(position, rotation, scale, "", "viking_room1234", *assetStore);

  GameState gameState { .assetStore = *assetStore,
						.gameObjects {house,
									  house2},      };


  // TODO(caleb): This should probabaly be assigned to a job queue somewhere
  // when we get to that point
  while (!house.load()) {
	std::printf("Loading house...");
  }

  return gameState;

} 

void drawDemoFrame(Renderer &renderer, GameState &gameState, std::chrono::duration<float> dt) {
  RenderState renderState = {};

  for (auto& obj : gameState.gameObjects) {
	//obj.update();
	obj.display(renderState);
	gameState.gameObjects[0].move(std::chrono::duration_cast<std::chrono::microseconds>(dt),
								  //skyVec3(0.0, 0.000001, 0.0),
								  skyVec3(0.0, 0.0, 0.0),
								  skyVec3(0.0, 0.000001, 0.0 ));

	gameState.gameObjects[1].move(std::chrono::duration_cast<std::chrono::microseconds>(dt),
								  skyVec3(0.0, 0.000001, 0.0),
								  skyVec3(0.0, 0.0, 0.000001));
  }

  auto ops = renderState.getRenderOps(renderer);
  renderer.drawFrame(ops);
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
		drawDemoFrame(renderer, gameState, current_frame - prev_frame);
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
