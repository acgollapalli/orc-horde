/*

SDG                                                                                               JJ

                                     Orc Horde

							  Vulkan Texture Implementation
*/

#include "vendor/stb_image.h"

#include "asset.hh"

Texture::Texture(skyGUID guid, AssetStore &assetStore, Renderer &renderer)
  : Asset(guid, assetStore, renderer) {}

bool Texture::load(){
  if (loaded) return true;

  std::string texturePath = assetStore.getLocation(guid); // TODO(caleb): get rid or std::string

  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(texturePath.c_str(),
							  &texWidth, &texHeight,
							  &texChannels, STBI_rgb_alpha);
  size_t imageSize = imageSize = texWidth * texHeight * 4;

  if (!pixels) { // TODO(caleb): Maybe assert instead  of throwing errors?
	char dst[500];
	// TODO(caleb): get rid of std::string
	std::sprintf(dst, "failed to load texture image! path: %s", texturePath.c_str()); 
	throw std::runtime_error(dst);
  }


  // TODO(caleb): Right now this doesn't store the miplevels because we're generating them
  // at load time. We may need that later
  renderer.createTextureImage(pixels, texWidth, texHeight, texChannels,
							  image.image, image.memory, image.imageView);

  renderer.addTextureImageToDescriptorSet(image.imageView, image.layerOffset);

  stbi_image_free(pixels);
  return true;
}

void Texture::unload(){
  renderer.destroyImageView(image.imageView);
  renderer.destroyImage(image.image);
  renderer.freeMemory(image.memory);
}

// TODO(caleb): implement all these
void Texture::loadLOD(LOD lod){}
void Texture::loadLOD(LOD lod[]){}
void Texture::unloadLOD(LOD lod){}
void Texture::unloadLOD(LOD lod[]){}

uint32_t Texture::getLayerOffset() {
  return image.layerOffset;
}
