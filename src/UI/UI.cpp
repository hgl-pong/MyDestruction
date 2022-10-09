#include "UI.h"
#include "../Common/Application.h"
#include "../Common/XUtil.h"
#include <Windows.h>
#include <DirectXColors.h>
#include "../FScene.h"
using namespace DirectX;
UIDrawer::UIDrawer(Application*application)
{
	name = "UI Drawer";
	app = application;
}

UIDrawer::~UIDrawer() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

bool UIDrawer::InitUI() {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // 允许键盘控制
	io.ConfigWindowsMoveFromTitleBarOnly = true;              // 仅允许标题拖动

	// 设置Dear ImGui风格
	ImGui::StyleColorsDark();

	// 设置平台/渲染器后端
	ImGui_ImplWin32_Init(FindWindow(NULL,L"Destruction Editor"));
	ImGui_ImplDX11_Init(app->m_pRenderer->getD3DDevice(), app->m_pRenderer->getD3DDeviceContext());

	return true;
}

void UIDrawer::Move(float dt) {
	// 更新摄像机
// 	if (app->m_pRenderer->m_CSManager.m_SelectedCamera <= CameraSelection::CameraSelection_Light)
// 		app->m_FPSCameraController.Update(dt);

	ImGuiIO& io = ImGui::GetIO();

	// ******************
	// 自由摄像机的操作
	//
	float d1 = 0.0f, d2 = 0.0f, d3 = 0.0f;
	if (ImGui::IsKeyDown('W'))
		d1 += dt;
	if (ImGui::IsKeyDown('S'))
		d1 -= dt;
	if (ImGui::IsKeyDown('A'))
		d2 -= dt;
	if (ImGui::IsKeyDown('D'))
		d2 += dt;
	if (ImGui::IsKeyDown('E'))
		d3 -= dt;
	if (ImGui::IsKeyDown('Q'))
		d3 += dt;
	app->m_pViewerCamera->MoveForward(d1 * 10.0f*speed);
	app->m_pViewerCamera->Strafe(d2 * 10.0f*speed);
	app->m_pViewerCamera->UpAndDown(d3 * 10.0f * speed);

	if (io.WantCaptureMouse)
		return;
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
	{
		app->m_pViewerCamera->Pitch(io.MouseDelta.y * 0.01f);
		app->m_pViewerCamera->RotateY(io.MouseDelta.x * 0.01f);
	}
}


void UIDrawer::newFrame() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void UIDrawer::RenderUI() {
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

LRESULT UIDrawer::UIHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);
}

void UIDrawer::drawRendererControlPanel()
{
	ImGui::Begin("Control Panel",0, ImGuiWindowFlags_NoCollapse);
	ImGui::SliderInt("Camera Speed", &speed, 1, 10);

	if (ImGui::Button("OK")) {

	}
	ImGui::End();
}

void UIDrawer::UpdateUI() {
	drawRendererControlPanel();
	//drawSceneWindow();
	drawMenuBar();
	pick();
	//drawCascadeShadowWindow();
}

void UIDrawer::drawMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4 previous_color = style.Colors[ImGuiCol_Text];
		style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		if (ImGui::BeginMenu("File"))
		{
			style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

			if (ImGui::MenuItem("Save File")) {
				//App->blast->CreateBlastFile();
			}

			ImGui::EndMenu();
			style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		}

		if (ImGui::BeginMenu("Run"))
		{
			style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

			if (ImGui::MenuItem("Run"))
			{
				app->m_pScene->SetSimulateState(true);
			}
			if (ImGui::MenuItem("Pause"))
			{
				app->m_pScene->SetSimulateState(false);
			}
			if (ImGui::MenuItem("Replay"))
			{

			}
			ImGui::EndMenu();
			style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		}

		if (ImGui::BeginMenu("Tools"))
		{
			style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

			if (ImGui::MenuItem("Chunk Tree"))
			{

			}
			if (ImGui::MenuItem("Fracture Tool"))
			{

			}
			ImGui::EndMenu();
			style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		}
		ImGui::EndMainMenuBar();
		style.Colors[ImGuiCol_Text] = previous_color;
	}
}

void UIDrawer::pick()
{

	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		XMFLOAT3 pos = app->m_pViewerCamera->GetPosition();
		//Ray ray = Ray(pos, XMFLOAT3(-pos.x,26-pos.y,-pos.z));
		Ray ray = Ray::ScreenToRay(*app->m_pViewerCamera.get(), (float)io.MousePos.x, (float)io.MousePos.y);
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			app->m_pScene->Intersection(ray);
	}

}

