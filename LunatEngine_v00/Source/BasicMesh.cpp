#include "Globals.h"
#include "Application.h"
#include "ModuleResources.h"
#include "BasicMesh.h"
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"

void BasicMesh::Load(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive)
{
	name = mesh.name;

	const auto& itPos = primitive.attributes.find("POSITION");

	if (itPos != primitive.attributes.end()) 
	{
		ModuleResources* resources = app->getResources();

		const tinygltf::Accessor& posAcc = model.accessors[itPos->second];

		numVertices = uint32_t(posAcc.count);

		vertices = std::make_unique<Vertex[]>(numVertices);
		uint8_t* vertexData = reinterpret_cast<uint8_t*>(vertices.get());

		LoadAccessorData(vertexData + offsetof(Vertex, position), sizeof(Vector3), sizeof(Vertex), numVertices, model, itPos->second);
		LoadAccessorData(vertexData + offsetof(Vertex, texCoord0), sizeof(Vector3), sizeof(Vertex), numVertices, model, primitive.attributes, "TEXCOORD_0");
		LoadAccessorData(vertexData + offsetof(Vertex, normal), sizeof(Vector3), sizeof(Vertex), numVertices, model, primitive.attributes, "NORMAL");
		if (!LoadAccessorData(vertexData + offsetof(Vertex, tangent), sizeof(Vector3), sizeof(Vertex), numVertices, model, primitive.attributes, "TANGENT"))
		{
			std::vector<Vector4> tangents;
			tangents.resize(numVertices);
			if (LoadAccessorData(reinterpret_cast<uint8_t*>(tangents.data()), sizeof(Vector4), sizeof(Vector4), numVertices, model, primitive.attributes, "TANGENT")) 
			{
				for (UINT i = 0; i < numVertices; ++i)
				{
					Vector3& dst = vertices[i].tangent;
					const Vector4& src = tangents[i];
					dst.x = src.x;
					dst.y = src.y;
					dst.z = src.z * src.w;
				}
			}
		}

		vertexBuffer = resources->CreateDefaultBuffer(vertices.get(), numVertices * sizeof(Vertex));
		
		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(Vertex);
		vertexBufferView.SizeInBytes = numVertices * sizeof(BasicMesh::Vertex);

		if (primitive.indices >= 0) 
		{
			const tinygltf::Accessor& indAcc = model.accessors[primitive.indices];
			_ASSERT_EXPR(indAcc.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT || 
						 indAcc.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT || 
						 indAcc.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE, "unsuported index format");

			if(indAcc.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT || indAcc.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT || indAcc.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE)
			{
				static const DXGI_FORMAT formats[3] = { DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R32_UINT };

				indexElementSize = tinygltf::GetComponentSizeInBytes(indAcc.componentType);
				numIndices = uint32_t(indAcc.count);

				indices = std::make_unique<uint8_t[]>(numIndices * indexElementSize);
				LoadAccessorData(indices.get(), indexElementSize, indexElementSize, numIndices, model, primitive.indices);
				indexBuffer = resources->CreateDefaultBuffer(indices.get(), numIndices * indexElementSize);

				indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
				indexBufferView.Format = formats[indexElementSize << 1];
				indexBufferView.SizeInBytes = numIndices * indexElementSize;
			}
		}

		materialIndex = primitive.material;
	}
}

bool BasicMesh::LoadAccessorData(uint8_t* data, size_t elemSize, size_t stride, size_t count, const tinygltf::Model& model, int index)
{
	const tinygltf::Accessor& accesor = model.accessors[index];
	size_t defaultStride = tinygltf::GetComponentSizeInBytes(accesor.componentType) * tinygltf::GetNumComponentsInType(accesor.type);
	if (count == accesor.count && defaultStride == elemSize)
	{
		const tinygltf::BufferView& view = model.bufferViews[accesor.bufferView];
		const uint8_t* bufferData = reinterpret_cast<const uint8_t*>(&(model.buffers[view.buffer].data[accesor.byteOffset + view.byteOffset]));

		size_t bufferStride = view.byteStride == 0 ? defaultStride : view.byteStride;

		for (uint32_t i = 0; i < count; i++)
		{
			memcpy(data, bufferData, elemSize);
			data += stride;
			bufferData += bufferStride;
		}

		return true;
	}

	return false;
}

bool BasicMesh::LoadAccessorData(uint8_t* data, size_t elemSize, size_t stride, size_t count, const tinygltf::Model& model, const std::map<std::string, int>& attributes, const char* accessorName)
{
	const auto& it = attributes.find(accessorName);

	if (it != attributes.end()) 
	{
		return LoadAccessorData(data, elemSize, stride, count, model, it->second);
	}

	return false;
}
