#ifndef APPLICATION
#define APPLICATION

#include <wrl/client.h>
#include <string>
#include <d2d1.h>
#include <dwrite.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include "../renderer/Renderer.h"
#include "../Logger/Logger.h"
#include "../Importer/MeshImporter.h"
#include "../Importer/TextureImporter.h"
#include "MaterialManager.h"
#include "CpuTimer.h"
#include "../UI/UI.h"

namespace Graphics {
    class Renderer;
}
class FPhysics;
class FScene;
class Application
{
public:
    enum class AppState
    {
        RUN,
        STOP,
        REPLAY
    };
    Application(HINSTANCE hInstance);    // 在构造函数的初始化列表应当设置好初始参数
    virtual ~Application();

    HINSTANCE AppInst()const;       // 获取应用实例的句柄
    HWND      MainWnd()const;       // 获取主窗口句柄
    float     AspectRatio()const;   // 获取屏幕宽高比

    int Run();                      // 运行程序，执行消息事件的循环

    bool Init();                      // 初始化窗口、Direct2D和Direct3D部分
    void OnResize();                  // 在窗口大小变动的时候调用
    void UpdateScene(float dt);   // 完成每一帧的更新
    void DrawScene();             // 完成每一帧的绘制
    LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);// 窗口的消息回调函数
protected:
    bool InitMainWindow();      // 窗口初始化
    void CalculateFrameStats(); // 计算每秒帧数并在窗口显示

protected:

    HINSTANCE m_hAppInst;        // 应用实例句柄

    bool      m_AppPaused;       // 应用是否暂停
    bool      m_Minimized;       // 应用是否最小化
    bool      m_Maximized;       // 应用是否最大化
    bool      m_Resizing;        // 窗口大小是否变化

	

    CpuTimer m_Timer;           // 计时器


    // 派生类应该在构造函数设置好这些自定义的初始参数
    std::wstring m_MainWndCaption;                               // 主窗口标题

    
    int m_ClientWidth;                                           // 窗口宽度
    int m_ClientHeight;                                          // 窗口高度
 
public:
    Graphics::Renderer* m_pRenderer; //渲染器
    UIDrawer* m_pUIDrawer; //UI绘制
    MeshImporter* m_pMeshImporter;
    TextureImporter* m_pTextureImporter;
    MaterialManager* m_pMaterialManager;
    FPhysics* m_pPhysics;
    FScene* m_pScene;

	// 摄像机
	std::shared_ptr<Camera> m_pViewerCamera;						// 用户摄像机
	std::shared_ptr<Camera> m_pLightCamera;							// 光源摄像机
	FirstPersonCameraController m_FPSCameraController;				// 摄像机控制器

    HWND      m_hMainWnd;        // 主窗口句柄

    	// GPU计时
	GpuTimer m_GpuTimer_Shadow;
	GpuTimer m_GpuTimer_Lighting;
	GpuTimer m_GpuTimer_Skybox;
    GpuTimer m_GpuTimer_BoundingBox;

};

#endif //APPLICATION
