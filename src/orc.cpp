/*

SDG                                                                                               JJ

                                     Orc Horde

							      Game Object: Orc
*/

#include "game_object.hh"

Orc::Orc(skyVec3 position, AssetStore &assetStore)
  : GameObject(position)
  , rotation(skyQuat::unitVec())
  , scale(0.1f)
{
  // TODO(caleb): we may need to handle the case later on that we may pass in
  // a guid that does not return that type
  texture = assetStore.getTexture(ORC_TEXTURE_GUID);
  mesh = assetStore.getMesh(ORC_GUID);
}

void Orc::update(std::chrono::microseconds dt){
  std::printf("Update called on orc\n");

  move(dt, skyVec3(0.0, -0.0000005, 0.0 ), skyVec3(0.0, 0.0, 0.0));
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
