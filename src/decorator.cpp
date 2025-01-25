/*

SDG                                                                                               JJ

                                     Orc Horde

							    Game Object: Decorator
*/

#include "game_object.hh"

Decorator::Decorator(skyVec3 position, float scale, skyGUID textureId, AssetStore &assetStore)
  : GameObject(position)
  , scale(scale)
{
  // TODO(caleb): we may need to handle the case later on that we may pass in
  // a guid that does not return that type
  texture = assetStore.getTexture(textureId);
  mesh = assetStore.getMesh(DECORATOR_GUID);
}

GameOps Decorator::update(std::chrono::microseconds dt, GameState &gameState){ return {}; }

void Decorator::display(RenderState &renderState){
  Instance thisInstance {
	.position = position,
	.rotation = static_cast<skyVec4>(skyQuat::unitVec()), // FIXME(caleb): orient towards the player
	.scale = scale,
	.textureIndex = texture->getLayerOffset()
  };

  mesh->display(renderState, thisInstance); // NOTE(caleb): this adds instance to renderstate
}

bool Decorator::load() {
  bool textureLoaded, meshLoaded;
  assert(texture != nullptr);
  assert(mesh != nullptr);

  textureLoaded = texture->load();
  meshLoaded = mesh->load();

  return (textureLoaded && meshLoaded);
}
