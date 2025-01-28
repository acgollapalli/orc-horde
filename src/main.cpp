/*

SDG                                                                                               JJ

                                             Orc Horde

									A Game Made (almost) from scratch
*/



#include "game_object.hh"

std::chrono::duration MIN_FRAME_TIME = 1ms;

GameState initGameState(Renderer &renderer) {;
  AssetStore *assetStore = new AssetStore(renderer);

  skyVec3 position (0.0,0.0,0.0);
  skyQuat rotation = skyQuat::unitVec();
  float scale = 1.0f;
  GameObject *map = new RigidBody(position, rotation, 11.4,
								MAP_TEXTURE_GUID, DECORATOR_GUID, *assetStore);

  GameObject *orc = new Orc(skyVec3(0.0, 4.0, 0.0), *assetStore);
  GameObject *human = new Human(skyVec3(0.0, -4.0, 0.0), *assetStore);

  // TODO(caleb): add a human to span here as well, once we have the human assets.
  
  GameState gameState { .assetStore = *assetStore,
						.gameObjects {orc, human, map},};

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
  for (int i = 0; i < ORCS_PER_FRAME && gameState.gameObjects.size() < MAX_GAME_OBJECTS/2; i++) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<float> distribution(-5.4, 5.4);

    float x_rand = distribution(generator);

	GameObject *orc = new Orc(skyVec3(x_rand, 4.4, 0.0), gameState.assetStore);

	gameState.gameObjects.push_back(orc);
  }
}

void spawnHumans(GameState &gameState) {
  for (int i = 0; i < HUMANS_PER_FRAME && gameState.gameObjects.size() < MAX_GAME_OBJECTS/2; i++) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<float> distribution(-5.4, 5.4);

    float x_rand = distribution(generator);

	GameObject *human = new Human(skyVec3(x_rand, -2.4, 0.0), gameState.assetStore);

	gameState.gameObjects.push_back(human);
  }
}

void drawDemoFrame(Renderer &renderer, GameState &gameState, std::chrono::duration<float> dt) {
  RenderState renderState = {};

  auto dt_micros = std::chrono::duration_cast<std::chrono::microseconds>(dt);

  //spawnOrcs(gameState);
  //spawnHumans(gameState);
  
  for (auto& obj : gameState.gameObjects) {
	obj->update(dt_micros, gameState);
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
