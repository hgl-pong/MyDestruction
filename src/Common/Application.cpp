#include "Application.h"
#include "DXTrace.h"
#include "XUtil.h"
#include <sstream>
#include "../Utils.h"
#include "../FPhysics.h"
#include "../FScene.h"
#include "../FActor.h"
#pragma warning(disable: 6031)
using namespace DirectX;
extern "C"
{
    // 在具有多显卡的硬件设备中，优先使用NVIDIA或AMD的显卡运行
    // 需要在.exe中使用
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
}

namespace
{
    // This is just used to forward Windows messages from a global window
    // procedure to our member function window procedure because we cannot
    // assign a member function to WNDCLASS::lpfnWndProc.
    Application* g_pApplication = nullptr;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Forward hwnd on because we can get messages (e.g., WM_CREATE)
    // before CreateWindow returns, and thus before m_hMainWnd is valid.
    return g_pApplication->MsgProc(hwnd, msg, wParam, lParam);
}

Application::Application(HINSTANCE hInstance)
    : m_hAppInst(hInstance),
    m_MainWndCaption(L"Fracture"),
    m_hMainWnd(nullptr),
    m_AppPaused(false),
    m_Minimized(false),
    m_Maximized(false),
    m_Resizing(false),
	m_pRenderer(new Graphics::Renderer(this)),
	m_pUIDrawer(new UIDrawer(this)),
	m_pMeshImporter(new MeshImporter(this)),
	m_pTextureImporter(new TextureImporter(this)),
    m_pMaterialManager(new MaterialManager(this)),
    m_pPhysics(new FPhysics())

{
	m_ClientHeight = *m_pRenderer->getClientHeight();
	m_ClientWidth = *m_pRenderer->getClientWidth();

    // 让一个全局指针获取这个类，这样我们就可以在Windows消息处理的回调函数
    // 让这个类调用内部的回调函数了
    g_pApplication = this;
}


Application::~Application()
{
	FDELETE(m_pRenderer);
	FDELETE(m_pUIDrawer);
	FDELETE(m_pTextureImporter);
    FDELETE(m_pMaterialManager);
	FDELETE(m_pMeshImporter);
}

HINSTANCE Application::AppInst()const
{
    return m_hAppInst;
}

HWND Application::MainWnd()const
{
    return m_hMainWnd;
}

float Application::AspectRatio()const
{
    return static_cast<float>(m_ClientWidth) /(m_ClientHeight);
}

int Application::Run()
{
    MSG msg = { 0 };

    m_Timer.Reset();

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            m_Timer.Tick();

            CalculateFrameStats();
            m_pUIDrawer->newFrame();
            UpdateScene(m_Timer.DeltaTime());
            DrawScene();
        }
    }

    return (int)msg.wParam;
}

bool Application::Init()
{

    if (!InitMainWindow())
        return false;
    Logger::Debug("Application","初始化窗口成功!");

    if (!m_pRenderer->InitDirect3D())
        return false;
    Logger::Debug(m_pRenderer->name,"初始化Direct3D成功!");

    if (!m_pMeshImporter->Init(m_pRenderer->getD3DDevice()))
        return false;
    Logger::Debug(m_pMeshImporter->name, "初始化MeshImporter成功!");

    if (!m_pTextureImporter->Init(m_pRenderer->getD3DDevice()))
        return false;
    Logger::Debug(m_pTextureImporter->name, "初始化TextureImporter成功!");

	auto viewerCamera = std::make_shared<FirstPersonCamera>();
	m_pViewerCamera = viewerCamera;
;
	m_pViewerCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	m_pViewerCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	m_pViewerCamera->LookAt(XMFLOAT3(0.0f, 60.0f, -120.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));


	auto lightCamera = std::make_shared<FirstPersonCamera>();
	m_pLightCamera = lightCamera;

	m_pLightCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	lightCamera->LookAt(XMFLOAT3(-320.0f, 300.0f, -220.3f), XMFLOAT3(), XMFLOAT3(0.0f, 1.0f, 0.0f));
	lightCamera->SetFrustum(XM_PI / 3, 1.0f, 0.1f, 1000.0f);

	m_FPSCameraController.InitCamera(viewerCamera.get());
	m_FPSCameraController.SetMoveSpeed(10.0f);

	if (!m_pPhysics->Init())
		return false;
	Logger::Debug("FPhysics", "初始化FPhysics成功！");

    m_pScene = new FScene();
    m_pScene->Init();
    m_pScene->SetSimulateState(true);
    m_pPhysics->AddScene(m_pScene);

    if (!m_pUIDrawer->InitUI())
         return false;
    Logger::Debug(m_pUIDrawer->name,"初始化UIDrawer成功!");

	if (!m_pRenderer->Init())
		return false;
    Logger::Debug(m_pRenderer->name,"初始化Renderer成功！");

    Logger::Debug("Application","加载完成!");

    FActor* actor=new FActor();
    char name[] = "box";
    actor->Init(name);
    actor->OnEnterScene(m_pScene);

    return true;
}

