/*

SDG                                                                                               JJ

                                     Orc Horde

									 Renderer
*/

#include <vector>
#include <map>
#include "vendor/tiny_obj_loader.h"

#include "asset.hh"

/* ========================== Asset Base Class ==========================*/
Asset::Asset(GUID guid, AssetStore &assetStore, Renderer &renderer)
  :guid(guid)
  ,assetStore(assetStore)
  ,renderer(renderer)
  ,loaded(false) // do we need this?
{
  // do something here
  // RAII is the C++ thing, so maybe we should acquire the asset from a file?
  // or should we provide explicit loading and caching semantics?

  // ... let's be explicit. And let's write something that we can easily
  // port to a c header later.
}

Asset::~Asset() {
  //if (loaded) unload(); // we MAY actually just want to have this in AssetStore instead
}
