/*

SDG                                                                                               JJ

                                     Orc Horde

							      Game Object: Bullet
*/

#include <limits>;

#include "game_object.hh"

static float MOVEMENT_RATE_REGULAR = 0.00001; // * dt
static float MOVEMENT_RATE_SUPER = 0.000005; // * dt

static float HIT_RADIUS_REGULAR = 0.01;
static float HIT_RADIUS_SUPER = 0.05;

static float KILL_RADIUS_REGULAR = HIT_RADIUS_REGULAR * 2;
static float KILL_RADIUS_SUPER = HIT_RADIUS_SUPER * 3;

static std::chrono::duration EXPLOSION_DURATION = 500ms; 


Bullet::Bullet(skyVec3 position, skyVec3 direction, bool superBullet, AssetStore &assetStore)
  : GameObject(position)
  , direction(direction)
  , superBullet(superBullet)
  , rotation(skyQuat::unitVec())
  , scale(0.1f)
{
  glm::normalize(direction);

  skyGUID bullet_texture_guid = superBullet ? BULLET_TEXTURE_GUID_SUPER : BULLET_TEXTURE_GUID_REGULAR;
  skyGUID bullet_mesh_guid = superBullet ? BULLET_MESH_GUID_SUPER : BULLET_MESH_GUID_REGULAR;

  // TODO(caleb): we may need to handle the case later on that we may pass in
  // a guid that does not return that type
  texture = assetStore.getTexture(bullet_texture_guid);
  mesh = assetStore.getMesh(bullet_mesh_guid);
  type = Bullet_e;
}

std::vector<GameOp> Bullet::update(std::chrono::microseconds dt, GameState &gameState){
  float movement_rate = superBullet ? MOVEMENT_RATE_SUPER : MOVEMENT_RATE_REGULAR;
  float hit_radius = superBullet ? HIT_RADIUS_SUPER : HIT_RADIUS_REGULAR;
  float kill_radius = superBullet ? KILL_RADIUS_SUPER : KILL_RADIUS_REGULAR;

  skyGUID explosion_meshId= superBullet ? EXPLOSION_GUID : SUPER_EXPLOSION_GUID;
  skyGUID explosion_texId= superBullet ? EXPLOSION_TEXTURE_GUID : SUPER_EXPLOSION_TEXTURE_GUID;

  bool outOfBounds = position.y > WORLD_TOP_COORD ||
	                 position.y < -WORLD_TOP_COORD ||
	      			 position.x > WORLD_RIGHT_COORD ||
                     position.x < -WORLD_RIGHT_COORD;

  if (outOfBounds) return {{ DeleteSelf_e, this }};

  std::vector<GameOp> ops;
  float dt_micros= static_cast<float>(dt.count());

  position += direction * movement_rate * dt_micros;

  auto enemiesHit = scanEnemies(hit_radius, kill_radius, gameState);

  if (enemiesHit.size() > 0) {
	for (auto enemy : enemiesHit) {
	  ops.push_back(GameOp{ .type = Kill_e, .operand = enemy });
	}
	ops.push_back(GameOp{ .type = DeleteSelf_e, .operand = this });

	auto explosion = new Animation(position, explosion_texId, explosion_meshId,
								   EXPLOSION_DURATION, gameState.assetStore);
	ops.push_back(GameOp{ Spawn_e, explosion });
  }
  return ops;
}

void Bullet::display(RenderState &renderState){ // TODO(caleb): account for death animation
  Instance thisInstance {
	.position = position,
	.rotation = static_cast<skyVec4>(rotation),
	.scale = scale,
	.textureIndex = texture->getLayerOffset()
  };

  mesh->display(renderState, thisInstance); // NOTE(caleb): this adds instance to renderstate
}

void Bullet::move(std::chrono::microseconds dt, skyVec3 dv, skyVec3 dw) {
  std::printf("Moving orc from %f", position);

  float dt_micros= static_cast<float>(dt.count());
  position += dv * dt_micros;
  
  std::printf(" to %f \n", position);

  // assume dw is an angular velocity
  rotation += skyQuat{0.0f, dw.x, dw.y, dw.z} * rotation * (dt_micros / 2.0f);
  rotation.normalize();
}

bool Bullet::load() {
  bool textureLoaded, meshLoaded;
  assert(texture != nullptr);
  assert(mesh != nullptr);

  textureLoaded = texture->load();
  meshLoaded = mesh->load();

  return (textureLoaded && meshLoaded);
}

std::vector<GameObject *> Bullet::scanEnemies(float hitRadius, float killRadius, GameState &gameState) {
  std::vector<GameObject *> out;
  bool hit = false;
  for (auto obj : gameState.gameObjects) {
	if (obj->type == Orc_e) {
	  Location orcLocation = obj->position;
	  float orcDistance = glm::length(position - orcLocation);

	  if (orcDistance < killRadius) out.push_back(obj);
	  if (orcDistance < hitRadius) hit = true;
	}
  }

  if (!hit) out.clear();

  return out;
}	

