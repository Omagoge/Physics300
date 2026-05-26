#include "Viewport.hpp"

#include "App.hpp"
#include "Objects/EditorCamera.hpp"
#include "Objects/EditorGizmo.hpp"
#include "Toast/Components/AtlasRendererComponent.hpp"
#include "Toast/Components/AtlasSpriteComponent.hpp"
#include "imgui.h"

#include <Toast/Input/InputListener.hpp>
#include <Toast/Renderer/DebugDrawLayer.hpp>
#include <Toast/Renderer/HUD/HUDLayer.hpp>
#include <Toast/Renderer/OpenGL/OpenGLRenderer.hpp>
#include <functional>
#include <glm/gtc/type_ptr.hpp>

namespace editor {

void ViewportWindow::Init() {
	m_gizmos.Init();
}

void ViewportWindow::SetViewMode(ViewMode view_mode) {
	m_viewMode = view_mode;
	m_gizmos.UpdateState(m_viewMode);
	EditorCamera::GetInstance()->SetViewMode(m_viewMode);
}

bool ViewportWindow::is_inFucking_Actual_Focus() {
	// If viewport rect hasn't been initialized this frame, bail out
	if (m_viewportMax.x <= m_viewportMin.x || m_viewportMax.y <= m_viewportMin.y) {
		return false;
	}

	const ImVec2 mouse = ImGui::GetIO().MousePos;
	return mouse.x >= m_viewportMin.x && mouse.x <= m_viewportMax.x && mouse.y >= m_viewportMin.y && mouse.y <= m_viewportMax.y;
}

void ViewportWindow::ShowViewportStatusBar() {
	if (ImGui::BeginMenuBar()) {
		if (ImGui::Button(m_viewMode ? "3D" : "2D")) {
			SetViewMode((m_viewMode) ? VIEW2D : VIEW3D);
		}
		m_gizmos.ShowButtons();
		ImGui::EndMenuBar();
	}
}

void ViewportWindow::Show() {
	PROFILE_ZONE;
	ShowViewport();
	// ShowGame(); // TODO: this has no actualy imp
}

void ViewportWindow::ShowViewport() {
	PROFILE_ZONE;
	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_MenuBar);
	isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	ShowViewportStatusBar();

	const float avail_x = ImGui::GetContentRegionAvail().x;
	const float avail_y = ImGui::GetContentRegionAvail().y;
	int fb_w = static_cast<int>(std::fmax(1.0f, floorf(avail_x)));
	int fb_h = static_cast<int>(std::fmax(1.0f, floorf(avail_y)));

	// Send size and pos to inputsystem
	auto pos = ImGui::GetCursorScreenPos();

	// Cache viewport rect for hit testing
	m_viewportMin = pos;
	m_viewportMax = ImVec2(pos.x + fb_w, pos.y + fb_h);

	input::SetViewportPosition({ pos.x, pos.y });
	input::SetViewportSize({ fb_w, fb_h });

	if (auto* hud = renderer::HUD::HUDLayer::Get()) {
		hud->SetViewportOffset(static_cast<int>(pos.x), static_cast<int>(pos.y));
	}

	auto* fb = renderer::IRendererBase::GetInstance()->GetMainFramebuffer();
	if (fb->Width() != fb_w || fb->Height() != fb_h) {
		renderer::IRendererBase::GetInstance()->Resize(glm::uvec2(fb_w, fb_h));
	}
	GLuint tex = fb->GetColorTexture(0);


	ImGui::SetNextItemAllowOverlap();

	ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y));
	ImGui::Image(static_cast<intptr_t>(tex), ImVec2(fb_w, fb_h), ImVec2(0, 1), ImVec2(1, 0));

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AtlasSprite")) {
			auto* callback = reinterpret_cast<std::function<toast::AtlasSpriteComponent*()>**>(payload->Data);
			auto* obj = (**callback)();
			auto mouse_pos = ToastEditor::hierarchy().lastMouseWorldPos;
			obj->worldPosition({ mouse_pos.x, mouse_pos.y, 0 });
		}
		ImGui::EndDragDropTarget();
	}

	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(pos.x, pos.y, static_cast<float>(fb_w), static_cast<float>(fb_h));

	glm::mat4 view = renderer::IRendererBase::GetInstance()->GetViewMatrix();
	glm::mat4 projection = renderer::IRendererBase::GetInstance()->GetProjectionMatrix();

	m_gizmos.ShowGrid(view, projection);
	m_gizmos.ShowGizmo(view, projection);

	ImGui::End();
}

void ViewportWindow::ShowGame() {
	PROFILE_ZONE;
	ImGui::Begin("Game", nullptr, ImGuiWindowFlags_MenuBar);

	const float avail_x = ImGui::GetContentRegionAvail().x;
	const float avail_y = ImGui::GetContentRegionAvail().y;
	int fb_w = static_cast<int>(std::fmax(1.0f, floorf(avail_x)));
	int fb_h = static_cast<int>(std::fmax(1.0f, floorf(avail_y)));
	auto* fb = renderer::IRendererBase::GetInstance()->GetMainFramebuffer();
	if (fb->Width() != fb_w || fb->Height() != fb_h) {
		renderer::IRendererBase::GetInstance()->Resize(glm::uvec2(fb_w, fb_h));
	}
	ImVec2 pos = ImGui::GetCursorScreenPos();
	GLuint tex = fb->GetColorTexture(0);
	ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t)tex, ImVec2(pos.x, pos.y), ImVec2(pos.x + fb_w, pos.y + fb_h), ImVec2(0, 1), ImVec2(1, 0));
	ImGui::End();
}
}
