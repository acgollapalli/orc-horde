/*

SDG                                                                                               JJ

                                     Orc Horde

									 Assets
*/

#pragma once

#include <string>
#include <vector>
#include "vendor/tiny_obj_loader.h"

#include "renderer.hh" // TODO: Move vertex code to separate file

// TODO: IFDEF THIS:
struct VulkanBufferInfo {
  VkBuffer buffer;
  VkDeviceMemory memory;
  uint32_t size;
};

typedef VulkanBufferInfo VertexBuffer_st;
typedef VulkanBufferInfo IndexBuffer_st;

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

typedef std::string GUID;
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

typedef std::unordered_map<GUID, AssetInfo> AssetDB;

class AssetStore {
public:
  AssetStore(Renderer &renderer);
  void  		    		load(GUID guid);
  Asset * 					get(GUID guid);
  Texture *					get_texture(GUID guid); // TODO(caleb): fix this (odin casing instead of C++)
  Mesh *					get_mesh(GUID guid);
  int 						relinquish(GUID guid); // returns number of other claims on asset
  void						unload(GUID guid);
  void						force_unload(GUID guid);
  AssetLocation 			getLocation(GUID guid); // SUBJECT TO CHANGEs

private:
  AssetDB			        assetDb;
  Renderer & 				renderer;
};


	
/* ========================== Asset Classes ==========================*/
class Asset {
public:
  Asset(GUID guid, AssetStore &assetStore, Renderer &renderer); // TODO replace std string with something that isn't std string
  virtual ~Asset();
  // TODO Add copy and move constructors for all these because you do NOT want to be
  // copying all that data around
  
  GUID 					guid;
  virtual bool 			load() = 0;   // TODO: maybe this doesn't need to be virtual
  virtual void			unload() = 0; // TODO: maybe this doesn't have to be virtual either
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
  friend class AssetStore;
private:
  Texture(GUID guid, AssetStore &assetStore, Renderer &renderer);
  std::vector<LOD>	lod;
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
  RenderOp					display();
  friend class AssetStore;
  
private:
  Mesh(GUID guid, AssetStore &assetStore, Renderer &renderer);
  std::vector<LOD>			lod;
  VertexBuffer_st			vertices_st;
  IndexBuffer_st			indices_st;
};



