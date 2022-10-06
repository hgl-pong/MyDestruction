#include "Effects.h"
#include "XUtil.h"
#include "RenderStates.h"
#include "EffectHelper.h"
#include "DXTrace.h"
#include "Vertex.h"
#include "../Importer/TextureImporter.h"

using namespace DirectX;

# pragma warning(disable: 26812)

//
// SkyEffect::Impl ��Ҫ����SkyEffect�Ķ���
//
namespace Graphics {
    class SkyboxToneMapEffect::Impl
    {

    public:
        // ������ʽָ��
        Impl() {
            XMStoreFloat4x4(&m_View, XMMatrixIdentity());
            XMStoreFloat4x4(&m_Proj, XMMatrixIdentity());
        }
        ~Impl() = default;

    public:
        template<class T>
        using ComPtr = Microsoft::WRL::ComPtr<T>;

        std::unique_ptr<EffectHelper> m_pEffectHelper;
        std::shared_ptr<IEffectPass> m_pCurrEffectPass;

        ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexLayout;

        XMFLOAT4X4 m_View, m_Proj;
        UINT m_MsaaLevels = 1;
    };

    //
    // SkyEffect
    //

    namespace
    {
        // SkyEffect����
        static SkyboxToneMapEffect* g_pInstance = nullptr;
    }

    SkyboxToneMapEffect::SkyboxToneMapEffect()
    {
        if (g_pInstance)
            throw std::exception("BasicEffect is a singleton!");
        g_pInstance = this;
        pImpl = std::make_unique<SkyboxToneMapEffect::Impl>();
    }

    SkyboxToneMapEffect::~SkyboxToneMapEffect()
    {
    }

    SkyboxToneMapEffect::SkyboxToneMapEffect(SkyboxToneMapEffect&& moveFrom) noexcept
    {
        pImpl.swap(moveFrom.pImpl);
    }

    SkyboxToneMapEffect& SkyboxToneMapEffect::operator=(SkyboxToneMapEffect&& moveFrom) noexcept
    {
        pImpl.swap(moveFrom.pImpl);
        return *this;
    }

    SkyboxToneMapEffect& SkyboxToneMapEffect::Get()
    {
        if (!g_pInstance)
            throw std::exception("BasicEffect needs an instance!");
        return *g_pInstance;
    }

    bool SkyboxToneMapEffect::InitAll(ID3D11Device* device)
    {
        if (!device)
            return false;

        if (!RenderStates::IsInit())
            throw std::exception("RenderStates need to be initialized first!");

        pImpl->m_pEffectHelper = std::make_unique<EffectHelper>();

        Microsoft::WRL::ComPtr<ID3DBlob> blob;
        D3D_SHADER_MACRO defines[] = {
            {"MSAA_SAMPLES", "1"},
            {nullptr, nullptr}
        };

        // ******************
        // ����������ɫ��
        //

        HR(pImpl->m_pEffectHelper->CreateShaderFromFile("SkyboxVS", L"Shaders\\SkyboxToneMap.hlsl", device,
            "SkyboxVS", "vs_5_0", defines, blob.GetAddressOf()));
        // �������㲼��
        HR(device->CreateInputLayout(VertexPosNormalTex::GetInputLayout(), ARRAYSIZE(VertexPosNormalTex::GetInputLayout()),
            blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosNormalTexLayout.GetAddressOf()));

        int msaaSamples = 1;
        while (msaaSamples <= 8)
        {
            // ******************
            // ����������ɫ��
            //
            std::string msaaSamplesStr = std::to_string(msaaSamples);
            defines[0].Definition = msaaSamplesStr.c_str();
            std::string shaderName = "Skybox_" + msaaSamplesStr + "xMSAA_PS";
            HR(pImpl->m_pEffectHelper->CreateShaderFromFile(shaderName, L"Shaders\\SkyboxToneMap.hlsl", device, "SkyboxPS", "ps_5_0", defines));

            // ******************
            // ����ͨ��
            //
            std::string passName = "Skybox_" + msaaSamplesStr + "xMSAA";
            EffectPassDesc passDesc;
            passDesc.nameVS = "SkyboxVS";
            passDesc.namePS = shaderName.c_str();
            HR(pImpl->m_pEffectHelper->AddEffectPass(passName, device, &passDesc));
            {
                auto pPass = pImpl->m_pEffectHelper->GetEffectPass(passName);
                pPass->SetRasterizerState(RenderStates::RSNoCull.Get());
            }


            msaaSamples <<= 1;
        }

        pImpl->m_pEffectHelper->SetSamplerStateByName("g_SamplerDiffuse", RenderStates::SSAnistropicWrap16x.Get());

        // ���õ��Զ�����
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
        pImpl->m_pVertexPosNormalTexLayout->SetPrivateData(WKPDID_D3DDebugObjectName, LEN_AND_STR("SkyEffect.VertexPosNormalTexLayout"));
#endif
        pImpl->m_pEffectHelper->SetDebugObjectName("SkyboxEffect");

        return true;
    }

