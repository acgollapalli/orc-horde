/*

SDG                                                                                               JJ

                                     Orc Horde

							      Game Object: Human
*/

#include <limits>;

#include "game_object.hh"

static int FIRE_RATE_REGULAR = 60; // per second
static int FIRE_RATE_BLESSED = 120; // per second

static float MOVEMENT_RATE_REGULAR = 0.000003; // * dt
static float MOVEMENT_RATE_BLESSED = 0.000005; // * dt

static float KILL_RADIUS = 0.000001;

static std::chrono::duration DEATH_ANIMATION_DURATION = 500ms;

Human::Human(skyVec3 position, AssetStore &assetStore)
  : GameObject(position)
  , rotation(skyQuat::unitVec())
  , scale(0.1f)
  , usSinceLastFired(1000 / 60)
  , blessed(false)
	
{
  // TODO(caleb): we may need to handle the case later on that we may pass in
  // a guid that does not return that type
  texture = assetStore.getTexture(HUMAN_TEXTURE_GUID);
  mesh = assetStore.getMesh(HUMAN_GUID);
  type = Human_e;
}

GameOps Human::update(std::chrono::microseconds dt, GameState &gameState){
  GameOps ops;

  int fire_rate = (blessed) ? FIRE_RATE_BLESSED : FIRE_RATE_REGULAR;
  float movement_rate = (blessed) ? MOVEMENT_RATE_BLESSED : MOVEMENT_RATE_REGULAR;

  int fire_ms = 1000 / fire_rate;

  if ((usSinceLastFired += dt.count()) > fire_ms) {
	Location orcLocation = findNearestOrc(gameState);
	if (orcLocation.y < WORLD_TOP_COORD) {
	  std::printf("Shooting orc at %zf, %zf, %zf\n", orcLocation.x, orcLocation.y, orcLocation.z);
	  auto fire_op = fireAtOrc(orcLocation, gameState);
	  ops.push_back(fire_op);
	  usSinceLastFired = 0;
	}
  } else {
	std::printf("need to wait %d ms to fire\n", fire_ms - usSinceLastFired);
  }
  return ops;
}

void Human::display(RenderState &renderState){ // TODO(caleb): account for death animation
  Instance thisInstance {
	.position = position,
	.rotation = static_cast<skyVec4>(rotation),
	.scale = scale,
	.textureIndex = texture->getLayerOffset()
  };

  mesh->display(renderState, thisInstance); // NOTE(caleb): this adds instance to renderstate
}

void Human::move(std::chrono::microseconds dt, skyVec3 dv, skyVec3 dw) {
  std::printf("Moving orc from %f", position);

  float dt_micros= static_cast<float>(dt.count());
  position += dv * dt_micros;
  
  std::printf(" to %f \n", position);

  // assume dw is an angular velocity
  rotation += skyQuat{0.0f, dw.x, dw.y, dw.z} * rotation * (dt_micros / 2.0f);
  rotation.normalize();
}

bool Human::load() {
  bool textureLoaded, meshLoaded;
  assert(texture != nullptr);
  assert(mesh != nullptr);

  textureLoaded = texture->load();
  meshLoaded = mesh->load();

  return (textureLoaded && meshLoaded);
}

Location Human::findNearestOrc(GameState &gameState) {
  float minDistance = std::numeric_limits<float>::max();
  Location minLocation = Location(0.0, 6.0, 0.0);


  for (auto obj : gameState.gameObjects) {
	if (obj->type == Orc_e) {
	  Location orcLocation = obj->position;
	  float orcDistance = glm::length(position - orcLocation);
	  if (orcDistance < minDistance) {
		minDistance = orcDistance;
		minLocation = orcLocation;
	  }
	}
  }

  return minLocation;
}


GameOp Human::fireAtOrc(Location location, GameState &gameState) {
  skyVec3 velocity = glm::normalize(location - position);
  GameObject *bullet = new Bullet(position, velocity, blessed, gameState.assetStore);
  return GameOp { .type = Spawn_e, .operand = bullet };
}

GameOps Human::kill(GameState &gameState) {
  auto death_animation = new Animation(position, HUMAN_DEAD_GUID, HUMAN_DEAD_TEXTURE_GUID,
									   DEATH_ANIMATION_DURATION, gameState.assetStore);

  return { { Spawn_e, death_animation },
		   { DeleteSelf_e, this } };
}

