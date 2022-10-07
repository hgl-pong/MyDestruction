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
        bool Init();//��ʼ��  
        bool InitDirect3D();        // Direct3D��ʼ��
        void UpdateScene();   // ���ÿһ֡�ĸ���
        void DrawScene();             // ���ÿһ֡�Ļ���
        void DrawMeshQueue(IEffect* effect);
		bool AddRenderMesh(MeshData* data);
		bool RemoveRenderMesh(MeshData* data);
		bool AddVoroMesh(MeshData* data);
		bool RemoveVoroMesh(MeshData* data);
        bool AddRenderBoundingBox(size_t mesh_id, BoundingBox* box);
        bool RemoveRenderBoundingBox(size_t mesh_id);

		Graphics::MeshData* CreateVoroMeshData(FWireMesh*mesh);


        /**
         * �����Ӵ����
         * @param {int} width
         * @return {*}
         */
        void setClientWidth(int width);

        /**
         * �����Ӵ��߶�
         * @param {int} height
         * @return {*}
         */
        void setClientHeight(int height);

        int* getClientWidth();//��ȡ�Ӵ����

        int* getClientHeight();//��ȡ�Ӵ��߶�
        void CreateBuffer(const CD3D11_BUFFER_DESC* desc, const D3D11_SUBRESOURCE_DATA* initData, ID3D11Buffer** buffer);

        ID3D11Device* getD3DDevice();

        ID3D11DeviceContext* getD3DDeviceContext();

        bool is3dDeviceNotNull();

        void SwapChainPresent();

        void OnResize();                  // �ڴ��ڴ�С�䶯��ʱ�����
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
        // ʹ��ģ�����(C++11)��������
        template <class T>
        using ComPtr = Microsoft::WRL::ComPtr<T>;
        // Direct3D 11
        ComPtr<ID3D11Device> m_pd3dDevice;							// D3D11�豸
        ComPtr<ID3D11DeviceContext> m_pd3dImmediateContext;			// D3D11�豸������
        ComPtr<IDXGISwapChain> m_pSwapChain;						// D3D11������

        // Direct3D 11.1
        ComPtr<ID3D11Device1> m_pd3dDevice1;						// D3D11.1�豸
        ComPtr<ID3D11DeviceContext1> m_pd3dImmediateContext1;		// D3D11.1�豸������
        ComPtr<IDXGISwapChain1> m_pSwapChain1;						// D3D11.1������

        // ������Դ
        ComPtr<ID3D11Texture2D> m_pDepthStencilBuffer;				// ���ģ�建����
        ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;			// ��ȾĿ����ͼ
        ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;			// ���ģ����ͼ
        D3D11_VIEWPORT m_ScreenViewport;                            // �ӿ�

        bool m_IsDxgiFlipModel = false; // �Ƿ�ʹ��DXGI��תģ��
        UINT m_BackBufferCount = 0;		// �󱸻�������Ŀ
        UINT m_FrameCount = 0;          // ��ǰ֡
        ComPtr<ID3D11RenderTargetView> m_pRenderTargetViews[2];     // ���к󱸻�������Ӧ����ȾĿ����ͼ

    public:
        bool  m_Enable4xMsaa;	 // �Ƿ���4�����ز���	
        CascadedShadowManager m_CSManager;
        bool  use_skybox;//�Ƿ�ʹ����պУ�Ĭ��ʹ�ã�

        bool need_gpu_timer_reset = false;

        // MSAA
        int m_MsaaSamples = 1;

        // ��Ӱ

        bool m_DebugShadow = false;


        UINT      m_4xMsaaQuality;   // MSAA֧�ֵ������ȼ�

        /**
         * TEST
         */
         // ������Դ
        std::unique_ptr<Texture2D> m_pLitBuffer;                        // ������Ⱦ������
        std::unique_ptr<Depth2D> m_pDepthBuffer;                        // ��Ȼ�����
        std::unique_ptr<Texture2D> m_pSceneBuffer;				// ����


        SkyBox m_Skybox;											// ��պ�ģ��

        // ��Ч
        std::unique_ptr<ForwardEffect> m_pForwardEffect;				// ǰ����Ⱦ��Ч
        std::unique_ptr<ShadowEffect> m_pShadowEffect;					// ��Ӱ��Ч
        std::unique_ptr<SkyboxToneMapEffect> m_pSkyboxEffect;			// ��պ���Ч
        std::unique_ptr<BoudingBoxEffect> m_pBoundingBoxEffect;			// ��պ���Ч
        ComPtr<ID3D11ShaderResourceView> m_pTextureCubeSRV;				// ��պ�����

		std::set<Graphics::MeshData*>m_RenderMesh;
		std::set<Graphics::MeshData*>m_VoroMesh;
        Graphics::MeshData* m_pHitPos;
        int m_ClientWidth;                                           // �ӿڿ��
        int m_ClientHeight;                                          // �ӿڸ߶�
    };
}
#endif//RENDERER