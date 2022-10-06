#ifndef TEXTURE_IMPORTER
#define TEXTURE_IMPORTER
#include <unordered_map>
#include <string>
#include "../Common/WinMin.h"
#include <d3d11_1.h>
#include <wrl/client.h>
#include "../Common/Base.h"

class TextureImporter:public Base
{
public:
    TextureImporter(Application* application);
    ~TextureImporter();
    TextureImporter(TextureImporter&) = delete;
    TextureImporter& operator=(const TextureImporter&) = delete;
    TextureImporter(TextureImporter&&) = default;
    TextureImporter& operator=(TextureImporter&&) = default;

    static TextureImporter& Get();
    bool Init(ID3D11Device* device);
    ID3D11ShaderResourceView* CreateTexture(std::string_view filename, bool enableMips = false, bool forceSRGB = false);
    bool AddTexture(std::string_view name, ID3D11ShaderResourceView* texture);
    ID3D11ShaderResourceView* GetTexture(std::string_view filename);

private:
    using XID = size_t;

    Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pDeviceContext;
    std::unordered_map<XID, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_TextureSRVs;
};
#endif //TEXTURE_IMPORTER