/*

SDG                                                                                               JJ

                                     Orc Horde

									 Animation
*/

#include "game_object.hh"

Animation::Animation(skyVec3 position, skyGUID textureId, skyGUID meshId,
					 std::chrono::milliseconds duration, AssetStore &assetStore)
  : GameObject(position)
  , timeLeft(0)
  , rotation(skyQuat::unitVec())
  , scale(1.0f)
{
  // TODO(caleb): we may need to handle the case later on that we may pass in
  // a guid that does not return that type
  timeLeft += duration;
  texture = assetStore.getTexture(textureId);
  mesh = assetStore.getMesh(meshId);
  type = Animation_e;
}

GameOps Animation::update(std::chrono::microseconds dt, GameState &gameState){
  timeLeft = std::chrono::duration_cast<std::chrono::milliseconds>(timeLeft - dt);
  if (timeLeft > std::chrono::milliseconds::zero()) {
	return {};
  } else {
	GameOp deleteSelf { .type = DeleteSelf_e, .operand = this };
	GameOps ops;
	ops.push_back(deleteSelf);
	return ops;
  }
}

void Animation::display(RenderState &renderState){
  Instance thisInstance {
	.position = position,
	.rotation = static_cast<skyVec4>(rotation),
	.scale = scale,
	.textureIndex = texture->getLayerOffset()
  };

  mesh->display(renderState, thisInstance); // NOTE(caleb): this adds instance to renderstate
}

void Animation::move(std::chrono::microseconds dt, skyVec3 dv, skyVec3 dw) {
  float dt_micros= static_cast<float>(dt.count());
  position += dv * dt_micros;

  // assume dw is an angular velocity
  rotation += skyQuat{0.0f, dw.x, dw.y, dw.z} * rotation * (dt_micros / 2.0f);
  rotation.normalize();
}

bool Animation::load() {
  bool textureLoaded, meshLoaded;
  assert(texture != nullptr);
  assert(mesh != nullptr);

  textureLoaded = texture->load();
  meshLoaded = mesh->load();

  return (textureLoaded && meshLoaded);
}
