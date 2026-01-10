#pragma once

namespace tinygltf { class Model; struct Mesh; struct Primitive; }

class BasicMesh
{
public:
	struct Vertex {
		Vector3 position = Vector3::Zero;
		Vector2 texCoord0 = Vector2::Zero;
		Vector3 normal = Vector3::UnitZ;
		Vector3 tangent = Vector3::UnitX;
	};

	void Load(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive);

private:

	typedef std::unique_ptr<Vertex[]> VertexArray;
	typedef std::unique_ptr<uint8_t[]> IndexArray;

	std::string name;
	uint32_t numVertices = 0;
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	uint32_t indexElementSize = 0;
	uint32_t numIndices = 0;
	int32_t materialIndex = -1;
	
	VertexArray vertices;
	IndexArray indices;

	bool LoadAccessorData(uint8_t* data, size_t elemSize, size_t stride, size_t count, const tinygltf::Model& model, int index);
	bool LoadAccessorData(uint8_t* data, size_t elemSize, size_t stride, size_t count, const tinygltf::Model& model, const std::map<std::string, int>& attributes, const char* accessorName);
};

