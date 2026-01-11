#pragma once
#include <span>
#include "BasicMesh.h"
#include "BasicMaterial.h"

namespace tinygltf { class Model; }

class BasicModel
{
public:
	void Load(const char* filename, const char* basePath, BasicMaterial::Type materialType);

	std::span<const BasicMesh> GetMeshes() const { return std::span<const BasicMesh>(meshes.get(), numMeshes);}
	std::span<const BasicMaterial> GetMaterials() const { return std::span<const BasicMaterial>(materials.get(), numMaterials); }
	std::span<BasicMaterial> GetMaterials() { return std::span<BasicMaterial>(materials.get(), numMaterials); }

	uint32_t GetNumMaterials() { return numMaterials; }

	void SetModelMatrix(const Matrix& m) { matrix = m; }
	const Matrix& GetModelMatrix() const { return matrix; }

private:
	void LoadMeshes(const tinygltf::Model& model);
	void LoadMaterials(const tinygltf::Model& model, const char* basepath, BasicMaterial::Type materialType);
	
	std::unique_ptr < BasicMaterial[] > materials;
	std::unique_ptr < BasicMesh[] > meshes;

	uint32_t numMeshes = 0;
	uint32_t numMaterials = 0;

	Matrix matrix = Matrix::Identity;
};

