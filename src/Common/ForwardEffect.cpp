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
// ForwardEffect::Impl ��Ҫ����ForwardEffect�Ķ���
//
using namespace Graphics;
class ForwardEffect::Impl
{
public:
    // ������ʽָ��
    Impl() {}
    ~Impl() = default;

public:
    template<class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    std::unique_ptr<EffectHelper> m_pEffectHelper;

    std::shared_ptr<IEffectPass> m_pCurrEffectPass;

    ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexLayout;

    XMFLOAT4X4 m_World{}, m_View{}, m_Proj{};
    int m_CascadeLevel = 0;
    int m_DerivativeOffset = 0;
    int m_CascadeBlend = 0;
    int m_CascadeSelection = 0;
    int m_PCFKernelSize = 1;
    int m_ShadowSize = 1024;
};

//
// ForwardEffect
//

namespace
{
    // ForwardEffect����
    static ForwardEffect * g_pInstance = nullptr;
}

ForwardEffect::ForwardEffect()
{
    if (g_pInstance)
        throw std::exception("ForwardEffect is a singleton!");
    g_pInstance = this;
    pImpl = std::make_unique<ForwardEffect::Impl>();
}

ForwardEffect::~ForwardEffect()
{
}

ForwardEffect::ForwardEffect(ForwardEffect && moveFrom) noexcept
{
    pImpl.swap(moveFrom.pImpl);
}

ForwardEffect & ForwardEffect::operator=(ForwardEffect && moveFrom) noexcept
{
    pImpl.swap(moveFrom.pImpl);
    return *this;
}

ForwardEffect & ForwardEffect::Get()
{
    if (!g_pInstance)
        throw std::exception("ForwardEffect needs an instance!");
    return *g_pInstance;
}


bool ForwardEffect::InitAll(ID3D11Device * device)
{
    if (!device)
        return false;

    if (!RenderStates::IsInit())
        throw std::exception("RenderStates need to be initialized first!");

    pImpl->m_pEffectHelper = std::make_unique<EffectHelper>();

    // Ϊ�˶�ÿ����ɫ����������Ű汾����Ҫ��ͬһ���ļ������64�ְ汾����ɫ����
    
    const char* numStrs[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8"};
    D3D_SHADER_MACRO defines[] =
    {
        { "CASCADE_COUNT_FLAG", "1" },
        { "USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG", "0" },
        { "BLEND_BETWEEN_CASCADE_LAYERS_FLAG", "0" },
        { "SELECT_CASCADE_BY_INTERVAL_FLAG", "0" },
        { nullptr, nullptr }
    };

    Microsoft::WRL::ComPtr<ID3DBlob> blob;
    // ����������ɫ��
    pImpl->m_pEffectHelper->CreateShaderFromFile("GeometryVS", L"Shaders/Rendering.hlsl", device,
        "GeometryVS", "vs_5_0", defines, blob.GetAddressOf());
    // �������㲼��
    HR(device->CreateInputLayout(VertexPosNormalTex::GetInputLayout(), ARRAYSIZE(VertexPosNormalTex::GetInputLayout()),
        blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosNormalTexLayout.GetAddressOf()));

    // ǰ��λ����
    // [��������][ƫ��ƫ��][��������][����ѡ��]
    std::string psName = "0000_ForwardPS";
    std::string passName = "0000_Forward";
    EffectPassDesc passDesc;
    
    // ����ͨ��
    passDesc.nameVS = "GeometryVS";
    pImpl->m_pEffectHelper->AddEffectPass("PreZ_Forward", device, &passDesc);

    for (int cascadeCount = 1; cascadeCount <= 8; ++cascadeCount)
    {
        psName[0] = passName[0] = '0' + cascadeCount;
        defines[0].Definition = numStrs[cascadeCount];
        for (int derivativeIdx = 0; derivativeIdx < 2; ++derivativeIdx)
        {
            psName[1] = passName[1] = '0' + derivativeIdx;
            defines[1].Definition = numStrs[derivativeIdx];
            for (int blendIdx = 0; blendIdx < 2; ++blendIdx)
            {
                psName[2] = passName[2] = '0' + blendIdx;
                defines[2].Definition = numStrs[blendIdx];
                for (int intervalIdx = 0; intervalIdx < 2; ++intervalIdx)
                {
                    psName[3] = passName[3] = '0' + intervalIdx;
                    defines[3].Definition = numStrs[intervalIdx];

                    // ����������ɫ��
                    pImpl->m_pEffectHelper->CreateShaderFromFile(psName, L"Shaders/Rendering.hlsl", device, 
                        "ForwardPS", "ps_5_0", defines);

                    // ����ͨ��
                    passDesc.nameVS = "GeometryVS";
                    passDesc.namePS = psName;
                    pImpl->m_pEffectHelper->AddEffectPass(passName, device, &passDesc);
                }
            }
        }
    }
    
    


    pImpl->m_pEffectHelper->SetSamplerStateByName("g_SamplerDiffuse", RenderStates::SSAnistropicWrap16x.Get());
    pImpl->m_pEffectHelper->SetSamplerStateByName("g_SamplerShadow", RenderStates::SSShadowPCF.Get());

    // ���õ��Զ�����
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    std::string str = "ForwardEffect.VertexPosNormalTexLayout";
    pImpl->m_pVertexPosNormalTexLayout->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)str.length(), str.c_str());
