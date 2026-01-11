#pragma once

#include "ModuleShaderDescriptors.h"

namespace tinygltf { class Model; struct Material; }

struct BasicMaterialData 
{
	XMFLOAT4 baseColor;
	BOOL hasColorTexture;
};

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
	enum Type
	{
		BASIC = 0,
		PHONG,
		PBR_PHONG,
		METALLIC_ROUGHNESS
	};

	Type materialType = BASIC;

	void Load(const tinygltf::Model& model, const tinygltf::Material& material, Type type, const char* basepath);

	const D3D12_GPU_DESCRIPTOR_HANDLE& GetTexturesTableDesc() const { return textureTableDesc; }
	const PBRPhongMaterialData& GetPBRPhongMaterial() const { _ASSERTE(materialType == PBR_PHONG); return materialData.pbrPhong; }
	const BasicMaterialData& GetBasicMaterial() const { _ASSERTE(materialType == BASIC); return materialData.basic; }

private:
	union 
	{
		BasicMaterialData basic;
		PBRPhongMaterialData pbrPhong;
	} materialData;

	std::string name;
	ComPtr<ID3D12Resource> basecColourTex;
	D3D12_GPU_DESCRIPTOR_HANDLE textureTableDesc;

	
};

