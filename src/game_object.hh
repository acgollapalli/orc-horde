/*

SDG                                                                                               JJ

                                     Orc Horde

									 Game Object
*/

#pragma once

#include "math.hh"
#include <chrono>
using namespace std::literals;

#include "game.hh"
#include "asset.hh"

typedef skyVec3 Location;
typedef skyVec2 DecoratorSize;

typedef std::vector<GameOp> GameOps;

enum GameObjectType {
  RigidBody_e, // TODO(caleb): add these to each constructor
  Decorator_e,
  Orc_e,
  Human_e,
  Bullet_e,
  Animation_e,
};

class GameObject {
public:
  GameObject(skyVec3 position) :position(position){};
  //~GameObject();

  
  virtual GameOps 		update(std::chrono::microseconds dt, GameState &gameState) = 0;
  virtual void 			display(RenderState &renderState) = 0;
  virtual bool			load() = 0;
  virtual GameOps		kill(GameState &gameState);
  //protected:
  skyVec3 				position;
  GameObjectType		type;
  std::vector<GameOp>   mailbox;
  uint32_t				generation = 0;
};

inline GameOps GameObject::kill(GameState &gameState) {
  	GameOp deleteSelf {
	  .type = DeleteSelf_e,
	  .operand = this,
	};
	GameOps ops;
	ops.push_back(deleteSelf);
	return ops;
}
  

class RigidBody : public GameObject {
public:
  RigidBody(skyVec3 position, skyQuat rotation, float scale,
			GUID textureId, GUID meshId, AssetStore &assetStore);
  //~RigidBody();
  GameOps 				update(std::chrono::microseconds dt, GameState &gameState);
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
  GameOps 				update(std::chrono::microseconds dt, GameState &gameState);
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
  GameOps 				update(std::chrono::microseconds dt, GameState &gameState);
  void 					display(RenderState &renderState);
  void                  move(std::chrono::microseconds dt, skyVec3 dv, skyVec3 dw);
  bool 					load();
private:
  // TODO(caleb): add private copy constructor
  skyQuat               rotation;
  float 				scale;
  Mesh *				mesh;
  Texture *				texture;
  Location				findNearestHuman(GameState &gameState, GameObject **human);

};

class Human: public GameObject {
public:
  Human(skyVec3 position, AssetStore &assetStore);
  GameOps 				update(std::chrono::microseconds dt, GameState &gameState);
  void 					display(RenderState &renderState);
  void                  move(std::chrono::microseconds dt, skyVec3 dv, skyVec3 dw);
  bool 					load();
  GameOps				kill(GameState &gameState); // TODO(caleb): add death animation
private:
  // TODO(caleb): add private copy constructor
  skyQuat               rotation;
  float 				scale;
  Mesh *				mesh;
  Texture *				texture;
  bool					blessed;
  int					usSinceLastFired;
  Location				findNearestOrc(GameState &gameState);
  GameOp				fireAtOrc(Location location, GameState &gameState);
};

class Bullet: public GameObject {
public:
  Bullet(skyVec3 position, skyVec3 direction, bool superBullet, AssetStore &assetStore);
  GameOps 						update(std::chrono::microseconds dt, GameState &gameState);
  void 							display(RenderState &renderState);
  void                  		move(std::chrono::microseconds dt, skyVec3 dv, skyVec3 dw);
  bool 							load();
private:
  // TODO(caleb): add private copy constructor
  Mesh *						mesh;
  Texture *						texture;
  skyQuat               		rotation;
  float 						scale;

  skyVec3						direction;
  bool 							superBullet;
  std::vector<GameObject *>		scanEnemies(float hitRadius, float killRadius, GameState &gameState);
};

// FIXME(caleb): right now animations are just meshes that are spawned and then self delete
class Animation: public GameObject {
public:
  Animation(skyVec3 position, skyGUID textureId, skyGUID meshId,
			std::chrono::milliseconds duration, AssetStore &assetStore);
  GameOps 						update(std::chrono::microseconds dt, GameState &gameState);
  void 							display(RenderState &renderState);
  void                  		move(std::chrono::microseconds dt, skyVec3 dv, skyVec3 dw);
  bool 							load();
private:
  std::chrono::milliseconds     timeLeft;
  skyQuat               		rotation;
  float 						scale;
  Mesh *						mesh;
  Texture *						texture;
};
