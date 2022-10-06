#include "MaterialManager.h"
#define STBI_WINDOWS_UTF8
#include "stb_image.h"
#include "XUtil.h"
#include "DXTrace.h"
#include "DDSTextureLoader.h"
#include <filesystem>
#include "Application.h"

namespace
{
	// TextureManagerµ¥Àý
	MaterialManager* s_pInstance = nullptr;
}

MaterialManager::MaterialManager(Application* application)
{
	name = "Material Manager";
	app = application;
	if (s_pInstance)
		throw std::exception("MaterialManager is a singleton!");
	s_pInstance = this;
}

MaterialManager::~MaterialManager()
{
	m_pMaterials.clear();
}

MaterialManager& MaterialManager::Get()
{
	if (!s_pInstance)
		throw std::exception("MaterialManager needs an instance!");
	return *s_pInstance;
}

bool MaterialManager::Init(ID3D11Device* device)
{
	return true;
}

Material* MaterialManager::createMaterial(std::string_view filename, const XMFLOAT4 ambient,
	const XMFLOAT4 diffuse, const XMFLOAT4 specular, const XMFLOAT4 emissive, const XMFLOAT4 transparent,
	const XMFLOAT4 reflective) 
{
	XID nameID = StringToID(filename);
	if (m_pMaterials.find(nameID) != m_pMaterials.end())
		return m_pMaterials[nameID];
	Material* material = new Material();
	XMFLOAT4 vec{};
	float value{};
	uint32_t boolean{};
	uint32_t num = 3;
	material->Set("$AmbientColor", vec);
	material->Set("$DiffuseColor", vec);
	material->Set("$SpecularColor", vec);
	material->Set("$EmissiveColor", vec);
	material->Set("$TransparentColor", vec);
	material->Set("$ReflectiveColor", vec);
	TextureImporter::Get().CreateTexture(filename, true, true);
	material->SetTexture("$Diffuse", std::string{ filename }.c_str());
	//material->SetTexture("$Normal", std::string{ filename }.c_str());
	
	m_pMaterials[nameID] = material;
	return material;
}

bool MaterialManager::AddMaterial(std::string_view name, Material* material)
{
	XID nameID = StringToID(name);
	m_pMaterials[nameID] = material;
	return true;
}

Material* MaterialManager::GetMaterial(std::string_view filename)
{
	XID fileID = StringToID(filename);
	if (m_pMaterials.find(fileID) != m_pMaterials.end())
		return m_pMaterials[fileID];
	return nullptr;
}
