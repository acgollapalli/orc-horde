/*

SDG                                                                                               JJ

                                     Orc Horde

							  Vulkan Mesh Implementation
*/

#include "asset.hh"

Mesh::Mesh(skyGUID guid, AssetStore &assetStore, Renderer &renderer)
  : Asset(guid, assetStore, renderer)
{}

bool Mesh::load() {
  if (loaded) return true;

  switch (assetStore.getLocationType(guid)) {
  case File_e: 		return loadFromFile();
  case Computed_e:	return loadComputed();
  default: 			throw std::logic_error("Only file and computed meshes can be loaded for meshes");
  }

  return false;
}

bool Mesh::loadFromFile() {
  std::string modelPath = assetStore.getLocation(guid);

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials,
						 &err, modelPath.c_str())) {
	throw std::runtime_error(err);
  }

  std::unordered_map<Vertex, uint32_t> uniqueVertices{};

  std::vector<Vertex> vertices{};
  std::vector<Index> indices{};

  for (const auto &shape : shapes) {
	for (const auto &index : shape.mesh.indices) {
	  Vertex vertex{};

	  vertex.pos = {
		attrib.vertices[3 * index.vertex_index + 0],
		attrib.vertices[3 * index.vertex_index + 1],
		attrib.vertices[3 * index.vertex_index + 2]
	  };

	  vertex.texCoord = {
		attrib.texcoords[2 * index.texcoord_index + 0],
		1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
	  };

	  vertex.color = {1.0f, 1.0f, 1.0f};

	  if (uniqueVertices.count(vertex) == 0) {
		uniqueVertices[vertex] = static_cast<Index>(vertices.size());
		vertices.push_back(vertex);
	  }

	  indices.push_back(uniqueVertices[vertex]);
	}
  }

  vertices_st.size = static_cast<uint32_t>(vertices.size()); // not currently used
  indices_st.size = static_cast<uint32_t>(indices.size());
  renderer.createVertexBuffer(vertices, vertices_st.buffer, vertices_st.memory);
  renderer.createIndexBuffer(indices, indices_st.buffer, indices_st.memory);
  return true;
}

// TODO(Caleb): You want to get passed a function pointer or something here
// but right now the only thing we're computing is the Plane shape so we don't generalize it.
bool Mesh::loadComputed() {
  const std::vector<Vertex> vertices = {
	/*     Position                 Color           Texture (UV) */
	{ {-0.5f, -0.5f, 0.0f},	 {1.0f, 0.0f, 0.0f}, 	{0.0f, 1.0f} },
    { {0.5f, -0.5f, 0.0f},	 {0.0f, 1.0f, 0.0f}, 	{1.0f, 1.0f} },
    { {0.5f, 0.5f, 0.0f}, 	 {0.0f, 0.0f, 1.0f}, 	{1.0f, 0.0f} },
    { {-0.5f, 0.5f, 0.0f},	 {1.0f, 1.0f, 1.0f}, 	{0.0f, 0.0f} }
  };
  
  const std::vector<Index> indices = { 0, 1, 2, 2, 3, 0 };
  
  vertices_st.size = static_cast<uint32_t>(vertices.size()); // not currently used
  indices_st.size = static_cast<uint32_t>(indices.size());
  renderer.createVertexBuffer(vertices, vertices_st.buffer, vertices_st.memory);
  renderer.createIndexBuffer(indices, indices_st.buffer, indices_st.memory);
  return true;
}

void Mesh::unload() {
  renderer.destroyBuffer(vertices_st.buffer);
  renderer.freeMemory(vertices_st.memory);

  renderer.destroyBuffer(indices_st.buffer);
  renderer.freeMemory(indices_st.memory);

  // TODO(caleb): destroy instance memory here
}

void	Mesh::load(LOD lod) {}
void	Mesh::unload(LOD lod){}

void Mesh::display (RenderState &renderState, Instance &thisInstance) {
  Renderable &renderable = renderState.assets[guid];
  if (renderable.numIndices != indices_st.size) {
	renderable.vertexBuffer = vertices_st.buffer;
	renderable.indexBuffer = indices_st.buffer;
	renderable.numIndices = indices_st.size;
  }
  renderable.instances.push_back(thisInstance);
}
