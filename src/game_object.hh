/*

SDG                                                                                               JJ

                                     Orc Horde

									 Game Object
*/

#include <glm/glm.hpp> // TODO(caleb): get rid of GLM

#include "asset.hh"

typedef glm::vec2 skyVec2; //  FIXME(caleb): you're a trained mathematician. WRITE YOUR OWN MATH
typedef glm::vec3 skyVec3; // TODO(caleb): seriously, write your own math library

typedef skyVec3 Location;
typedef skyVec2 DecoratorSize;

class GameObject {
public:
  GameObject(skyVec3 position) :position(position){};
  //~GameObject();

  virtual void update() = 0;
  virtual RenderOp display() = 0;
  virtual void move() = 0;
private:
  skyVec3 position;
};

class RigidBody : public GameObject {
public:
  RigidBody(skyVec3 position, skyVec3 rotation, float scale,
			GUID textureId, GUID meshId, AssetStore &assetStore);
  //~RigidBody();
  void update();
  RenderOp display();
  void move();
  bool load();
private:
  // TODO(caleb): add private copy constructor
  skyVec3 rotation;
  float scale;
  Mesh *mesh;
  Texture *texture;
};

class Decorator : public GameObject {
public:
  Decorator(skyVec3 position, float scale, Texture texture);
  ~Decorator();
private:
  // TODO(caleb): add private copy constructor
  float scale;
  Mesh *mesh;
  Texture *texture;
};

/* ======================= Rigid Body Implementation ==================== */
RigidBody::RigidBody(skyVec3 position, skyVec3 rotation, float scale,
					 GUID textureId, GUID meshId, AssetStore &assetStore)
  : GameObject(position)
  , rotation(rotation)
  , scale(scale)
{
  // TODO(caleb): we may need to handle the case later on that we may pass in
  // a guid that does not return that type
  texture = assetStore.get_texture(textureId);
  mesh = assetStore.get_mesh(meshId);
}

void RigidBody::update(){}
RenderOp RigidBody::display(){
  return mesh->display();
}
void RigidBody::move(){}
bool RigidBody::load() {
  bool textureLoaded, meshLoaded = true;
  if (texture != nullptr) {
	textureLoaded = texture->load();
  }
  if (mesh != nullptr) {
	meshLoaded = mesh->load();
  }
  return (textureLoaded && meshLoaded);
}


  
