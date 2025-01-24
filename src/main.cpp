/*

SDG                                                                                               JJ

                                               Orc Horde

									A Game Made (almost) from scratch
*/

#include <iostream>
#include <stdexcept>
#include <chrono>
#include <random>

#include "game_object.hh"

#ifndef PI
#define PI 3.14159
#endif

using namespace std::literals;

std::chrono::duration MIN_FRAME_TIME = 1ms;

int ORCS_PER_FRAME = 1; // FIXME(caleb): need this to be per ms

struct GameState {
  AssetStore &assetStore;
  std::vector<GameObject*> gameObjects;
};

GameState initGameState(Renderer &renderer) {;
  AssetStore *assetStore = new AssetStore(renderer);

  skyVec3 position (0.0,0.0,0.0);
  skyQuat rotation = skyQuat::unitVec();
  float scale = 1.0f;
  GameObject *map = new RigidBody(position, rotation, 11.4,
								MAP_TEXTURE_GUID, DECORATOR_GUID, *assetStore);

  skyVec3 orcPosition(0.0, 5.0, 0.0); // spawn one orce offscreen to force asset loading
  GameObject *orc = new Orc(skyVec3(0.0, 4.0, 0.0), *assetStore);

  // TODO(caleb): add a human to span here as well, once we have the human assets.
  
  GameState gameState { .assetStore = *assetStore,
						.gameObjects {orc, map},};


  // TODO(caleb): This should probably be assigned to a job queue when we get to that point
  bool loadingAssets= true;
  while (loadingAssets) { // currently, this loop _blocks_ the main thread
	loadingAssets = false;
	for (auto& obj : gameState.gameObjects) {
	  if (!obj->load()) loadingAssets = true; // currently each load _also_ blocks the main thread
	}
  }

  return gameState;
}

void spawnOrcs(GameState &gameState) {
  for (int i = 0; i < ORCS_PER_FRAME && gameState.gameObjects.size() < MAX_GAME_OBJECTS; i++) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<float> distribution(-5.4, 5.4);

    float x_rand = distribution(generator);

	GameObject *orc = new Orc(skyVec3(x_rand, 4.0, 0.0), gameState.assetStore);

	gameState.gameObjects.push_back(orc);
  }
}


void drawDemoFrame(Renderer &renderer, GameState &gameState, std::chrono::duration<float> dt) {
  RenderState renderState = {};

  auto dt_micros = std::chrono::duration_cast<std::chrono::microseconds>(dt);

  spawnOrcs(gameState);
  
  for (auto& obj : gameState.gameObjects) {
	obj->update(dt_micros);
	obj->display(renderState);
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
