#pragma once
#include "BasicMesh.h"
#include "BasicMaterial.h"

namespace tinygltf { class Model; }

class BasicModel
{
public:
	void Load(const char* filename, const char* basePath);

private:
	void LoadMeshes(const tinygltf::Model& model);
	void LoadMaterials(const tinygltf::Model& model, const char* basepath);
	
	std::unique_ptr < BasicMaterial[] > materials;
	std::unique_ptr < BasicMesh[] > meshes;

	uint32_t numMeshes = 0;
	uint32_t numMaterials = 0;
};

