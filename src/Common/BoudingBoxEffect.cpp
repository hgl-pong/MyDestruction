#include "Effects.h"
#include "XUtil.h"
#include "RenderStates.h"
#include "EffectHelper.h"
#include "DXTrace.h"
#include "Vertex.h"
#include "../Importer/TextureImporter.h"
#include <wrl/client.h>
using namespace DirectX;

# pragma warning(disable: 26812)

//
// BoudingBoxEffect::Impl 需要先于BoudingBoxEffect的定义
//
namespace Graphics {
	class BoudingBoxEffect::Impl
	{

	public:
		// 必须显式指定
		Impl() {
			XMStoreFloat4x4(&m_World, XMMatrixIdentity());
			XMStoreFloat4x4(&m_View, XMMatrixIdentity());
			XMStoreFloat4x4(&m_Proj, XMMatrixIdentity());
		}
		~Impl() = default;

	public:

		std::unique_ptr<EffectHelper> m_pEffectHelper;
		std::shared_ptr<IEffectPass> m_pCurrEffectPass;

		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pVertexColorLayout;

		XMFLOAT4X4 m_View, m_Proj, m_World;
	};

	//
	// BoudingBoxEffect
	//

	namespace
	{
		// BoudingBoxEffect单例
		static BoudingBoxEffect* g_pInstance = nullptr;
	}

	BoudingBoxEffect::BoudingBoxEffect()
	{
		if (g_pInstance)
			throw std::exception("BasicEffect is a singleton!");
		g_pInstance = this;
		pImpl = std::make_unique<BoudingBoxEffect::Impl>();
	}

	BoudingBoxEffect::~BoudingBoxEffect()
	{
	}

	BoudingBoxEffect::BoudingBoxEffect(BoudingBoxEffect&& moveFrom) noexcept
	{
		pImpl.swap(moveFrom.pImpl);
	}

	BoudingBoxEffect& BoudingBoxEffect::operator=(BoudingBoxEffect&& moveFrom) noexcept
	{
		pImpl.swap(moveFrom.pImpl);
		return *this;
	}

	BoudingBoxEffect& BoudingBoxEffect::Get()
	{
		if (!g_pInstance)
			throw std::exception("BasicEffect needs an instance!");
		return *g_pInstance;
	}

	bool BoudingBoxEffect::InitAll(ID3D11Device* device)
	{
		if (!device)
			return false;

		if (!RenderStates::IsInit())
			throw std::exception("RenderStates need to be initialized first!");

		pImpl->m_pEffectHelper = std::make_unique<EffectHelper>();

		Microsoft::WRL::ComPtr<ID3DBlob> blob;
		// ******************
		// 创建顶点着色器
		//
		// 
		// 创建顶点着色器
		HR(pImpl->m_pEffectHelper->CreateShaderFromFile("CubeVS", L"Shaders\\Cube.hlsl", device,
			"CubeVS", "vs_5_0", nullptr, blob.GetAddressOf()));
		// 创建顶点布局
		HR(device->CreateInputLayout(VertexPosColor::GetInputLayout(), ARRAYSIZE(VertexPosColor::GetInputLayout()),
			blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexColorLayout.GetAddressOf()));

		HR(pImpl->m_pEffectHelper->CreateShaderFromFile("CubePS", L"Shaders\\Cube.hlsl", device, "CubePS", "ps_5_0", nullptr));

		// ******************
		// 创建通道
		//
		std::string passName = "Cube";
		EffectPassDesc passDesc;
		passDesc.nameVS = "CubeVS";
		passDesc.namePS = "CubePS";
		HR(pImpl->m_pEffectHelper->AddEffectPass(passName, device, &passDesc));
		auto pPass = pImpl->m_pEffectHelper->GetEffectPass(passName);
		pPass->SetRasterizerState(RenderStates::RSNoCull.Get());

		return true;
	}

	void BoudingBoxEffect::SetRenderDefault(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->IASetInputLayout(pImpl->m_pVertexColorLayout.Get());
		pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("Cube");
		//pImpl->m_pCurrEffectPass->SetDepthStencilState(RenderStates::DSSGreaterEqual.Get(), 0);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}

	void XM_CALLCONV BoudingBoxEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
	{
		XMStoreFloat4x4(&pImpl->m_View, W);
	}

	void XM_CALLCONV BoudingBoxEffect::SetViewMatrix(DirectX::FXMMATRIX V)
	{
		XMStoreFloat4x4(&pImpl->m_View, V);
	}

	void XM_CALLCONV BoudingBoxEffect::SetProjMatrix(DirectX::FXMMATRIX P)
	{
		XMStoreFloat4x4(&pImpl->m_Proj, P);
	}

	MeshDataInput BoudingBoxEffect::GetInputData(const Graphics::MeshData& meshData)
	{
		MeshDataInput input;
		input.pVertexBuffers = {
			meshData.m_pVertices.Get(),
			meshData.m_pColors.Get(),
		};
		input.strides = { 12, 16 };
		input.offsets = { 0, 0 };

		input.pIndexBuffer = meshData.m_pIndices.Get();

		return input;
	}


	void BoudingBoxEffect::Apply(ID3D11DeviceContext* deviceContext)
	{

		XMMATRIX WVP = XMLoadFloat4x4(&pImpl->m_World) * XMLoadFloat4x4(&pImpl->m_View) * XMLoadFloat4x4(&pImpl->m_Proj);
		pImpl->m_pEffectHelper->GetConstantBufferVariable("g_MViewProj")->SetFloatMatrix(4, 4, (const FLOAT*)&WVP);

		pImpl->m_pCurrEffectPass->Apply(deviceContext);
	}

}
