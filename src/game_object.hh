/*

SDG                                                                                               JJ

                                     Orc Horde

									 Game Object
*/

#pragma once

#include "math.hh"
#include <chrono>

#include "asset.hh"

typedef skyVec3 Location;
typedef skyVec2 DecoratorSize;

class GameObject {
public:
  GameObject(skyVec3 position) :position(position){};
  //~GameObject();

  virtual void 			update(std::chrono::microseconds dt) = 0;
  virtual void 			display(RenderState &renderState) = 0;
  virtual bool			load() = 0;
protected:
  skyVec3 position;
};

class RigidBody : public GameObject {
public:
  RigidBody(skyVec3 position, skyQuat rotation, float scale,
			GUID textureId, GUID meshId, AssetStore &assetStore);
  //~RigidBody();
  void 					update(std::chrono::microseconds dt);
  void 					display(RenderState &renderState);
  void                  move(std::chrono::microseconds dt, skyVec3 dv, skyVec3 dw);
  bool 					load();
private:
  // TODO(caleb): add private copy constructor
  skyQuat               rotation;
  float 				scale;
  Mesh *				mesh;
  Texture *				texture;
};

// FIXME(caleb): these are supposed to be always rotated towards the player,
class Decorator : public GameObject {
public:
  Decorator(skyVec3 position, float scale, skyGUID textureId, AssetStore &assetStore);
  void 					update(std::chrono::microseconds dt);
  void 					display(RenderState &renderState);
  bool 					load();
  //~Decorator();
private:
  // TODO(caleb): add private copy constructor
  float scale;
  Mesh *mesh;
  Texture *texture;
};


class Orc : public GameObject {
public:
  Orc(skyVec3 position, AssetStore &assetStore);
  void 					update(std::chrono::microseconds dt);
  void 					display(RenderState &renderState);
  void                  move(std::chrono::microseconds dt, skyVec3 dv, skyVec3 dw);
  bool 					load();
private:
  // TODO(caleb): add private copy constructor
  bool isDead = false;
  skyQuat               rotation;
  float 				scale;
  Mesh *				mesh;
  Texture *				texture;

};
