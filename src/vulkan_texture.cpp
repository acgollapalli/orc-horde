/*

SDG                                                                                               JJ

                                     Orc Horde

							  Vulkan Texture Implementation
*/

#include "asset.hh"

Texture::Texture(GUID guid, AssetStore &assetStore, Renderer &renderer)
  : Asset(guid, assetStore, renderer)
{}

// TODO(caleb): implement all these
bool Texture::load(){ return true; }
void Texture::unload(){}
void Texture::loadLOD(LOD lod){}
void Texture::loadLOD(LOD lod[]){}
void Texture::unloadLOD(LOD lod){}
void Texture::unloadLOD(LOD lod[]){}