    void SkyboxToneMapEffect::SetRenderDefault(ID3D11DeviceContext* deviceContext)
    {
        deviceContext->IASetInputLayout(pImpl->m_pVertexPosNormalTexLayout.Get());
        std::string passName = "Skybox_" + std::to_string(pImpl->m_MsaaLevels) + "xMSAA";
        pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass(passName);
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void XM_CALLCONV SkyboxToneMapEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
    {
        UNREFERENCED_PARAMETER(W);
    }

    void XM_CALLCONV SkyboxToneMapEffect::SetViewMatrix(DirectX::FXMMATRIX V)
    {
        XMStoreFloat4x4(&pImpl->m_View, V);
    }

    void XM_CALLCONV SkyboxToneMapEffect::SetProjMatrix(DirectX::FXMMATRIX P)
    {
        XMStoreFloat4x4(&pImpl->m_Proj, P);
    }

    MeshDataInput SkyboxToneMapEffect::GetInputData(const Graphics::MeshData& meshData)
    {
        MeshDataInput input;
        input.pVertexBuffers = {
            meshData.m_pVertices.Get(),
            meshData.m_pNormals.Get(),
            meshData.m_pTexcoordArrays.empty() ? nullptr : meshData.m_pTexcoordArrays[0].Get()
        };
        input.strides = { 12, 12, 8 };
        input.offsets = { 0, 0, 0 };

        input.pIndexBuffer = meshData.m_pIndices.Get();
        input.indexCount = meshData.m_IndexCount;

        return input;
    }

    void SkyboxToneMapEffect::SetSkyboxTexture(ID3D11ShaderResourceView* skyboxTexture)
    {
        pImpl->m_pEffectHelper->SetShaderResourceByName("g_SkyboxTexture", skyboxTexture);
    }

    void SkyboxToneMapEffect::SetDepthTexture(ID3D11ShaderResourceView* depthTexture)
    {
        pImpl->m_pEffectHelper->SetShaderResourceByName("g_DepthTexture", depthTexture);
    }

    void SkyboxToneMapEffect::SetLitTexture(ID3D11ShaderResourceView* litTexture)
    {
        pImpl->m_pEffectHelper->SetShaderResourceByName("g_LitTexture", litTexture);
    }

    void SkyboxToneMapEffect::SetMsaaSamples(UINT msaaSamples)
    {
        pImpl->m_MsaaLevels = msaaSamples;
    }

    void SkyboxToneMapEffect::Apply(ID3D11DeviceContext* deviceContext)
    {
        XMMATRIX VP = XMLoadFloat4x4(&pImpl->m_View) * XMLoadFloat4x4(&pImpl->m_Proj);
        VP = XMMatrixTranspose(VP);
        pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ViewProj")->SetFloatMatrix(4, 4, (const FLOAT*)&VP);

        pImpl->m_pCurrEffectPass->Apply(deviceContext);
    }

}