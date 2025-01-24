/*

SDG                                                                                               JJ

                                     Orc Horde

									 Assets
*/

#pragma once

#include <string>
#include <vector>
#include "vendor/tiny_obj_loader.h"

#include "containers.hh"

#include "renderer.hh" // TODO: Move vertex code to separate file

// TODO(caleb): move all this to a Plaform specific header full of ifdefs
struct VulkanBufferInfo {
  VkBuffer buffer;
  VkDeviceMemory memory;
  uint32_t size;
};

typedef VulkanBufferInfo VertexBuffer_st;
typedef VulkanBufferInfo IndexBuffer_st;

struct VulkanImageInfo {
  VkImage image;
  VkDeviceMemory memory;
  VkImageView imageView;
  uint32_t layerOffset;
};

typedef VulkanImageInfo Image_st;
// END IFDEF

// TODO: consider replacing with STL style traits class
typedef enum {
  Mesh_e,
  Texture_e,
  Sound_e,
  Video_e,
  Bytecode_e,
  Other_e,
} AssetType;

typedef enum {
  File_e,
  Network_e,
} AssetLocationType;

typedef std::string skyGUID;
typedef std::string AssetLocation;
typedef size_t AssetSize;


typedef int LOD;

class Asset;
class Mesh;
class Texture;

/* ========================== Asset Storage ==========================*/
struct AssetInfo {
  AssetType type;
  AssetLocationType locationType;
  AssetSize assetSize;
  AssetLocation assetLocation;
  Asset *asset;
};

typedef std::unordered_map<skyGUID, AssetInfo> AssetDB;

class AssetStore {
public:
  AssetStore(Renderer &renderer);
  void  		    		load(skyGUID guid);
  Asset * 					get(skyGUID guid);
  Texture *					getTexture(skyGUID guid); // TODO(caleb): fix this (odin casing instead of C++)
  Mesh *					getMesh(skyGUID guid);
  int 						relinquish(skyGUID guid); // returns number of other claims on asset
  void						unload(skyGUID guid);
  void						forceUnload(skyGUID guid);
  AssetLocation 			getLocation(skyGUID guid); // SUBJECT TO CHANGES

private:
  AssetDB			        assetDb;
  Renderer & 				renderer;
};


	
/* ========================== Asset Classes ==========================*/
class Asset {
public:
  Asset(skyGUID guid, AssetStore &assetStore, Renderer &renderer); // TODO replace std string with something that isn't std string
  virtual ~Asset();
  // TODO Add copy and move constructors for all these because you do NOT want to be
  // copying all that data around
  
  skyGUID 					guid;
  virtual bool 			load() = 0;   // TODO: maybe this doesn't need to be virtual
  virtual void			unload() = 0; // TODO: maybe this doesn't have to be virtual either
  int					generation = 0; 
  friend class AssetStore;
protected:
  bool 					loaded;
  AssetStore & 			assetStore;
  Renderer & 			renderer;
};


// TODO: For loading LOD's, the most common use case will probably be loading
// the higher LOD's as you get closer to an object
// Is the best way to just a pointer to each individual LOD, or to copy them
// over into a graphics driver compatible, mipmapped single buffer each time
// a new LOD is requested.
// or maybe we store a separete LOD object of texture and asset pairings?
class Texture : public Asset {
public:
  //  ~Texture();
  bool              load();
  void				unload();
  void              loadLOD(LOD lod);
  void              loadLOD(LOD lod[]);
  void              unloadLOD(LOD lod);
  void              unloadLOD(LOD lod[]);
  uint32_t			getLayerOffset();
  friend class AssetStore;
private:
  Texture(skyGUID guid, AssetStore &assetStore, Renderer &renderer);
  std::vector<LOD>	lod;
  Image_st			image;
};

// same questions apply, except here we have the additional question of 
// are we passing multiple separate LOD buffers to the thing or what?
// I'm sure i'll figure it out as we get into actually building the game
class Mesh : public Asset {
public:
  //~Mesh();

  bool              		load();
  void						load(LOD lod);
  void						unload();
  void						unload(LOD lod);
  void 						display(RenderState &renderState, Instance &thisInstance);
  friend class AssetStore;
  
private:
  Mesh(skyGUID guid, AssetStore &assetStore, Renderer &renderer);
  std::vector<LOD>			lod;
  VertexBuffer_st			vertices_st;
  IndexBuffer_st			indices_st;
};



