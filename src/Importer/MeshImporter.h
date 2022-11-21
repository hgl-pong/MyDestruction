#ifndef MESH_IMPORTER
#define MESH_IMPORTER
#include "../Common/WinMin.h"
#include "../Common/Geometry.h"
#include "../Common/Material.h"
#include "../Common/MeshData.h"
#include <d3d11_1.h>
#include "../Common/XUtil.h"
#include <wrl/client.h>
#include "../Common/Base.h"
namespace KG3D_Destruction {
    class DestructionMesh;
}
struct Model
{
    std::vector<Material*> materials;
    std::vector<Graphics::MeshData> meshDatas;
    DirectX::BoundingBox boundingbox;
    Geometry::MeshData meshdata;;
};


class MeshImporter:public Base
{
public:
    MeshImporter(Application*application);
    ~MeshImporter();
    MeshImporter(MeshImporter&) = delete;
    MeshImporter& operator=(const MeshImporter&) = delete;
    MeshImporter(MeshImporter&&) = default;
    MeshImporter& operator=(MeshImporter&&) = default;

    static MeshImporter& Get();
    bool Init(ID3D11Device* device);
    const Model* CreateFromFile(std::string_view filename);
    const Model* CreateFromGeometry(std::string_view name, const Geometry::MeshData& data);

    const Model* GetModel(std::string_view name) const;
    Model* GetModel(std::string_view name);
    const Model* GetModel(XID nameID) const;
    Model* GetModel(XID nameID);

private:
    Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pDeviceContext;
    std::unordered_map<size_t, Model> m_Models;
};

#endif //MESH_IMPORTER

