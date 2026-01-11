#include "Globals.h"
#include "Application.h"
#include "ModuleResources.h"
#include "BasicMaterial.h"
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"

void BasicMaterial::Load(const tinygltf::Model& model, const tinygltf::Material& material, Type type, const char* basepath)
{
	name = material.name;

	materialType = type;

	Vector4 baseColor = Vector4(float(material.pbrMetallicRoughness.baseColorFactor[0]),
		float(material.pbrMetallicRoughness.baseColorFactor[1]),
		float(material.pbrMetallicRoughness.baseColorFactor[2]),
		float(material.pbrMetallicRoughness.baseColorFactor[3]));

	BOOL hasColourTexture = FALSE;
	auto loadTexture = [](int index, const tinygltf::Model& model, const char* basepath, bool defaultSRGB, ComPtr<ID3D12Resource>& outTex) -> BOOL
		{
			if (index < 0 || index >= int(model.textures.size()))
			{
				return FALSE;
			}

			const tinygltf::Texture& texture = model.textures[index];
			const tinygltf::Image& image = model.images[texture.source];

			if (!image.uri.empty())
			{
				outTex = app->getResources()->CreateTextureFromFile(std::string(basepath) + image.uri);
				return true;
			}

			return FALSE;
		};

	hasColourTexture = loadTexture(material.pbrMetallicRoughness.baseColorTexture.index, model, basepath, true, basecColourTex);

	if (materialType == BASIC) 
	{
		materialData.basic.baseColor = baseColor;
		materialData.basic.hasColorTexture = hasColourTexture;
	}
	else if(materialType == PBR_PHONG)
	{
		materialData.pbrPhong.difuseColor = XMFLOAT3(baseColor.x, baseColor.y, baseColor.z);
		materialData.pbrPhong.hasDefuseTex = hasColourTexture;
		materialData.pbrPhong.shininess = 64.0f;
		materialData.pbrPhong.specularColor = XMFLOAT3(0.015f, 0.015f, 0.015f);
	}
	

	ModuleShaderDescriptors* descriptors = app->getShaderDescriptors();

	textureTableDesc = descriptors->GetGPUHandle((UINT8)0u);

	if (hasColourTexture) 
	{
		descriptors->createTextureSRV(basecColourTex.Get(), 0);
	}
	else 
	{
		descriptors->createNullTexture2DSRV();
	}
}
