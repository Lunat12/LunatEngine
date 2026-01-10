#pragma once

#include "ModuleShaderDescriptors.h"

namespace tinygltf { class Model; struct Material; }

struct PBRPhongMaterialData 
{
	XMFLOAT3 difuseColor;
	BOOL hasDefuseTex;
	XMFLOAT3 specularColor;
	float shininess;
};

class BasicMaterial
{
public:

	void Load(const tinygltf::Model& model, const tinygltf::Material& material, const char* basepath);

private:
	std::string name;
	ComPtr<ID3D12Resource> basecColourTex;
	ModuleShaderDescriptors textureTableDesc;

	PBRPhongMaterialData pbrPhong;
};

