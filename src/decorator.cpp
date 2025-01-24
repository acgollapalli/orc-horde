/*

SDG                                                                                               JJ

                                     Orc Horde

							    Game Object: Decorator
*/

#include "game_object.hh"

Decorator::Decorator(skyVec3 position, skyQuat rotation, float scale,
					 skyGUID textureId, skyGUID meshId, AssetStore &assetStore)
  : GameObject(position)
  , rotation(rotation)
  , scale(scale)
{
  // TODO(caleb): we may need to handle the case later on that we may pass in
  // a guid that does not return that type
  texture = assetStore.getTexture(textureId);
  mesh = assetStore.getMesh(meshId);
}

void Decorator::update(){}
void Decorator::display(RenderState &renderState){
  Instance thisInstance {
	.position = position,
	.rotation = static_cast<skyVec4>(rotation),
	.scale = scale,
	.textureIndex = texture->getLayerOffset()
  };

  mesh->display(renderState, thisInstance); // NOTE(caleb): this adds instance to renderstate
}

void Decorator::move(std::chrono::microseconds dt, skyVec3 dv, skyVec3 dw) {
  float dt_micros= static_cast<float>(dt.count());
  position += dv * dt_micros;

  // assume dw is an angular velocity
  rotation += skyQuat{0.0f, dw.x, dw.y, dw.z} * rotation * (dt_micros / 2.0f);
  rotation.normalize();
}

bool Decorator::load() {
  bool textureLoaded, meshLoaded;
  assert(texture != nullptr);
  assert(mesh != nullptr);

  textureLoaded = texture->load();
  meshLoaded = mesh->load();

  return (textureLoaded && meshLoaded);
}
