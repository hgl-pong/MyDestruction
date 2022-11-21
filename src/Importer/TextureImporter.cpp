#define STBI_WINDOWS_UTF8
#include "../Common/stb_image.h"
#include "TextureImporter.h"
#include "../Common/XUtil.h"
#include "../Common/DXTrace.h"
#include "../Common/DDSTextureLoader.h"
#include <filesystem>
#include "../Common/Application.h"

namespace
{
    // TextureManager单例
    TextureImporter* s_pInstance = nullptr;
}

TextureImporter::TextureImporter(Application*application)
{
    name = "Texture Importer";
    app = application;
    if (s_pInstance)
        throw std::exception("TextureImporter is a singleton!");
    s_pInstance = this;
}

TextureImporter::~TextureImporter()
{
}

TextureImporter& TextureImporter::Get()
{
    if (!s_pInstance)
        throw std::exception("TextureImporter needs an instance!");
    return *s_pInstance;
}

bool TextureImporter::Init(ID3D11Device* device)
{
    m_pDevice = device;
    m_pDevice->GetImmediateContext(m_pDeviceContext.ReleaseAndGetAddressOf());
	return true;
}

ID3D11ShaderResourceView* TextureImporter::CreateTexture(std::string_view filename, bool enableMips, bool forceSRGB)
{
	Logger::Debug(name, "导入纹理:%s", std::string{ filename }.c_str());
	XID fileID = StringToID(filename);
	if (m_TextureSRVs.count(fileID))
		return m_TextureSRVs[fileID].Get();

	auto& res = m_TextureSRVs[fileID];
	std::wstring wstr = UTF8ToWString(filename);
	if (FAILED(DirectX::CreateDDSTextureFromFileEx(m_pDevice.Get(),
		enableMips ? m_pDeviceContext.Get() : nullptr,
		wstr.c_str(), 0, D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE, 0, 0,
		forceSRGB, nullptr, res.GetAddressOf())))
	{
		int width, height, comp;

		stbi_uc* img_data = stbi_load(filename.data(), &width, &height, &comp, STBI_rgb_alpha);
		if (img_data)
		{
			CD3D11_TEXTURE2D_DESC texDesc(forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
				width, height, 1,
				enableMips ? 0 : 1,
				D3D11_BIND_SHADER_RESOURCE | (enableMips ? D3D11_BIND_RENDER_TARGET : 0),
				D3D11_USAGE_DEFAULT, 0, 1, 0,
				enableMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
			Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
			HR(m_pDevice->CreateTexture2D(&texDesc, nullptr, tex.GetAddressOf()));
			m_pDeviceContext->UpdateSubresource(tex.Get(), 0, nullptr, img_data, width * sizeof(uint32_t), 0);
			CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D,
				forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);
			HR(m_pDevice->CreateShaderResourceView(tex.Get(), &srvDesc, res.GetAddressOf()));
			if (enableMips)
				m_pDeviceContext->GenerateMips(res.Get());

			std::string fname = std::filesystem::path(filename).filename().string();
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
			tex->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)fname.length(), fname.c_str());
#endif
		}
		stbi_image_free(img_data);
	}

	return res.Get();
}

ID3D11ShaderResourceView* TextureImporter::CreateFromMemory(std::string_view name, void* data, size_t byteWidth, bool enableMips, bool forceSRGB)
{
    XID fileID = StringToID(name);
    if (m_TextureSRVs.count(fileID))
        return m_TextureSRVs[fileID].Get();

    auto& res = m_TextureSRVs[fileID];
    int width, height, comp;
    stbi_uc* img_data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(data), (int)byteWidth, &width, &height, &comp, STBI_rgb_alpha);
    if (img_data)
    {
        CD3D11_TEXTURE2D_DESC texDesc(forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
            width, height, 1,
            enableMips ? 0 : 1,
            D3D11_BIND_SHADER_RESOURCE | (enableMips ? D3D11_BIND_RENDER_TARGET : 0),
            D3D11_USAGE_DEFAULT, 0, 1, 0,
            enableMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
        Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
        HR(m_pDevice->CreateTexture2D(&texDesc, nullptr, tex.GetAddressOf()));
        // 上传纹理数据
        m_pDeviceContext->UpdateSubresource(tex.Get(), 0, nullptr, img_data, width * sizeof(uint32_t), 0);
        CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D,
            forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);
        // 创建SRV
        HR(m_pDevice->CreateShaderResourceView(tex.Get(), &srvDesc, res.ReleaseAndGetAddressOf()));
        // 生成mipmap
        if (enableMips)
            m_pDeviceContext->GenerateMips(res.Get());
        stbi_image_free(img_data);
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
        SetDebugObjectName(res.Get(), name);
#endif
    }
    else
    {
        std::string warning = "[Warning]: TextureManager::CreateFromMemory, failed to create texture \"";
        warning += name;
        warning += "\"\n";
        OutputDebugStringA(warning.c_str());
    }
    return res.Get();

}

bool TextureImporter::AddTexture(std::string_view name, ID3D11ShaderResourceView* texture)
{
    XID nameID = StringToID(name);
    return m_TextureSRVs.try_emplace(nameID, texture).second;
}

ID3D11ShaderResourceView* TextureImporter::GetTexture(std::string_view filename)
{
    XID fileID = StringToID(filename);
    if (m_TextureSRVs.count(fileID))
        return m_TextureSRVs[fileID].Get();
    return nullptr;
}
