/*

SDG                                                                                               JJ

                                             Orc Horde

	        					Asset Store (Vulkan Implementation)
*/

#include "asset.hh"

AssetStore::AssetStore(Renderer &renderer)
  :renderer(renderer)
{
  AssetInfo houseMeshInfo {
	.type = Mesh_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = HOUSE_PATH,
	.asset = nullptr
  };
  assetDb[HOUSE_GUID] = houseMeshInfo;

  AssetInfo houseTextureInfo {
	.type = Texture_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = HOUSE_TEXTURE_PATH,
	.asset = nullptr,
  };
  assetDb[HOUSE_TEXTURE_GUID] = houseTextureInfo;

  AssetInfo orcMeshInfo {
	.type = Mesh_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = ORC_PATH,
	.asset = nullptr
  };
  assetDb[ORC_GUID] = orcMeshInfo;

  AssetInfo orcTextureInfo {
	.type = Texture_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = ORC_TEXTURE_PATH,
	.asset = nullptr,
  };
  assetDb[ORC_TEXTURE_GUID] = orcTextureInfo;

  AssetInfo mapTextureInfo {
	.type = Texture_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = MAP_TEXTURE_PATH,
	.asset = nullptr,
  };
  assetDb[MAP_TEXTURE_GUID] = mapTextureInfo;

  AssetInfo decoratorMeshInfo {
	.type = Mesh_e,
	.locationType = Computed_e,
	.assetSize = 0,
	.assetLocation = "", // TODO(caleb): add a way to get a computed value here
	.asset = nullptr,
  };
  assetDb[DECORATOR_GUID] = decoratorMeshInfo;

  AssetInfo humanMeshInfo {
	.type = Mesh_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = HUMAN_PATH,
	.asset = nullptr
  };
  assetDb[HUMAN_GUID] = humanMeshInfo;

  AssetInfo humanTextureInfo {
	.type = Texture_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = HUMAN_TEXTURE_PATH,
	.asset = nullptr,
  };
  assetDb[HUMAN_TEXTURE_GUID] = humanTextureInfo;

  AssetInfo bulletRegularMeshInfo {
	.type = Mesh_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = BULLET_MESH_PATH_REGULAR,
	.asset = nullptr
  };
  assetDb[BULLET_MESH_GUID_REGULAR] = bulletRegularMeshInfo;

  AssetInfo bulletRegularTextureInfo {
	.type = Texture_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = BULLET_TEXTURE_PATH_REGULAR,
	.asset = nullptr,
  };
  assetDb[BULLET_TEXTURE_GUID_REGULAR] = bulletRegularTextureInfo;

  AssetInfo bulletSuperMeshInfo {
	.type = Mesh_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = BULLET_MESH_PATH_SUPER,
	.asset = nullptr
  };
  assetDb[BULLET_MESH_GUID_SUPER] = bulletSuperMeshInfo;

  AssetInfo bulletSuperTextureInfo {
	.type = Texture_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = BULLET_TEXTURE_PATH_SUPER,
	.asset = nullptr,
  };
  assetDb[BULLET_TEXTURE_GUID_SUPER] = bulletSuperTextureInfo;

  AssetInfo explosionMeshInfo {
	.type = Mesh_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = EXPLOSION_PATH,
	.asset = nullptr
  };
  assetDb[EXPLOSION_GUID] = explosionMeshInfo;

  AssetInfo explosionTextureInfo {
	.type = Texture_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = EXPLOSION_TEXTURE_PATH,
	.asset = nullptr,
  };
  assetDb[EXPLOSION_TEXTURE_GUID] = explosionTextureInfo;

  AssetInfo explosionSuperMeshInfo {
	.type = Mesh_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = SUPER_EXPLOSION_PATH,
	.asset = nullptr
  };
  assetDb[SUPER_EXPLOSION_GUID] = explosionSuperMeshInfo;

  AssetInfo explosionSuperTextureInfo {
	.type = Texture_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = SUPER_EXPLOSION_TEXTURE_PATH,
	.asset = nullptr,
  };
  assetDb[SUPER_EXPLOSION_TEXTURE_GUID] = explosionSuperTextureInfo;

  AssetInfo humanDeadMeshInfo {
	.type = Mesh_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = HUMAN_DEAD_PATH,
	.asset = nullptr
  };
  assetDb[HUMAN_DEAD_GUID] = humanDeadMeshInfo;

  AssetInfo humanDeadTextureInfo{
	.type = Texture_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = HUMAN_DEAD_TEXTURE_PATH,
	.asset = nullptr
  };
  assetDb[HUMAN_DEAD_TEXTURE_GUID] = humanDeadTextureInfo;
}

bool AssetStore::load(skyGUID guid) {
  Asset *asset = get(guid);
  std::printf("loading %s\n", guid.c_str());
  return asset->load();
}

bool AssetStore::load(std::vector<skyGUID> guids) {
  bool allLoaded = true;
  for ( skyGUID guid : guids ) {
	allLoaded == allLoaded && load(guid);
  }
  return allLoaded;
}

Asset *AssetStore::get(skyGUID guid) {
  AssetInfo info = assetDb.at(guid);

  // TODO(caleb): See how this is used
  // because we may just want to load the asset here
  if (info.asset != nullptr) {
	return info.asset;
  } else {
	Asset *asset;
	switch (info.type) {
	case Texture_e:
	  asset = new Texture(guid, *this, renderer);
	  break;
	case Mesh_e:
	  asset = new Mesh(guid, *this, renderer);
	  break;
	default:
	  throw std::logic_error("other types of assets not yet defined!");
	}

	assetDb[guid].asset = asset;
	return asset;
  }
}

Texture *AssetStore::getTexture(skyGUID guid) {
  AssetInfo info;
  try {
	info = assetDb.at(guid);
	if (info.type != Texture_e) {
	  std::printf("Got bad GUID %s \n", guid.c_str());
	}
	assert(info.type == Texture_e);
  } catch (const std::out_of_range& ex) {
	return nullptr; // TODO: remove this eventually
  }

  if (info.asset != nullptr) {
	return static_cast<Texture *>(info.asset);
  } else {
	assetDb[guid].asset = new Texture(guid, *this, renderer);
	return static_cast<Texture *>(assetDb[guid].asset);
  }
}

Mesh *AssetStore::getMesh(skyGUID guid) {
  AssetInfo info;
  try {
	info = assetDb.at(guid);
	assert(info.type == Mesh_e);
  } catch (const std::out_of_range& ex) {
	return nullptr; // TODO: remove this eventually
  }


  if (info.asset != nullptr) {
	return static_cast<Mesh *>(info.asset);
  } else {
	assetDb[guid].asset = new Mesh(guid, *this, renderer);
	return static_cast<Mesh *>(assetDb[guid].asset);
  }
}

AssetLocation AssetStore::getLocation(skyGUID guid) {
  // TODO(caleb): AssetLocation is a string, should it be a struct
  // with AssetLocationType on it?
  AssetInfo info = assetDb[guid];
  return info.assetLocation;
}


AssetLocationType AssetStore::getLocationType(skyGUID guid) {
  // TODO(caleb): AssetLocation is a string, should it be a struct
  // with AssetLocationType on it?
  AssetInfo info = assetDb[guid];
  return info.locationType;
}

