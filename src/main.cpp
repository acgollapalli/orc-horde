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
  skyQuat rotation = skyQuat::unitVec();
  float scale = 1.0f;
  RigidBody house = RigidBody(position, rotation, scale, "viking_room1234_tex", "viking_room1234", *assetStore);

  RigidBody orc = RigidBody(position, rotation, scale * 0.1, "orc_low_poly_tex", "orc_low_poly", *assetStore);

  RigidBody house2 = RigidBody(position, rotation, scale, "viking_room1234_tex", "viking_room1234", *assetStore);

  RigidBody map = RigidBody(position, rotation, 11.4, MAP_TEXTURE_GUID, DECORATOR_GUID, *assetStore);

  GameState gameState { .assetStore = *assetStore,
						.gameObjects {
						  house,
						  orc,
						  house2,
						  map,
						},};


  // TODO(caleb): This should probabaly be assigned to a job queue somewhere
  // when we get to that point
  bool loadingAssets= true;
  while (loadingAssets) {
	loadingAssets = false;
	for (auto& obj : gameState.gameObjects) {
	  if (!obj.load()) loadingAssets = true;
	}
  }

  return gameState;

} 

void drawDemoFrame(Renderer &renderer, GameState &gameState, std::chrono::duration<float> dt) {
  RenderState renderState = {};

  for (auto& obj : gameState.gameObjects) {
	//obj.update();
	obj.display(renderState);
	//gameState.gameObjects[0].move(std::chrono::duration_cast<std::chrono::microseconds>(dt),
	//							  //skyVec3(0.0, 0.000001, 0.0),
	//							  skyVec3(0.0, 0.0, 0.0),
	//							  skyVec3(0.0, 0.000001, 0.0 ));

	// orc
	gameState.gameObjects[0].move(std::chrono::duration_cast<std::chrono::microseconds>(dt),
								  skyVec3(0.0, 0.000001, 0.0),
								  skyVec3(0.0, 0.0, 0.000001));

	// other house
	gameState.gameObjects[1].move(std::chrono::duration_cast<std::chrono::microseconds>(dt),
								  skyVec3(0.0, 0.0, 0.0000001),
								  skyVec3(0.0, 0.0, 0.0));

	// map
	//gameState.gameObjects[3].move(std::chrono::duration_cast<std::chrono::microseconds>(dt),
	//							  skyVec3(0.0, 0.0, -0.0000001),
	//							  skyVec3(0.0, 0.0, 0.0));

  }

  auto ops = renderState.getRenderOps(renderer);
  renderer.drawFrame(ops);
}


int main (int argc, char *argv[]) {
  Renderer renderer;

  try {
	renderer.initWindow();
    renderer.initGraphics();

	GameState gameState = initGameState(renderer);

	auto prev_frame = std::chrono::high_resolution_clock::now();

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
