/*

SDG                                                                                               JJ

                                             Orc Horde

									A Game Made (almost) from scratch
*/



#include "game_object.hh"

std::chrono::duration MIN_FRAME_TIME = 1ms;

void passGameOpsToMailboxes(std::vector<GameOp> ops, GameState &gameState) {
  for (auto &op : ops) {
	switch (op.type) {
	case Spawn_e:
	  gameState.mailbox.push_back(op);
	  break;
	case Kill_e:
	  op.operand->mailbox.push_back(op);
	  break;
	case DeleteSelf_e:
	  gameState.mailbox.push_back(op);
	  break;
	}
  }
}

void handleEntityGameOps(std::vector<GameOp> &mailbox, GameState &gameState) {
  for (auto &op : mailbox) {
	if (op.type == Kill_e) {
	  auto worldOps = op.operand->kill(gameState); // this is not a great way to handle this
	  for (auto worldOp : worldOps) {
		gameState.mailbox.push_back(worldOp);
	  }
	} else {
	  // nothing here yet
	}
  }

  mailbox.clear();
}

void handleWorldGameOps(GameState &gameState) {
  for (auto &op : gameState.mailbox) {
	switch (op.type) {
	case Spawn_e:
	  gameState.gameObjects.push_back(op.operand);
	  break;
	case DeleteSelf_e:
	  // This would be much simpler with an object pool
	  // and is an area that could be easily optimized
	  // but I am tired and the game jam is over in two days
	  // and I don't have have core game systems implemented yet
	  auto array_size = gameState.gameObjects.size();
	  for (int i = 0; i < array_size; i++) {
		if (gameState.gameObjects[i] == op.operand) {
		  gameState.gameObjects.erase(gameState.gameObjects.begin() + i);
		  delete(op.operand);
		  array_size = gameState.gameObjects.size();
		  break;
		}
	  }
	  break;
	}
  }
  gameState.mailbox.clear();
}

GameState initGameState(Renderer &renderer) {;
  AssetStore *assetStore = new AssetStore(renderer);

  assetStore->load(ALL_GAME_ASSETS);

  skyVec3 position (0.0,0.0,0.0);
  skyQuat rotation = skyQuat::unitVec();
  float scale = 1.0f;
  GameObject *map = new RigidBody(position, rotation, 11.4,
								MAP_TEXTURE_GUID, DECORATOR_GUID, *assetStore);

  GameState gameState { .assetStore = *assetStore,
						.gameObjects {map},};

  return gameState;
}

void spawnOrcs(GameState &gameState) {
  for (int i = 0; i < ORCS_PER_FRAME && gameState.gameObjects.size() < MAX_GAME_OBJECTS/8; i++) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<float> distribution(-5.4, 5.4);

    float x_rand = distribution(generator);

	GameObject *orc = new Orc(skyVec3(x_rand, 4.4, 0.0), gameState.assetStore);

	gameState.gameObjects.push_back(orc);
  }
}

void spawnHumans(GameState &gameState) {
  for (int i = 0; i < HUMANS_PER_FRAME && gameState.gameObjects.size() < MAX_GAME_OBJECTS/8; i++) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<float> distribution(-5.4, 5.4);
    std::uniform_int_distribution spawnDist(0, 100);

	if (spawnDist(generator) < 20) {
	  float x_rand = distribution(generator);

	  GameObject *human = new Human(skyVec3(x_rand, -2.4, 0.0), gameState.assetStore);
	  Human *human_ = static_cast<Human*>(human); // only for demo purposes
	  human_->blessed = true; // only for demo purposes

	  gameState.gameObjects.push_back(human);
	}
  }
}

void drawDemoFrame(Renderer &renderer, GameState &gameState, std::chrono::duration<float> dt) {
  RenderState renderState = {};

  auto dt_micros = std::chrono::duration_cast<std::chrono::microseconds>(dt);

  spawnOrcs(gameState);
  spawnHumans(gameState);
  
  for (GameObject *obj : gameState.gameObjects) {
	auto ops = obj->update(dt_micros, gameState);
	passGameOpsToMailboxes(ops, gameState);
	obj->generation++; // increment generation to odd number (will make sense with object pool)

  }

  for (GameObject *obj : gameState.gameObjects) {
	handleEntityGameOps(obj->mailbox, gameState);
	obj->generation++; // increment to even (wrapping is fine);
  }

  handleWorldGameOps(gameState);

  for (GameObject *obj : gameState.gameObjects) {
	obj->display(renderState);
	obj->generation++;
  }

  int numBullets = 0;
  for (GameObject *obj : gameState.gameObjects) {
	if (obj->type == Bullet_e) { numBullets++; }
  }

  auto ops = renderState.getRenderOps(renderer);
  renderer.drawFrame(ops);
}

static void handleCursorMovement(Window window, double xpos, double ypos) {
  std::printf("Moving to %d, %d", xpos, ypos);
}

static void handleMouseButton(Window window, int button, int action, int mods) {
  std::printf("Mouse Button Pressed, %d, %d, %d", button, action, mods);
}


int main (int argc, char *argv[]) {
  Renderer renderer;

  try {

	renderer.initWindow();
    renderer.initGraphics();

	// TODO(caleb): createCursor here once we load the cursor sprite
	renderer.setCursorMovementCallback(glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR), (CursorPositionCallback)handleCursorMovement);
	renderer.setMouseButtonCallback((MouseButtonCallback)handleMouseButton);

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
