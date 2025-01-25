/*

SDG                                                                                               JJ

                                     Orc Horde

							      Game Object: Orc
*/

#include "game_object.hh"

static float SIGHT_RADIUS = 1.5;
static float KILL_RADIUS = 0.000001;

static float MOVEMENT_SPEED = 0.0000005;

Orc::Orc(skyVec3 position, AssetStore &assetStore)
  : GameObject(position)
  , rotation(skyQuat::unitVec())
  , scale(0.1f)
{
  // TODO(caleb): we may need to handle the case later on that we may pass in
  // a guid that does not return that type
  texture = assetStore.getTexture(ORC_TEXTURE_GUID);
  mesh = assetStore.getMesh(ORC_GUID);
  type = Orc_e;
}

GameOps Orc::update(std::chrono::microseconds dt, GameState &gameState){
  GameOps ops;

  skyVec3 direction(0.0f, 1.0f, 0.0f);

  GameObject *human;

  Location humanLocation = findNearestHuman(gameState, &human);
  float humanDistance = norm(position - humanLocation);

  if (humanDistance < KILL_RADIUS) {
	GameOp kill_op { .type=Kill_e, .operand=human};
	ops.push_back(kill_op);
  } else {
	if (humanDistance < SIGHT_RADIUS) {
	  direction = glm::normalize(humanLocation - position);
	}

	move(dt, direction * MOVEMENT_SPEED, skyVec3(0.0, 0.0, 0.0));
  }

  return ops;
}

void Orc::display(RenderState &renderState){ // TODO(caleb): account for death animation
  Instance thisInstance {
	.position = position,
	.rotation = static_cast<skyVec4>(rotation),
	.scale = scale,
	.textureIndex = texture->getLayerOffset()
  };

  mesh->display(renderState, thisInstance); // NOTE(caleb): this adds instance to renderstate
}

void Orc::move(std::chrono::microseconds dt, skyVec3 dv, skyVec3 dw) {
  std::printf("Moving orc from %f", position);

  float dt_micros= static_cast<float>(dt.count());
  position += dv * dt_micros;
  
  std::printf(" to %f \n", position);

  // assume dw is an angular velocity
  rotation += skyQuat{0.0f, dw.x, dw.y, dw.z} * rotation * (dt_micros / 2.0f);
  rotation.normalize();
}

bool Orc::load() {
  bool textureLoaded, meshLoaded;
  assert(texture != nullptr);
  assert(mesh != nullptr);

  textureLoaded = texture->load();
  meshLoaded = mesh->load();

  return (textureLoaded && meshLoaded);
}

Location Orc::findNearestHuman(GameState &gameState, GameObject **human) {
  float minDistance = std::numeric_limits<float>::max();
  Location minLocation = Location(0.0, -6.0, 0.0);

  for (auto obj : gameState.gameObjects) {
	if (obj->type == Human_e) {
	  Location orcLocation = obj->position;
	  float orcDistance = glm::length(position - orcLocation);
	  if (orcDistance < minDistance) {
		minDistance = orcDistance;
		minLocation = orcLocation;
		*human = obj;
	  }
	}
  }

  return minLocation;
}
