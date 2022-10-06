#ifndef UI_H
#define UI_H

#ifdef _WINDOWS_
#include <wrl/client.h>
#endif

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "../Renderer/Renderer.h"
#include "../Common/Camera.h"
#include "../Common/Base.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class UIDrawer:public Base{
public:
	UIDrawer(Application*app);
	~UIDrawer();
	bool InitUI();
	void Move(float dt);
	void newFrame();
	void RenderUI();
	void UpdateUI();
	void drawMenuBar();
	LRESULT UIHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	void drawRendererControlPanel();
private:
	int speed = 1;
};

#endif //UI_H