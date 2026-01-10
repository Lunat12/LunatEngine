#include "Globals.h"
#include "BasicModel.h"
#include "Application.h"
#include "ModuleShaderDescriptors.h"
#include "ModuleResources.h"
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_IMPLEMENTATION /* Only in one of the includes */
#include "tiny_gltf.h"

void BasicModel::Load(const char* filename, const char* basePath)
{
	tinygltf::TinyGLTF gltfContext;
	tinygltf::Model model;
	std::string error, warning;
	bool loadOk = gltfContext.LoadASCIIFromFile(&model, &error, &warning, filename);
	if (loadOk)
	{
		LoadMeshes(model);
		LoadMaterials(model, basePath);
	}
	else LOG("Error loading %s: %s", filename, error.c_str());
}

void BasicModel::LoadMeshes(const tinygltf::Model& model)
{
	auto countPrimitives = [](const tinygltf::Model& m) -> size_t
		{
			size_t count = 0;
			for (const tinygltf::Mesh& mesh : m.meshes)
			{
				count += mesh.primitives.size();
			}
			return count;
		};

	numMeshes = uint32_t(countPrimitives(model));

	meshes = std::make_unique<BasicMesh[]>(numMeshes);
	int MeshIndex = 0;

	for (const tinygltf::Mesh& mesh : model.meshes)
	{
		for (const tinygltf::Primitive& primitive : mesh.primitives)
		{
			meshes[MeshIndex++].Load(model, mesh, primitive);
		}
	}
}

void BasicModel::LoadMaterials(const tinygltf::Model& model, const char* basepath)
{
	ModuleShaderDescriptors* descriptors = app->getShaderDescriptors();
	ModuleResources* resources = app->getResources();
	
	numMaterials = uint32_t(model.materials.size());
	materials = std::make_unique<BasicMaterial[]>(numMaterials);

	int materialIndex = 0;
	for (const tinygltf::Material& material : model.materials) 
	{
		materials[materialIndex++].Load(model, material, basepath);
	}
}
