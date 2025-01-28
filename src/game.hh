/*

SDG                                                                                               JJ

                                               Orc Horde

									A Game Made (almost) from scratch
*/

#pragma once

#include <chrono>
#include <iostream>
#include <random>
#include <vector>

class GameObject;
class AssetStore;

const int ORCS_PER_FRAME = 1; // FIXME(caleb): need this to be per ms
const int HUMANS_PER_FRAME = 1; // FIXME(caleb): need this to be per ms

enum GameOpType {
  Spawn_e,
  Kill_e,
  DeleteSelf_e,
};

struct GameState {
  AssetStore &assetStore;
  std::vector<GameObject*> gameObjects;
};


struct GameOp {
  GameOpType 	type;
  GameObject *  operand;
};
