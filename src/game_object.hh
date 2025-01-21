/*

SDG                                                                                               JJ

                                     Orc Horde

									 Game Object
*/

#pragma once

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

  virtual void 			update() = 0;
  virtual void 			display(RenderState &renderState) = 0;
  virtual void 			move() = 0;
protected:
  skyVec3 position;
};

class RigidBody : public GameObject {
public:
  RigidBody(skyVec3 position, skyVec3 rotation, float scale,
			GUID textureId, GUID meshId, AssetStore &assetStore);
  //~RigidBody();
  void 					update();
  void 					display(RenderState &renderState);
  void 					move();
  bool 					load();
private:
  // TODO(caleb): add private copy constructor
  skyVec3 				rotation;
  float 				scale;
  Mesh *				mesh;
  Texture *				texture;
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

