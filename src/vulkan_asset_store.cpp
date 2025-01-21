/*

SDG                                                                                               JJ

                                             Orc Horde

	        					Asset Store (Vulkan Implementation)
*/

#include "asset.hh"

const AssetLocation MODEL_PATH = "./models/viking_room/viking_room.obj";
const GUID MODEL_GUID = "viking_room1234";

AssetStore::AssetStore(Renderer &renderer)
  :renderer(renderer)
{
  AssetInfo assetInfo {
	.type = Mesh_e,
	.locationType = File_e,
	.assetSize = 0,
	.assetLocation = MODEL_PATH,
	.asset = nullptr
  };
  assetDb[MODEL_GUID] = assetInfo;
}

Asset *AssetStore::get(GUID guid) {
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

Texture *AssetStore::getTexture(GUID guid) {
  AssetInfo info;
  try {
	info = assetDb.at(guid);
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

Mesh *AssetStore::getMesh(GUID guid) {
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

AssetLocation AssetStore::getLocation(GUID guid) {
  // TODO(caleb): AssetLocation is a string, should it be a struct
  // with AssetLocationType on it?
  std::printf("\n\n\n assetDb size: %d \n\n\n", (assetDb.size()));
  AssetInfo info = assetDb[guid];
  return info.assetLocation;
}
