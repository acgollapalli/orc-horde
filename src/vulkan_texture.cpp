/*

SDG                                                                                               JJ

                                     Orc Horde

							  Vulkan Texture Implementation
*/

#include "asset.hh"

Texture::Texture(GUID guid, AssetStore &assetstore, Renderer &renderer)
  : Asset(guid, assetStore, renderer)
{}

void Texture::load(){}
void Texture::unload(){}
void Texture::loadLOD(LOD lod){}
void Texture::loadLOD(LOD lod[]){}
void Texture::unloadLOD(LOD lod){}
void Texture::unloadLOD(LOD lod[]){}