#endif
    pImpl->m_pEffectHelper->SetDebugObjectName("ForwardEffect");

    return true;
}

void XM_CALLCONV ForwardEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
{
    XMStoreFloat4x4(&pImpl->m_World, W);
}

void XM_CALLCONV ForwardEffect::SetViewMatrix(DirectX::FXMMATRIX V)
{
    XMStoreFloat4x4(&pImpl->m_View, V);
}

void XM_CALLCONV ForwardEffect::SetProjMatrix(DirectX::FXMMATRIX P)
{
    XMStoreFloat4x4(&pImpl->m_Proj, P);
}

void ForwardEffect::SetMaterial(const Material* material)
{
    TextureImporter& tm = TextureImporter::Get();

    const std::string& str = material->GetTexture("$Diffuse");
    pImpl->m_pEffectHelper->SetShaderResourceByName("g_TextureDiffuse", tm.GetTexture(str));
}

MeshDataInput ForwardEffect::GetInputData(const MeshData& meshData)
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

void ForwardEffect::SetCascadeLevels(int cascadeLevels)
{
    pImpl->m_CascadeLevel = cascadeLevels;
}

void ForwardEffect::SetPCFDerivativesOffsetEnabled(bool enable)
{
    pImpl->m_DerivativeOffset = enable;
}

void ForwardEffect::SetCascadeBlendEnabled(bool enable)
{
    pImpl->m_CascadeBlend = enable;
}

void ForwardEffect::SetCascadeIntervalSelectionEnabled(bool enable)
{
    pImpl->m_CascadeSelection = enable;
}

void ForwardEffect::SetCascadeVisulization(bool enable)
{
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_VisualizeCascades")->SetSInt(enable);
}

void ForwardEffect::SetCascadeOffsets(const DirectX::XMFLOAT4 offsets[8])
{
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_CascadeOffset")->SetRaw(offsets);
}

void ForwardEffect::SetCascadeScales(const DirectX::XMFLOAT4 scales[8])
{
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_CascadeScale")->SetRaw(scales);
}

void ForwardEffect::SetCascadeFrustumsEyeSpaceDepths(const float depths[8])
{
    float depthsArray[8][4] = { {depths[0]},{depths[1]}, {depths[2]}, {depths[3]}, 
        {depths[4]}, {depths[5]}, {depths[6]}, {depths[7]} };
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_CascadeFrustumsEyeSpaceDepthsFloat")->SetRaw(depths);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_CascadeFrustumsEyeSpaceDepthsFloat4")->SetRaw(depthsArray);
}

void ForwardEffect::SetCascadeBlendArea(float blendArea)
{
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_CascadeBlendArea")->SetFloat(blendArea);
}

