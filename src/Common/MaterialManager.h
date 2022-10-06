#ifndef MATERIAL_MANAGER_H
#define MATERIAL_MANAGER_H
#include <unordered_map>
#include <string>
#include "WinMin.h"
#include <d3d11_1.h>
#include <wrl/client.h>
#include "Base.h"
#include <DirectXMath.h>
using namespace DirectX;
class Material;
class MaterialManager :public Base
{
public:
    MaterialManager(Application* application);
    ~MaterialManager();
    MaterialManager(MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;
    MaterialManager(MaterialManager&&) = default;
    MaterialManager& operator=(MaterialManager&&) = default;

    static MaterialManager& Get();
    bool Init(ID3D11Device* device);

	Material* createMaterial(std::string_view filename,
		const XMFLOAT4 ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f),
		const XMFLOAT4 diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f),
		const XMFLOAT4 specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f),
		const XMFLOAT4 emissive = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f),
		const XMFLOAT4 transparent = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f),
		const XMFLOAT4 reflective = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f));

    bool AddMaterial(std::string_view name, Material* material);
    Material* GetMaterial(std::string_view filename);

private:
    using XID = size_t;
    std::unordered_map<XID, Material*> m_pMaterials;
};
#endif //TEXTURE_IMPORTER