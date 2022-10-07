#ifndef RENDERER
#define RENDERER


#include "../Effects.h"
#include "../Common/SkyBox.h"
#include "../Common/Collision.h"
#include "../Common/Base.h"
#include "../Common/RenderStates.h"

#include "../Common/GpuTimer.h"

#include "../Common/CascadedShadowManager.h"

#include "imgui.h"
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <filesystem>
#include <set>
#include "vec.h"
class FWireMesh;
namespace Graphics {
    class Renderer :public Base {
    public:
        Renderer(Application* application);
        ~Renderer();
        Renderer(Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = default;
        Renderer& operator=(Renderer&&) = default;

        static Renderer* Get();
        /**
         * @description:
         * @return {*}
         */
        bool isUseSkyBox();
        bool Init();//初始化  
        bool InitDirect3D();        // Direct3D初始化
        void UpdateScene();   // 完成每一帧的更新
        void DrawScene();             // 完成每一帧的绘制
        void DrawMeshQueue(IEffect* effect);
		bool AddRenderMesh(MeshData* data);
		bool RemoveRenderMesh(MeshData* data);
		bool AddVoroMesh(MeshData* data);
		bool RemoveVoroMesh(MeshData* data);
        bool AddRenderBoundingBox(size_t mesh_id, BoundingBox* box);
        bool RemoveRenderBoundingBox(size_t mesh_id);

		Graphics::MeshData* CreateVoroMeshData(FWireMesh*mesh);


        /**
         * 设置视窗宽度
         * @param {int} width
         * @return {*}
         */
        void setClientWidth(int width);

        /**
         * 设置视窗高度
         * @param {int} height
         * @return {*}
         */
        void setClientHeight(int height);

        int* getClientWidth();//获取视窗宽度

        int* getClientHeight();//获取视窗高度
        void CreateBuffer(const CD3D11_BUFFER_DESC* desc, const D3D11_SUBRESOURCE_DATA* initData, ID3D11Buffer** buffer);

        ID3D11Device* getD3DDevice();

        ID3D11DeviceContext* getD3DDeviceContext();

        bool is3dDeviceNotNull();

        void SwapChainPresent();

        void OnResize();                  // 在窗口大小变动的时候调用
        float AspectRatio()const;

        void LoadSceneBuffer(); 
		Graphics::MeshData* createBoundingBoxMesh(BoundingBox* box);
        Graphics::MeshData* createHitPosSphere(FVec3& pos, float radius);
    private:

        bool InitResource();
		bool DrawMesh(MeshData* mesh, IEffect* effect);
        bool DrawVoroMesh(MeshData* mesh);
        bool DrawBoundingBox(MeshData& mesh);


        void RenderShadowForAllCascades();
        void RenderForward();
        void RenderSkyboxAndToneMap();
        void RenderBoundingBoxes();


        ID3D11RenderTargetView* GetBackBufferRTV() { return m_pRenderTargetViews[m_FrameCount % m_BackBufferCount].Get(); }


    protected:
        // 使用模板别名(C++11)简化类型名
        template <class T>
        using ComPtr = Microsoft::WRL::ComPtr<T>;
        // Direct3D 11
        ComPtr<ID3D11Device> m_pd3dDevice;							// D3D11设备
        ComPtr<ID3D11DeviceContext> m_pd3dImmediateContext;			// D3D11设备上下文
        ComPtr<IDXGISwapChain> m_pSwapChain;						// D3D11交换链

        // Direct3D 11.1
        ComPtr<ID3D11Device1> m_pd3dDevice1;						// D3D11.1设备
        ComPtr<ID3D11DeviceContext1> m_pd3dImmediateContext1;		// D3D11.1设备上下文
        ComPtr<IDXGISwapChain1> m_pSwapChain1;						// D3D11.1交换链

        // 常用资源
        ComPtr<ID3D11Texture2D> m_pDepthStencilBuffer;				// 深度模板缓冲区
        ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;			// 渲染目标视图
        ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;			// 深度模板视图
        D3D11_VIEWPORT m_ScreenViewport;                            // 视口

        bool m_IsDxgiFlipModel = false; // 是否使用DXGI翻转模型
        UINT m_BackBufferCount = 0;		// 后备缓冲区数目
        UINT m_FrameCount = 0;          // 当前帧
        ComPtr<ID3D11RenderTargetView> m_pRenderTargetViews[2];     // 所有后备缓冲区对应的渲染目标视图

    public:
        bool  m_Enable4xMsaa;	 // 是否开启4倍多重采样	
        CascadedShadowManager m_CSManager;
        bool  use_skybox;//是否使用天空盒（默认使用）

        bool need_gpu_timer_reset = false;

        // MSAA
        int m_MsaaSamples = 1;

        // 阴影

        bool m_DebugShadow = false;


        UINT      m_4xMsaaQuality;   // MSAA支持的质量等级

        /**
         * TEST
         */
         // 各种资源
        std::unique_ptr<Texture2D> m_pLitBuffer;                        // 场景渲染缓冲区
        std::unique_ptr<Depth2D> m_pDepthBuffer;                        // 深度缓冲区
        std::unique_ptr<Texture2D> m_pSceneBuffer;				// 场景


        SkyBox m_Skybox;											// 天空盒模型

        // 特效
        std::unique_ptr<ForwardEffect> m_pForwardEffect;				// 前向渲染特效
        std::unique_ptr<ShadowEffect> m_pShadowEffect;					// 阴影特效
        std::unique_ptr<SkyboxToneMapEffect> m_pSkyboxEffect;			// 天空盒特效
        std::unique_ptr<BoudingBoxEffect> m_pBoundingBoxEffect;			// 天空盒特效
        ComPtr<ID3D11ShaderResourceView> m_pTextureCubeSRV;				// 天空盒纹理

		std::set<Graphics::MeshData*>m_RenderMesh;
		std::set<Graphics::MeshData*>m_VoroMesh;
        Graphics::MeshData* m_pHitPos;
        int m_ClientWidth;                                           // 视口宽度
        int m_ClientHeight;                                          // 视口高度
    };
}
#endif//RENDERER