void ForwardEffect::SetPCFKernelSize(int size)
{
    int start = -size / 2;
    int end = size + start;
    pImpl->m_PCFKernelSize = size;
    float padding = (size / 2) / (float)pImpl->m_ShadowSize;
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_PCFBlurForLoopStart")->SetSInt(start);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_PCFBlurForLoopEnd")->SetSInt(end);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_MinBorderPadding")->SetFloat(padding);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_MaxBorderPadding")->SetFloat(1.0f - padding);
}

void ForwardEffect::SetPCFDepthOffset(float offset)
{
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ShadowBias")->SetFloat(offset);
}

void ForwardEffect::SetShadowSize(int size)
{
    pImpl->m_ShadowSize = size;
    float padding = (pImpl->m_PCFKernelSize / 2) / (float)size;
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_TexelSize")->SetFloat(1.0f / size);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_MinBorderPadding")->SetFloat(padding);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_MaxBorderPadding")->SetFloat(1.0f - padding);
}

void XM_CALLCONV ForwardEffect::SetShadowViewMatrix(DirectX::FXMMATRIX ShadowView)
{
    XMMATRIX ShadowViewT = XMMatrixTranspose(ShadowView);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ShadowView")->SetFloatMatrix(4, 4, (const float *)&ShadowViewT);
}

void ForwardEffect::SetShadowTextureArray(ID3D11ShaderResourceView* shadow)
{
    pImpl->m_pEffectHelper->SetShaderResourceByName("g_TextureShadow", shadow);
}

void ForwardEffect::SetLightDir(const DirectX::XMFLOAT3& dir)
{
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_LightDir")->SetFloatVector(3, (const float*)&dir);
}

void ForwardEffect::SetRenderDefault(ID3D11DeviceContext* deviceContext, bool reversedZ)
{
    std::string passName = "0000_Forward";
    passName[0] = '0' + pImpl->m_CascadeLevel;
    passName[1] = '0' + pImpl->m_DerivativeOffset;
    passName[2] = '0' + pImpl->m_CascadeBlend;
    passName[3] = '0' + pImpl->m_CascadeSelection;
    deviceContext->IASetInputLayout(pImpl->m_pVertexPosNormalTexLayout.Get());
    pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass(passName);
    pImpl->m_pCurrEffectPass->SetDepthStencilState(reversedZ ? RenderStates::DSSGreaterEqual.Get() : nullptr, 0);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ForwardEffect::SetRenderPreZPass(ID3D11DeviceContext* deviceContext, bool reversedZ)
{
    deviceContext->IASetInputLayout(pImpl->m_pVertexPosNormalTexLayout.Get());
    pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("PreZ_Forward");
    pImpl->m_pCurrEffectPass->SetDepthStencilState(reversedZ ? RenderStates::DSSGreaterEqual.Get() : nullptr, 0);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ForwardEffect::Apply(ID3D11DeviceContext * deviceContext)
{
    XMMATRIX W = XMLoadFloat4x4(&pImpl->m_World);
    XMMATRIX V = XMLoadFloat4x4(&pImpl->m_View);
    XMMATRIX P = XMLoadFloat4x4(&pImpl->m_Proj);

    XMMATRIX WV = W * V;
    XMMATRIX WVP = W * V * P;
    XMMATRIX WInvT = XMath::InverseTranspose(W);

    W = XMMatrixTranspose(W);
    WV = XMMatrixTranspose(WV);
    WVP = XMMatrixTranspose(WVP);
    WInvT = XMMatrixTranspose(WInvT);

    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldInvTranspose")->SetFloatMatrix(4, 4, (FLOAT*)&WInvT);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldViewProj")->SetFloatMatrix(4, 4, (FLOAT*)&WVP);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldView")->SetFloatMatrix(4, 4, (FLOAT*)&WV);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_World")->SetFloatMatrix(4, 4, (FLOAT*)&W);

    if (pImpl->m_pCurrEffectPass)
        pImpl->m_pCurrEffectPass->Apply(deviceContext);
}