void Application::UpdateScene(float dt) {
	m_pUIDrawer->Move(dt);
	m_pUIDrawer->UpdateUI();
    m_pPhysics->Update();
	m_pRenderer->UpdateScene();
}

void Application::DrawScene() {
	m_pRenderer->DrawScene();
	m_pUIDrawer->RenderUI();
	m_pRenderer->SwapChainPresent();
}

void Application::OnResize()
{
    m_pRenderer->OnResize();
}

LRESULT Application::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (FAILED(m_pUIDrawer->UIHandler(hwnd, msg, wParam, lParam)))
		return E_FAIL;

    if (ImGui::GetCurrentContext()) {
        ImGuiIO& io = ImGui::GetIO();

        if (!io.WantCaptureMouse)
            switch (msg)
            {
                // WM_ACTIVATE is sent when the window is activated or deactivated.  
                // We pause the game when the window is deactivated and un pause it 
                // when it becomes active.  
            case WM_ACTIVATE:
                if (LOWORD(wParam) == WA_INACTIVE)
                {
                    m_AppPaused = true;
                    m_Timer.Stop();
                }
                else
                {
                    m_AppPaused = false;
                    m_Timer.Start();
                }
                return 0;

                // WM_SIZE is sent when the user resizes the window.  
            case WM_SIZE:
                // Save the new client area dimensions.
                m_ClientWidth = LOWORD(lParam);
                m_ClientHeight = HIWORD(lParam);
                if (m_pRenderer->is3dDeviceNotNull())
                {
                    if (wParam == SIZE_MINIMIZED)
                    {
                        m_AppPaused = true;
                        m_Minimized = true;
                        m_Maximized = false;
                    }
                    else if (wParam == SIZE_MAXIMIZED)
                    {
                        m_AppPaused = false;
                        m_Minimized = false;
                        m_Maximized = true;
                        OnResize();
                    }
                    else if (wParam == SIZE_RESTORED)
                    {

                        // Restoring from minimized state?
                        if (m_Minimized)
                        {
                            m_AppPaused = false;
                            m_Minimized = false;
                            OnResize();
                        }

                        // Restoring from maximized state?
                        else if (m_Maximized)
                        {
                            m_AppPaused = false;
                            m_Maximized = false;
                            OnResize();
                        }
                        else if (m_Resizing)
                        {
                            // If user is dragging the resize bars, we do not resize 
                            // the buffers here because as the user continuously 
                            // drags the resize bars, a stream of WM_SIZE messages are
                            // sent to the window, and it would be pointless (and slow)
                            // to resize for each WM_SIZE message received from dragging
                            // the resize bars.  So instead, we reset after the user is 
                            // done resizing the window and releases the resize bars, which 
                            // sends a WM_EXITSIZEMOVE message.
                        }
                        else // API call such as SetWindowPos or m_pSwapChain->SetFullscreenState.
                        {
                            OnResize();
                        }
                    }
                }
                return 0;

                // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
            case WM_ENTERSIZEMOVE:
                m_AppPaused = true;
                m_Resizing = true;
                m_Timer.Stop();
                return 0;

                // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
                // Here we reset everything based on the new window dimensions.
            case WM_EXITSIZEMOVE:
                m_AppPaused = false;
                m_Resizing = false;
                m_Timer.Start();
                OnResize();
                return 0;

                // WM_DESTROY is sent when the window is being destroyed.
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

                // The WM_MENUCHAR message is sent when a menu is active and the user presses 
                // a key that does not correspond to any mnemonic or accelerator key. 
            case WM_MENUCHAR:
                // Don't beep when we alt-enter.
                return MAKELRESULT(0, MNC_CLOSE);

                // Catch this message so to prevent the window from becoming too small.
            case WM_GETMINMAXINFO:
                ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
                ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
                return 0;
            }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}


bool Application::InitMainWindow()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hAppInst;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"D3DWndClassName";

    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
        return false;
    }

    // 将窗口调整到中心
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Compute window rectangle dimensions based on requested client area dimensions.
    RECT R = { 0, 0, m_ClientWidth, m_ClientHeight };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    m_hMainWnd = CreateWindow(L"D3DWndClassName", m_MainWndCaption.c_str(),
        WS_OVERLAPPEDWINDOW, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, 0, 0, m_hAppInst, 0);

    if (!m_hMainWnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(m_hMainWnd, SW_SHOW);
    UpdateWindow(m_hMainWnd);

    return true;
}

void Application::CalculateFrameStats()
{
    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        std::wostringstream outs;
        outs.precision(6);
        outs << m_MainWndCaption << L"    "
            << L"FPS: " << fps << L"    "
            << L"Frame Time: " << mspf << L" (ms)";
        SetWindowText(m_hMainWnd, outs.str().c_str());

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

