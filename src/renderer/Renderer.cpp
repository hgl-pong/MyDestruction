#include "Renderer.h"
#include "../Common/Application.h"
#include "../Common/XUtil.h"
#include "../Common/DXTrace.h"
#include "../Common/XUtil.h"
#include <Windows.h>
#include <wrl/client.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include "../FRenderMesh.h"
#include "../FWireMesh.h"
using namespace DirectX;
namespace
{
	Graphics::Renderer* s_pInstance = nullptr;
}
namespace Graphics {
	Renderer::Renderer(Application* application) :
		m_ClientWidth(920),
		m_ClientHeight(540),
		m_Enable4xMsaa(true),		// 4xMSAA
		m_4xMsaaQuality(0),
		m_pd3dDevice(nullptr),
		m_pd3dImmediateContext(nullptr),
		m_pSwapChain(nullptr),
		m_pRenderTargetView(nullptr),
		m_pDepthStencilView(nullptr),
		use_skybox(true),
		m_pForwardEffect(std::make_unique<ForwardEffect>()),
		m_pSkyboxEffect(std::make_unique<SkyboxToneMapEffect>()),
		m_pBoundingBoxEffect(std::make_unique<BoudingBoxEffect>()),
		m_pShadowEffect(std::make_unique<ShadowEffect>()),
		m_pHitPos(new Graphics::MeshData()),
		m_CSManager()
	{
		name = "Renderer";
		app = application;
		ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));
		if (s_pInstance)
			throw std::exception("Renderer is a singleton!");
		s_pInstance = this;
	}

	Renderer::~Renderer() {
		// 恢复所有默认设定
		if (m_pd3dImmediateContext)
			m_pd3dImmediateContext->ClearState();
	}

	Renderer* Renderer::Get()
	{
		if (!s_pInstance)
			throw std::exception("Renderer needs an instance!");
		return s_pInstance;
	}

	bool Renderer::isUseSkyBox() {
		return use_skybox;
	}


	void Renderer::setClientWidth(int width) {
		m_ClientWidth = width;
	}

	void Renderer::setClientHeight(int height) {
		m_ClientHeight = height;
	}

	int* Renderer::getClientHeight() {
		return &m_ClientHeight;
	}

	int* Renderer::getClientWidth() {
		return &m_ClientWidth;
	}


	ID3D11Device* Renderer::getD3DDevice() {
		return m_pd3dDevice.Get();
	}

	ID3D11DeviceContext* Renderer::getD3DDeviceContext() {
		return m_pd3dImmediateContext.Get();
	}

	bool Renderer::is3dDeviceNotNull() {
		if (m_pd3dDevice)
			return true;
		else
			return false;
	}

	bool Renderer::Init() {

		app->m_GpuTimer_Shadow.Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());
		app->m_GpuTimer_Lighting.Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());
		app->m_GpuTimer_Skybox.Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());
		app->m_GpuTimer_BoundingBox.Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());
		// 务必先初始化所有渲染状态，以供下面的特效使用
		RenderStates::InitAll(m_pd3dDevice.Get());

		if (!m_pForwardEffect->InitAll(m_pd3dDevice.Get()))
			return false;

		if (!m_pShadowEffect->InitAll(m_pd3dDevice.Get()))
			return false;

		if (!m_pSkyboxEffect->InitAll(m_pd3dDevice.Get()))
			return false;

		if (!m_pBoundingBoxEffect->InitAll(m_pd3dDevice.Get()))
			return false;
		Logger::Debug(name, "Effect初始化成功!");
		if (!InitResource())
			return false;

		return true;
	}

	void Renderer::DrawScene() {
		assert(m_pd3dImmediateContext);
		assert(m_pSwapChain);

		// 创建后备缓冲区的渲染目标视图
		if (m_FrameCount < m_BackBufferCount)
		{
			ComPtr<ID3D11Texture2D> pBackBuffer;
			m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
			CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
			m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, m_pRenderTargetViews[m_FrameCount].ReleaseAndGetAddressOf());
		}
		//
		// 场景渲染部分
		//
		RenderSkyboxAndToneMap();
		RenderShadowForAllCascades();
		RenderForward();
		RenderBoundingBoxes();


		D3D11_VIEWPORT vp = app->m_pViewerCamera->GetViewPort();
		m_pd3dImmediateContext->RSSetViewports(1, &vp);

		ID3D11RenderTargetView* pRTVs[] = { GetBackBufferRTV() };
		m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);
	}

	bool Renderer::AddRenderMesh(MeshData* data)
	{
		return m_RenderMesh.emplace(data).second;
	}

	bool Renderer::RemoveRenderMesh(MeshData* data)
	{
		if (!data)
			return false;
		auto it = m_RenderMesh.find(data);
		if (it != m_RenderMesh.end())
			m_RenderMesh.erase(data);
		return true;
	}

	bool Renderer::AddVoroMesh(MeshData* data)
	{
		return m_VoroMesh.emplace(data).second;
	}

	bool Renderer::RemoveVoroMesh(MeshData* data)
	{
		auto it = m_VoroMesh.find(data);
		if (it != m_VoroMesh.end())
			m_VoroMesh.erase(data);
		return true;
	}

	bool Renderer::AddRenderBoundingBox(size_t mesh_id, BoundingBox* box)
	{
		return true;
	}

	bool Renderer::RemoveRenderBoundingBox(size_t mesh_id)
	{
		return true;
	}

	Graphics::MeshData* Renderer::CreateVoroMeshData(FWireMesh* mesh)
	{
		Graphics::MeshData* meshData = new Graphics::MeshData();

		CD3D11_BUFFER_DESC bufferDesc(0, D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };
		bufferDesc.ByteWidth = sizeof(XMFLOAT3)* mesh->vertices.size();
		initData.pSysMem = mesh->vertices.data();
		HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, meshData->m_pVertices.GetAddressOf()));

		bufferDesc.ByteWidth = sizeof(XMFLOAT4) * mesh->colors.size();
		initData.pSysMem = mesh->colors.data();
		HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, meshData->m_pColors.GetAddressOf()));

		// 设置索引缓冲区描述
		bufferDesc = CD3D11_BUFFER_DESC(sizeof(uint32_t) * mesh->indices.size(), D3D11_BIND_INDEX_BUFFER);

		// 新建索引缓冲区
		initData.pSysMem = mesh->indices.data();
		HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, meshData->m_pIndices.GetAddressOf()));

		meshData->m_VertexCount = mesh->vertices.size();
		meshData->m_IndexCount = mesh->indices.size();
		return meshData;
	}

	Graphics::MeshData* Renderer::CreateRenderMeshData(FRenderMesh* mesh)
	{
		Graphics::MeshData* meshData = new Graphics::MeshData();
		meshData->m_pTexcoordArrays.resize(1);

		CD3D11_BUFFER_DESC bufferDesc(0, D3D11_BIND_VERTEX_BUFFER);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };
		bufferDesc.ByteWidth = sizeof(XMFLOAT3) * mesh->m_Vertices.size();
		initData.pSysMem = mesh->m_Vertices.data();
		HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, meshData->m_pVertices.GetAddressOf()));

		bufferDesc.ByteWidth = sizeof(XMFLOAT3) * mesh->m_Normals.size();
		initData.pSysMem = mesh->m_Normals.data();
		HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, meshData->m_pNormals.GetAddressOf()));

		bufferDesc.ByteWidth = sizeof(XMFLOAT2) * mesh->m_UVs.size();
		initData.pSysMem = mesh->m_UVs.data();
		HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, meshData->m_pTexcoordArrays[0].GetAddressOf()));
		// 设置索引缓冲区描述
		bufferDesc = CD3D11_BUFFER_DESC(sizeof(uint32_t) * mesh->m_Indices.size(), D3D11_BIND_INDEX_BUFFER);

		// 新建索引缓冲区
		initData.pSysMem = mesh->m_Indices.data();
		HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, meshData->m_pIndices.GetAddressOf()));

		meshData->m_pMaterial = mesh->m_pMaterial;
		meshData->m_VertexCount = mesh->m_Vertices.size();
		meshData->m_IndexCount = mesh->m_Indices.size();
		return meshData;
	}

	void Renderer::SwapChainPresent() {
		HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));
	}


	void Renderer::UpdateScene() {

		if (need_gpu_timer_reset)
		{
			app->m_GpuTimer_Lighting.Reset(m_pd3dImmediateContext.Get());
			app->m_GpuTimer_Shadow.Reset(m_pd3dImmediateContext.Get());
			app->m_GpuTimer_Skybox.Reset(m_pd3dImmediateContext.Get());
			app->m_GpuTimer_BoundingBox.Reset(m_pd3dImmediateContext.Get());
		}

		if (m_CSManager.m_SelectedCamera == CameraSelection::CameraSelection_Eye)
		{
			// 注意：反向Z
			m_pForwardEffect->SetViewMatrix(app->m_pViewerCamera->GetViewMatrixXM());
			m_pForwardEffect->SetProjMatrix(app->m_pViewerCamera->GetProjMatrixXM(true));
			m_pBoundingBoxEffect->SetViewMatrix(app->m_pViewerCamera->GetViewMatrixXM());
			m_pBoundingBoxEffect->SetProjMatrix(app->m_pViewerCamera->GetProjMatrixXM(true));
			m_pSkyboxEffect->SetViewMatrix(app->m_pViewerCamera->GetViewMatrixXM());
			m_pSkyboxEffect->SetProjMatrix(app->m_pViewerCamera->GetProjMatrixXM(true));

		}
		else if (m_CSManager.m_SelectedCamera == CameraSelection::CameraSelection_Light)
		{
			// 注意：反向Z
			m_pForwardEffect->SetViewMatrix(app->m_pLightCamera->GetViewMatrixXM());
			m_pForwardEffect->SetProjMatrix(app->m_pLightCamera->GetProjMatrixXM(true));
			m_pBoundingBoxEffect->SetViewMatrix(app->m_pLightCamera->GetViewMatrixXM());
			m_pBoundingBoxEffect->SetProjMatrix(app->m_pLightCamera->GetProjMatrixXM(true));
			m_pSkyboxEffect->SetViewMatrix(app->m_pLightCamera->GetViewMatrixXM());
			m_pSkyboxEffect->SetProjMatrix(app->m_pLightCamera->GetProjMatrixXM(true));
		}
		else
		{
			// 注意：反向Z
			XMMATRIX ShadowProjRZ = m_CSManager.GetShadowProjectionXM(
				static_cast<int>(m_CSManager.m_SelectedCamera) - 2);
			ShadowProjRZ.r[2] *= g_XMNegateZ.v;
			ShadowProjRZ.r[3] = XMVectorSetZ(ShadowProjRZ.r[3], 1.0f - XMVectorGetZ(ShadowProjRZ.r[3]));

			m_pForwardEffect->SetViewMatrix(app->m_pLightCamera->GetViewMatrixXM());
			m_pForwardEffect->SetProjMatrix(ShadowProjRZ);
			m_pBoundingBoxEffect->SetViewMatrix(app->m_pLightCamera->GetViewMatrixXM());
			m_pBoundingBoxEffect->SetProjMatrix(ShadowProjRZ);
			m_pSkyboxEffect->SetViewMatrix(app->m_pLightCamera->GetViewMatrixXM());
			m_pSkyboxEffect->SetProjMatrix(ShadowProjRZ);
		}

		m_pShadowEffect->SetViewMatrix(app->m_pLightCamera->GetViewMatrixXM());
		//m_CSManager.UpdateFrame(*app->m_pViewerCamera, *app->m_pLightCamera, app->m_pBlaster->ground->m_pMeshData.m_BoundingBox);
		m_pForwardEffect->SetLightDir(app->m_pLightCamera->GetLookAxis());
	}


	bool Renderer::InitResource() {
		// ******************
		// 初始化特效
		//

		m_pForwardEffect->SetViewMatrix(app->m_pViewerCamera->GetViewMatrixXM());
		m_pForwardEffect->SetProjMatrix(app->m_pViewerCamera->GetProjMatrixXM(true));
		m_pForwardEffect->SetPCFKernelSize(m_CSManager.m_PCFKernelSize);
		m_pForwardEffect->SetPCFDepthOffset(m_CSManager.m_PCFDepthOffset);
		m_pForwardEffect->SetShadowSize(m_CSManager.m_ShadowSize);
		m_pForwardEffect->SetCascadeBlendEnabled(m_CSManager.m_BlendBetweenCascades);
		m_pForwardEffect->SetCascadeBlendArea(m_CSManager.m_BlendBetweenCascadesRange);
		m_pForwardEffect->SetCascadeLevels(m_CSManager.m_CascadeLevels);
		m_pForwardEffect->SetCascadeIntervalSelectionEnabled(static_cast<bool>(m_CSManager.m_SelectedCascadeSelection));

		m_pForwardEffect->SetPCFDerivativesOffsetEnabled(m_CSManager.m_DerivativeBasedOffset);

		m_pShadowEffect->SetViewMatrix(app->m_pLightCamera->GetViewMatrixXM());
		m_pSkyboxEffect->SetMsaaSamples(1);

		app->m_pMeshImporter->CreateFromGeometry("skyboxCube", Geometry::CreateBox());
		m_Skybox.SetModel(app->m_pMeshImporter->GetModel("skyboxCube"));

		Geometry::MeshData groundData = Geometry::CreatePlane(60, 60, 25, 25);
		Graphics::MeshData* ground = new Graphics::MeshData();;
		*ground = app->m_pMeshImporter->CreateFromGeometry("ground", groundData)->meshData;
		ground->m_pMaterial = app->m_pMaterialManager->createMaterial("..\\Texture\\floor.dds");
		ground->m_Transform.SetPosition(0, 0, 0);
		BoundingBox::CreateFromPoints(ground->m_BoundingBox, 4,
			groundData.vertices.data(), sizeof(XMFLOAT3));
		AddRenderMesh(ground);

		// ******************
		// 初始化纹理
		//
		m_pTextureCubeSRV = app->m_pTextureImporter->CreateTexture("..\\Texture\\Clouds.dds");


		// ******************
		// 初始化阴影
		//
		m_CSManager.InitResource(m_pd3dDevice.Get());
		return true;
	}

	bool Renderer::InitDirect3D()
	{
		HRESULT hr = S_OK;

		// 创建D3D设备 和 D3D设备上下文
		UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		// 驱动类型数组
		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);

		// 特性等级数组
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);

		D3D_FEATURE_LEVEL featureLevel;
		D3D_DRIVER_TYPE d3dDriverType;
		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			d3dDriverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDevice(nullptr, d3dDriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, m_pd3dDevice.GetAddressOf(), &featureLevel, m_pd3dImmediateContext.GetAddressOf());

			if (hr == E_INVALIDARG)
			{
				// Direct3D 11.0 的API不承认D3D_FEATURE_LEVEL_11_1，所以我们需要尝试特性等级11.0以及以下的版本
				hr = D3D11CreateDevice(nullptr, d3dDriverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
					D3D11_SDK_VERSION, m_pd3dDevice.GetAddressOf(), &featureLevel, m_pd3dImmediateContext.GetAddressOf());
			}

			if (SUCCEEDED(hr))
				break;
		}

		if (FAILED(hr))
		{
			MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
			return false;
		}

		// 检测是否支持特性等级11.0或11.1
		if (featureLevel != D3D_FEATURE_LEVEL_11_0 && featureLevel != D3D_FEATURE_LEVEL_11_1)
		{
			MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
			return false;
		}

		// 检测 MSAA支持的质量等级
		UINT quality;
		m_pd3dDevice->CheckMultisampleQualityLevels(
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 4, &quality);
		assert(quality > 0);




		ComPtr<IDXGIDevice> dxgiDevice = nullptr;
		ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;
		ComPtr<IDXGIFactory1> dxgiFactory1 = nullptr;	// D3D11.0(包含DXGI1.1)的接口类
		ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr;	// D3D11.1(包含DXGI1.2)特有的接口类

		// 为了正确创建 DXGI交换链，首先我们需要获取创建 D3D设备 的 DXGI工厂，否则会引发报错：
		// "IDXGIFactory::CreateSwapChain: This function is being called with a device from a different IDXGIFactory."
		HR(m_pd3dDevice.As(&dxgiDevice));
		HR(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));
		HR(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory1.GetAddressOf())));

		// 查看该对象是否包含IDXGIFactory2接口
		hr = dxgiFactory1.As(&dxgiFactory2);
		// 如果包含，则说明支持D3D11.1
		if (dxgiFactory2 != nullptr)
		{
			HR(m_pd3dDevice.As(&m_pd3dDevice1));
			HR(m_pd3dImmediateContext.As(&m_pd3dImmediateContext1));
			// 填充各种结构体用以描述交换链
			DXGI_SWAP_CHAIN_DESC1 sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.Width = m_ClientWidth;
			sd.Height = m_ClientHeight;
			sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
#if _WIN32_WINNT >= _WIN32_WINNT_WIN10
			m_BackBufferCount = 2;
			sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			m_IsDxgiFlipModel = true;
#else
			m_BackBufferCount = 1;
			sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			sd.Flags = 0;
#endif
			sd.BufferCount = m_BackBufferCount;


			DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd;
			fd.RefreshRate.Numerator = 60;
			fd.RefreshRate.Denominator = 1;
			fd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			fd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			fd.Windowed = TRUE;
			// 为当前窗口创建交换链
			HR(dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice.Get(), app->m_hMainWnd, &sd, &fd, nullptr, m_pSwapChain1.GetAddressOf()));
			HR(m_pSwapChain1.As(&m_pSwapChain));
		}
		else
		{
			// 填充DXGI_SWAP_CHAIN_DESC用以描述交换链
			DXGI_SWAP_CHAIN_DESC sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.BufferDesc.Width = m_ClientWidth;
			sd.BufferDesc.Height = m_ClientHeight;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = 1;
			sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			sd.OutputWindow = app->m_hMainWnd;
			sd.Windowed = TRUE;
			sd.Flags = 0;
			m_BackBufferCount = 1;
			HR(dxgiFactory1->CreateSwapChain(m_pd3dDevice.Get(), &sd, m_pSwapChain.GetAddressOf()));
		}

		// 可以禁止alt+enter全屏
		dxgiFactory1->MakeWindowAssociation(app->m_hMainWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

		// 设置调试对象名
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
		m_pd3dImmediateContext->SetPrivateData(WKPDID_D3DDebugObjectName, LEN_AND_STR("ImmediateContext"));
		m_pSwapChain->SetPrivateData(WKPDID_D3DDebugObjectName, LEN_AND_STR("SwapChain"));
#endif

		// 每当窗口被重新调整大小的时候，都需要调用这个OnResize函数。现在调用
		// 以避免代码重复
		OnResize();

		return true;
	}

	void Renderer::DrawMeshQueue(IEffect* effect) {
		for (auto it = m_RenderMesh.begin(); it != m_RenderMesh.end(); it++)
			DrawMesh(*it, effect);
	}
	bool Renderer::DrawMesh(MeshData* mesh, IEffect* effect) {
			IEffectMeshData* pEffectMeshData = dynamic_cast<IEffectMeshData*>(effect);

			IEffectMaterial* pEffectMaterial = dynamic_cast<IEffectMaterial*>(effect);
			if (pEffectMaterial) {
				if (mesh->m_pMaterial)
					pEffectMaterial->SetMaterial(mesh->m_pMaterial);
			}
			IEffectTransform* pEffectTransform = dynamic_cast<IEffectTransform*>(effect);
			if (pEffectTransform)
				pEffectTransform->SetWorldMatrix(mesh->m_Transform.GetLocalToWorldMatrixXM());

			effect->Apply(m_pd3dImmediateContext.Get());

			MeshDataInput input = pEffectMeshData->GetInputData(*mesh);
			{
				m_pd3dImmediateContext->IASetVertexBuffers(0, (uint32_t)input.pVertexBuffers.size(),
					input.pVertexBuffers.data(), input.strides.data(), input.offsets.data());
				m_pd3dImmediateContext->IASetIndexBuffer(input.pIndexBuffer,  DXGI_FORMAT_R32_UINT , 0);

				m_pd3dImmediateContext->DrawIndexed(input.indexCount, 0, 0);
			}
		
		return true;
	}

	bool Renderer::DrawVoroMesh(MeshData* mesh)
	{
		m_pBoundingBoxEffect->SetWorldMatrix(mesh->m_Transform.GetLocalToWorldMatrixXM());

		MeshDataInput input = m_pBoundingBoxEffect->GetInputData(*mesh);
		// 输入装配阶段的顶点缓冲区设置
		m_pBoundingBoxEffect->Apply(m_pd3dImmediateContext.Get());
		m_pd3dImmediateContext->IASetIndexBuffer(input.pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		m_pd3dImmediateContext->IASetVertexBuffers(0, 2,
			input.pVertexBuffers.data(), input.strides.data(), input.offsets.data());

		// 绘制立方体
		m_pd3dImmediateContext->DrawIndexed(input.indexCount, 0, 0);
		return true;
	}

	void Renderer::OnResize()
	{
		assert(m_pd3dImmediateContext);
		assert(m_pd3dDevice);
		assert(m_pSwapChain);

		if (m_pd3dDevice1 != nullptr)
		{
			assert(m_pd3dImmediateContext1);
			assert(m_pd3dDevice1);
			assert(m_pSwapChain1);
		}

		// 重设交换链并且重新创建渲染目标视图
		for (UINT i = 0; i < m_BackBufferCount; ++i)
			m_pRenderTargetViews[i].Reset();
		HR(m_pSwapChain->ResizeBuffers(m_BackBufferCount, m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM,
			m_IsDxgiFlipModel ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0));
		m_FrameCount = 0;

		DXGI_SAMPLE_DESC sampleDesc{};
		sampleDesc.Count = m_MsaaSamples;
		sampleDesc.Quality = 0;
		m_pLitBuffer = std::make_unique<Texture2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, sampleDesc);
		m_pDepthBuffer = std::make_unique<Depth2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight,
			D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, sampleDesc);

		// 摄像机变更显示
		if (app->m_pViewerCamera != nullptr)
		{
			app->m_pViewerCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 300.0f);
			app->m_pViewerCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
			// 注意：反向Z
			m_pForwardEffect->SetProjMatrix(app->m_pViewerCamera->GetProjMatrixXM(true));
			m_pBoundingBoxEffect->SetProjMatrix(app->m_pViewerCamera->GetProjMatrixXM(true));
		}

		m_pSceneBuffer = std::make_unique<Texture2D>(m_pd3dDevice.Get(), (float)m_ClientWidth, (float)m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

	}

	float Renderer::AspectRatio()const
	{
		return static_cast<float>(m_ClientWidth) / (m_ClientHeight);
	}



	void Renderer::RenderShadowForAllCascades()
	{
		app->m_GpuTimer_Shadow.Start();
		{
			m_pShadowEffect->SetRenderDefault(m_pd3dImmediateContext.Get());
			D3D11_VIEWPORT vp = m_CSManager.GetShadowViewport();
			m_pd3dImmediateContext->RSSetViewports(1, &vp);

			for (size_t cascadeIdx = 0; cascadeIdx < m_CSManager.m_CascadeLevels; ++cascadeIdx)
			{
				ID3D11RenderTargetView* nullRTV = nullptr;
				ID3D11DepthStencilView* depthDSV = m_CSManager.GetCascadeDepthStencilView(cascadeIdx);
				m_pd3dImmediateContext->ClearDepthStencilView(depthDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
				m_pd3dImmediateContext->OMSetRenderTargets(1, &nullRTV, depthDSV);

				XMMATRIX shadowProj = m_CSManager.GetShadowProjectionXM(cascadeIdx);
				m_pShadowEffect->SetProjMatrix(shadowProj);

				// 更新物体与投影立方体的裁剪
				BoundingOrientedBox obb = m_CSManager.GetShadowOBB(cascadeIdx);
				obb.Transform(obb, app->m_pLightCamera->GetLocalToWorldMatrixXM());
				DrawMeshQueue(m_pShadowEffect.get());
			}
		}
		app->m_GpuTimer_Shadow.Stop();
	}

	void Renderer::RenderForward()
	{
		app->m_GpuTimer_Lighting.Start();
		{
			float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_pd3dImmediateContext->ClearRenderTargetView(m_pLitBuffer->GetRenderTarget(), black);
			// 注意：反向Z
			m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthBuffer->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);

			D3D11_VIEWPORT viewport = app->m_pViewerCamera->GetViewPort();
			BoundingFrustum frustum;
			if (m_CSManager.m_SelectedCamera == CameraSelection::CameraSelection_Eye)
			{
				BoundingFrustum::CreateFromMatrix(frustum, app->m_pViewerCamera->GetProjMatrixXM());
				frustum.Transform(frustum, app->m_pViewerCamera->GetLocalToWorldMatrixXM());
				//app->m_pBlaster->m_pBlastScene->GetActor(StringToID("..\\Model\\Rock\\RockFlatLong_Set.obj"))->FrustumCulling(frustum);
			}
			else if (m_CSManager.m_SelectedCamera == CameraSelection::CameraSelection_Light)
			{
				BoundingFrustum::CreateFromMatrix(frustum, app->m_pLightCamera->GetProjMatrixXM());
				frustum.Transform(frustum, app->m_pLightCamera->GetLocalToWorldMatrixXM());
				//app->m_pBlaster->m_pBlastScene->GetActor(StringToID("..\\Model\\Rock\\RockFlatLong_Set.obj"))->FrustumCulling(frustum);
			}
			else
			{
				BoundingOrientedBox bbox = m_CSManager.GetShadowOBB(
					static_cast<int>(m_CSManager.m_SelectedCamera) - 2);
				bbox.Transform(bbox, app->m_pLightCamera->GetLocalToWorldMatrixXM());
				//app->m_pBlaster->m_pBlastScene->GetActor(StringToID("..\\Model\\Rock\\RockFlatLong_Set.obj"))->CubeCulling(bbox);
				viewport.Width = (float)(std::min)(m_ClientHeight, m_ClientWidth);
				viewport.Height = viewport.Width;
			}
			m_pd3dImmediateContext->RSSetViewports(1, &viewport);

			// 正常绘制
			ID3D11RenderTargetView* pRTVs[1] = { m_pLitBuffer->GetRenderTarget() };
			m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthBuffer->GetDepthStencil());


			m_pForwardEffect->SetCascadeFrustumsEyeSpaceDepths(m_CSManager.GetCascadePartitions());

			// 将NDC空间 [-1, +1]^2 变换到纹理坐标空间 [0, 1]^2
			static XMMATRIX T(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, -0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.5f, 0.5f, 0.0f, 1.0f);
			XMFLOAT4 scales[8]{};
			XMFLOAT4 offsets[8]{};
			for (size_t cascadeIndex = 0; cascadeIndex < m_CSManager.m_CascadeLevels; ++cascadeIndex)
			{
				XMMATRIX ShadowTexture = m_CSManager.GetShadowProjectionXM(cascadeIndex) * T;
				scales[cascadeIndex].x = XMVectorGetX(ShadowTexture.r[0]);
				scales[cascadeIndex].y = XMVectorGetY(ShadowTexture.r[1]);
				scales[cascadeIndex].z = XMVectorGetZ(ShadowTexture.r[2]);
				scales[cascadeIndex].w = 1.0f;
				XMStoreFloat3((XMFLOAT3*)(offsets + cascadeIndex), ShadowTexture.r[3]);
			}
			m_pForwardEffect->SetCascadeOffsets(offsets);
			m_pForwardEffect->SetCascadeScales(scales);
			m_pForwardEffect->SetShadowViewMatrix(app->m_pLightCamera->GetViewMatrixXM());
			m_pForwardEffect->SetShadowTextureArray(m_CSManager.GetCascadesOutput());
			// 注意：反向Z
			m_pForwardEffect->SetRenderDefault(m_pd3dImmediateContext.Get(), true);

			DrawMeshQueue(m_pForwardEffect.get());
			// 清除绑定
			m_pd3dImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
			m_pForwardEffect->SetShadowTextureArray(nullptr);
			m_pForwardEffect->Apply(m_pd3dImmediateContext.Get());
		}
		app->m_GpuTimer_Lighting.Stop();
	}


	void Renderer::RenderSkyboxAndToneMap()
	{
		app->m_GpuTimer_Skybox.Start();
		{
			m_pd3dImmediateContext->ClearRenderTargetView(GetBackBufferRTV(), DirectX::Colors::Cornsilk);
			D3D11_VIEWPORT skyboxViewport = app->m_pViewerCamera->GetViewPort();
			skyboxViewport.MinDepth = 1.0f;
			skyboxViewport.MaxDepth = 1.0f;
			m_pd3dImmediateContext->RSSetViewports(1, &skyboxViewport);

			m_pSkyboxEffect->SetRenderDefault(m_pd3dImmediateContext.Get());
			m_pSkyboxEffect->SetSkyboxTexture(m_pTextureCubeSRV.Get());
			m_pSkyboxEffect->SetLitTexture(m_pLitBuffer->GetShaderResource());
			m_pSkyboxEffect->SetDepthTexture(m_pDepthBuffer->GetShaderResource());

			// 由于全屏绘制，不需要用到深度缓冲区，也就不需要清空后备缓冲区了
			ID3D11RenderTargetView* pRTVs[] = { GetBackBufferRTV() };
			m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);
			m_Skybox.Draw(m_pd3dImmediateContext.Get(), m_pSkyboxEffect.get());

			// 清除状态
			m_pd3dImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
			m_pSkyboxEffect->SetLitTexture(nullptr);
			m_pSkyboxEffect->SetDepthTexture(nullptr);
			m_pSkyboxEffect->Apply(m_pd3dImmediateContext.Get());
		}
		app->m_GpuTimer_Skybox.Stop();
	}



	void Renderer::RenderBoundingBoxes() {
			app->m_GpuTimer_BoundingBox.Start();
			{
				// m_pd3dImmediateContext->ClearRenderTargetView(m_pLitBuffer->GetRenderTarget(), DirectX::Colors::Black);
				// m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthBuffer->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);
				D3D11_VIEWPORT Viewport = app->m_pViewerCamera->GetViewPort();
				m_pd3dImmediateContext->RSSetViewports(1, &Viewport);
				m_pBoundingBoxEffect->SetRenderDefault(m_pd3dImmediateContext.Get());
				ID3D11RenderTargetView* pRTVs[] = { m_pLitBuffer->GetRenderTarget() };
				m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthBuffer->GetDepthStencil());
				// m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);
				for (auto mesh : m_VoroMesh) {
					DrawVoroMesh(mesh);	
					//DrawBoundingBox(*mesh);
				}

				// 清除状态
				m_pd3dImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
			}
			app->m_GpuTimer_BoundingBox.Stop();
	}

	bool Renderer::DrawBoundingBox(MeshData& mesh) {
		//m_pBoundingBoxEffect->SetRenderDefault(m_pd3dImmediateContext.Get());
		m_pBoundingBoxEffect->SetWorldMatrix(mesh.m_Transform.GetLocalToWorldMatrixXM());

		MeshDataInput input = m_pBoundingBoxEffect->GetInputData(mesh);
		// 输入装配阶段的顶点缓冲区设置
		m_pBoundingBoxEffect->Apply(m_pd3dImmediateContext.Get());
		m_pd3dImmediateContext->IASetIndexBuffer(input.pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		m_pd3dImmediateContext->IASetVertexBuffers(0, 2,
			input.pVertexBuffers.data(), input.strides.data(), input.offsets.data());

		// 绘制立方体
		m_pd3dImmediateContext->DrawIndexed(24, 0, 0);
		//m_pBoundingBoxEffect->Apply(m_pd3dImmediateContext.Get());
		// 清除状态
		return true;
	}

	Graphics::MeshData* Renderer::createBoundingBoxMesh(BoundingBox* box) {
		Graphics::MeshData* meshData = new Graphics::MeshData();
		// meshData->m_BoundingBox = BoundingBox(XMFLOAT3(0, 0, 0), XMFLOAT3(50.0f, 50.0f, 50.0f));
		//  ******************
		//  设置立方体顶点
		//     5________ 6
		//     /|      /|
		//    /_|_____/ |
		//   1|4|_ _ 2|_|7
		//    | /     | /
		//    |/______|/
		//   0       3
		XMFLOAT3 center, extents, mnear, mfar;
		center = box->Center;
		extents = box->Extents;
		// center = meshData->m_BoundingBox.Center;
		// extents = meshData->m_BoundingBox.Extents;
		//  center = XMFLOAT3(box->Center.x/10, box->Center.y / 10, box->Center.z / 10);
		//  extents = XMFLOAT3(box->Extents.x / 10, box->Extents.y / 10, box->Extents.z / 10);
		mnear = XMFLOAT3(center.x - extents.x, center.y - extents.y, center.z - extents.z);
		mfar = XMFLOAT3(center.x + extents.x, center.y + extents.y, center.z + extents.z);

		XMFLOAT3 vertices[] =
		{
			{mnear},
			{XMFLOAT3(mnear.x, mfar.y, mnear.z)},
			{XMFLOAT3(mfar.x, mfar.y, mnear.z)},
			{XMFLOAT3(mfar.x, mnear.y, mnear.z)},

			{XMFLOAT3(mnear.x, mnear.y, mfar.z)},
			{XMFLOAT3(mnear.x, mfar.y, mfar.z)},
			{mfar},
			{XMFLOAT3(mfar.x, mnear.y, mfar.z)} };

		XMFLOAT4 colors[] = {
			XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f),
			XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f),
			XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f),
			XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f),
			XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f),
			XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f),
			XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f),
			XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f) };
		// VertexPosColor vertices[] =
		//{
		//	{ XMFLOAT3(-300.0f, -300.0f, -300.0f), XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f) },
		//	{ XMFLOAT3(-300.0f, 300.0f, -300.0f), XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f) },
		//	{ XMFLOAT3(300.0f, 300.0f, -300.0f), XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f) },
		//	{ XMFLOAT3(300.0f, -300.0f, -300.0f), XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f) },

		//	{ XMFLOAT3(-300.0f, -300.0f, 300.0f), XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f) },
		//	{ XMFLOAT3(-300.0f, 300.0f, 300.0f), XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f) },
		//	{ XMFLOAT3(300.0f, 300.0f, 300.0f), XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f)},
		//	{ XMFLOAT3(300.0f, -300.0f, 300.0f), XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f) }
		//};
		// 设置顶点缓冲区描述

		CD3D11_BUFFER_DESC bufferDesc(0, D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };
		bufferDesc.ByteWidth = sizeof vertices;
		initData.pSysMem = vertices;
		HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, meshData->m_pVertices.GetAddressOf()));

		bufferDesc.ByteWidth = sizeof colors;
		initData.pSysMem = colors;
		HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, meshData->m_pColors.GetAddressOf()));
		// ******************
		// 设置立方体顶点
		//    5________ 6
		//    /|      /|
		//   /_|_____/ |
		//  1|4|_ _ 2|_|7
		//   | /     | /
		//   |/______|/
		//  0       3
		// ******************
		// 索引数组
		//
		DWORD indices[] = {
			// 正面
			0, 1,
			1, 2,
			2, 3,
			3, 0,
			// 左面
			1, 5,
			5, 4,
			4, 0,
			// 背面
			4, 7,
			7, 6,
			6, 5,
			2, 6,
			3, 7 };
		// 设置索引缓冲区描述
		bufferDesc = CD3D11_BUFFER_DESC(sizeof indices, D3D11_BIND_INDEX_BUFFER);

		// 新建索引缓冲区
		initData.pSysMem = indices;
		HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, meshData->m_pIndices.GetAddressOf()));
		meshData->m_IndexCount = 24;
		return meshData;
	}

	Graphics::MeshData* Renderer::createHitPosSphere(FVec3& pos, float radius)
	{
		std::vector<XMFLOAT3> vertices;
		std::vector<XMFLOAT4> colors;
		std::vector<UINT> indices;
			XMFLOAT3 center(pos.X,pos.Y,pos.Z);
			XMFLOAT4 color = XMFLOAT4(0.5f, 0.5f, 0.9f, 1.0f);
			int vertsPerRow = 18;
			int nRows = 17;

			int nVerts = vertsPerRow * nRows + 2;


			for (int i = 1; i <= nRows; i++)
			{
				float phy = XM_PI * i / (nRows + 1);
				float tmpRadius = radius * sin(phy);
				for (int j = 0; j <= vertsPerRow; j++)
				{
					float theta = XM_2PI * j / vertsPerRow;

					float x = tmpRadius * cos(theta);
					float y = radius * cos(phy);
					float z = tmpRadius * sin(theta);

					//位置坐标
					vertices.push_back( XMFLOAT3(x + center.x, y + center.y, z + center.z));
					colors.push_back(color);

				}
			}

			int size =  (vertsPerRow+1) * nRows;
			//添加顶部和底部两个顶点信息
			vertices.push_back( XMFLOAT3(center.x, center.y+radius  ,center.z));
			colors.push_back(color);
			vertices.push_back( XMFLOAT3( center.x, center.y -radius,  center.z));
			colors.push_back(color);

			int start1 = 0;
			int start2 = size - vertsPerRow-1;
			int top = size;
			int bottom = size + 1;
			for (int i = 0; i < vertsPerRow; i++)
			{
				indices.push_back(top);
				indices.push_back(start1 + i + 1);
				indices.push_back(start1 + i);
				indices.push_back(start1 + i + 1);
				indices.push_back(start1 + i);
				indices.push_back(top);
			}

			for (int i = 0; i < vertsPerRow; i++)
			{
				indices.push_back(bottom);
				indices.push_back(start2 + i);
				indices.push_back(start2 + i);
				indices.push_back(start2 + i + 1);
				indices.push_back(start2 + i + 1);
				indices.push_back(bottom);


			}

			for (int i = 0; i < nRows-1 ; i++)
			{
				for (int j = 0; j < vertsPerRow; j++)
				{
					indices.push_back(i * vertsPerRow+i + j);
					indices.push_back((i + 1) * vertsPerRow + i + j + 1);
					indices.push_back((i + 1) * vertsPerRow + i + j + 1);
					indices.push_back((i + 1) * vertsPerRow + i + j);
					indices.push_back((i + 1) * vertsPerRow + i + j);
					indices.push_back(i * vertsPerRow + i + j);

					indices.push_back(i * vertsPerRow + i + j);
					indices.push_back(i * vertsPerRow + i + j + 1);
					indices.push_back(i * vertsPerRow + i + j + 1);
					indices.push_back((i + 1) * vertsPerRow + i + j + 1);
					indices.push_back((i + 1) * vertsPerRow + i + j + 1);
					indices.push_back(i * vertsPerRow + i + j);

				}
			}
		D3D11_BUFFER_DESC vbd;
		ZeroMemory(&vbd, sizeof(vbd));
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(XMFLOAT3) * vertices.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		// 新建顶点缓冲区
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = vertices.data();
		HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pHitPos->m_pVertices.GetAddressOf()));


		vbd.ByteWidth = sizeof(XMFLOAT4) * colors.size();
		InitData.pSysMem = colors.data();
		HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pHitPos->m_pColors.GetAddressOf()));
		// ******************
		// 索引数组
		//
		D3D11_BUFFER_DESC ibd;
		ZeroMemory(&ibd, sizeof(ibd));
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * indices.size();
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		// 新建索引缓冲区
		InitData.pSysMem = indices.data();
		HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pHitPos->m_pIndices.GetAddressOf()));
		m_pHitPos->m_IndexCount = indices.size();

		return m_pHitPos;
	}

	void Renderer::CreateBuffer(const CD3D11_BUFFER_DESC* desc, const D3D11_SUBRESOURCE_DATA* initData, ID3D11Buffer** buffer) {
		HR(m_pd3dDevice->CreateBuffer(desc, initData, buffer));
	}

	void Renderer::LoadSceneBuffer() {
		m_pd3dImmediateContext->ResolveSubresource(m_pSceneBuffer->GetTexture(), 0, m_pLitBuffer->GetTexture(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
	}

	bool Renderer::UpdateVerticesData(MeshData* meshData, std::vector<FVec3>& vertices) {
		D3D11_MAPPED_SUBRESOURCE MapedResource;
		HR(m_pd3dImmediateContext->Map(meshData->m_pVertices.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MapedResource));
		memcpy_s(MapedResource.pData,  sizeof(FVec3) * vertices.size(), vertices.data(), sizeof(FVec3) * vertices.size());

		m_pd3dImmediateContext->Unmap(meshData->m_pVertices.Get(), 0);
		
		return true;
	}
}