/*

SDG                                                                                               JJ

                                     Orc Horde

							  Vulkan Mesh Implementation
*/

#include "asset.hh"

Mesh::Mesh(GUID guid, AssetStore &assetstore, Renderer &renderer)
  : Asset(guid, assetStore, renderer)
{}

bool Mesh::load() {
  if (loaded) return;

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

RenderOp Mesh::display () {
  return RenderOp {
	.type = DrawMeshSimple,
	.vertexBuffer = vertices_st.buffer,
	.indexBuffer = indices_st.buffer,
	.instanceBuffer = {},
  };
}
