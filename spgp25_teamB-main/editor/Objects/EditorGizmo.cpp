#include "EditorGizmo.hpp"

#include "Actions/Actions.hpp"
#include "App.hpp"
#include "Objects/EditorCamera.hpp"
#include "imgui.h"

#include <ImGuizmo.h>
#include <Toast/Engine.hpp>
#include <Toast/Objects/Scene.hpp>
#include <Toast/Renderer/DebugDrawLayer.hpp>
#include <Toast/Window/WindowEvents.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

namespace editor {
void EditorGizmo::Init() {
	m_listener.Subscribe<event::WindowKey>([this](const event::WindowKey* e) {
		if (!ToastEditor::viewport_window().isFocused) {
			return false;
		}
		if (ToastEditor::isSimulate) {
			return false;
		}
		if (e->key == '1') {
			m_gizmoState = TRANSLATE2D;
			UpdateState(m_viewMode);
		} else if (e->key == '2') {
			m_gizmoState = ROTATE2D;
			UpdateState(m_viewMode);
		} else if (e->key == '3') {
			m_gizmoState = SCALE2D;
			UpdateState(m_viewMode);
		} else if (e->key == '4') {
			m_gizmoState = UNIVERSAL2D;
			UpdateState(m_viewMode);
		}
		return false;
	});
}

void EditorGizmo::ShowButtons() {
	if (ImGui::Button(m_gizmoMode ? "Local" : "World")) {
		m_gizmoMode = (m_gizmoMode) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
	}
	if (ImGui::Button(m_gridEnabled ? "Grid On" : "Grid Off")) {
		m_gridEnabled = !m_gridEnabled;
	}
	if (ImGui::Button(m_gizmoEnabled ? "Gizmos On" : "Gizmos Off")) {
		m_gizmoEnabled = !m_gizmoEnabled;
	}
	if (ImGui::Button(m_debugDraw ? "Debug Draw On" : "Debug Draw Off")) {
		m_debugDraw = !m_debugDraw;
		renderer::DebugDrawLayer::GetInstance()->SetEnabled(m_debugDraw);
	}

	if (ImGui::BeginPopupContextItem("##ImGuizmoFlags", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
		ImGui::CheckboxFlags("TRANSLATE_X", &m_gizmoState, 1u << 0);
		ImGui::CheckboxFlags("TRANSLATE_Y", &m_gizmoState, 1u << 1);
		ImGui::CheckboxFlags("TRANSLATE_Z", &m_gizmoState, 1u << 2);
		ImGui::CheckboxFlags("ROTATE_X", &m_gizmoState, 1u << 3);
		ImGui::CheckboxFlags("ROTATE_Y", &m_gizmoState, 1u << 4);
		ImGui::CheckboxFlags("ROTATE_Z", &m_gizmoState, 1u << 5);
		ImGui::CheckboxFlags("ROTATE_SCREEN", &m_gizmoState, 1u << 6);
		ImGui::CheckboxFlags("SCALE_X", &m_gizmoState, 1u << 7);
		ImGui::CheckboxFlags("SCALE_Y", &m_gizmoState, 1u << 8);
		ImGui::CheckboxFlags("SCALE_Z", &m_gizmoState, 1u << 9);
		ImGui::CheckboxFlags("BOUNDS", &m_gizmoState, 1u << 10);
		ImGui::CheckboxFlags("SCALE_XU", &m_gizmoState, 1u << 11);
		ImGui::CheckboxFlags("SCALE_YU", &m_gizmoState, 1u << 12);
		ImGui::CheckboxFlags("SCALE_ZU", &m_gizmoState, 1u << 13);
		ImGui::Text("The Thing %u", m_gizmoState);
		ImGui::EndPopup();
	}

	float avail = ImGui::GetContentRegionAvail().x;
	std::array<char, 64> buffer;    // NOLINT(cppcoreguidelines-pro-type-member-init)
	sprintf(buffer.data(), "Scroll Speed: %.1f", EditorCamera::GetInstance()->scrollSpeed);
	float text_width = ImGui::CalcTextSize(buffer.data()).x;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - text_width);
	ImGui::Text("%s", buffer.data());
}

void EditorGizmo::ShowGrid(glm::mat4 view, glm::mat4 proj) const {
	if (m_gridEnabled) {
		// mat4 grid_model = rotate(mat4(1.0f), radians(-90.0f), vec3(1, 0, 0));
		// mat4 view_for_grid = view * grid_model;
		// ImGuizmo::DrawGrid(value_ptr(view_for_grid), value_ptr(proj), value_ptr(mat4(1.0f)), 100.0f);

		renderer::DebugDrawLayer::GetInstance()->DrawGrid(100, view * proj);
	}
}

void EditorGizmo::ShowGizmo(glm::mat4 view, glm::mat4 proj) {
	if (editor::ToastEditor::isSimulate) {
		return;
	}
	gizmoList.Show(view, proj);
	ToastEditor* editor = ToastEditor::Instance();
	if (ToastEditor::hierarchy().SelectedObject() && m_gizmoEnabled && !dynamic_cast<toast::Scene*>(ToastEditor::hierarchy().SelectedObject())) {
		// Can't manipulate scenes

		// Determine selected transform and its parent's world matrix
		toast::TransformComponent* selected_transform = nullptr;

		if (auto* t = dynamic_cast<toast::TransformComponent*>(ToastEditor::hierarchy().SelectedObject())) {
			selected_transform = t;
		} else if (auto* t = dynamic_cast<toast::Component*>(ToastEditor::hierarchy().SelectedObject())) {
			// selected_transform = dynamic_cast<toast::Actor*>(t->parent())->transform(); COMENTED OUT because if component is movable it should inherint
			// from transform comp
		} else if (auto* actor = dynamic_cast<toast::Actor*>(ToastEditor::hierarchy().SelectedObject())) {
			selected_transform = actor->transform();
		}

		// early exit if actor does not hac¡ve a transform
		if (!selected_transform) {
			return;
		}

		// Get selected object's world model matrix
		glm::mat4 model = selected_transform->GetWorldMatrix();

		toast::TransformComponent* parent_transform = nullptr;
		if (selected_transform) {
			toast::Object* p = selected_transform->parent();
			while (p && !parent_transform) {
				if (auto* pa = dynamic_cast<toast::Actor*>(p)) {
					parent_transform = pa->transform();
					// Avoid self as parent in case of owning actor edge case
					if (parent_transform == selected_transform) {
						parent_transform = nullptr;
					}
				} else if (auto* pt = dynamic_cast<toast::TransformComponent*>(p)) {
					parent_transform = pt;
				}
				if (!parent_transform) {
					p = p->parent();
				}
			}
		}

		glm::mat4 parent_world = glm::mat4(1.0f);
		if (parent_transform) {
			parent_world = parent_transform->GetWorldMatrix();
		}

		// Manipulate in world space, but convert back to local against parent after manipulation
		ImGuizmo::Manipulate(
		    glm::value_ptr(view),
		    glm::value_ptr(proj),
		    static_cast<ImGuizmo::OPERATION>(m_gizmoState),
		    static_cast<ImGuizmo::MODE>(m_gizmoMode),
		    glm::value_ptr(model)
		);

		if (ImGuizmo::IsUsing()) {
			if (!m_isManipulating) {
				TOAST_INFO("ID OF OBJECT{}", selected_transform->id());
				ActionStack::GizmoObject(selected_transform);
				m_isManipulating = true;
			}
			// Convert the edited world matrix back to local space relative to parent
			glm::mat4 local_mat = model;
			if (parent_transform) {
				// Invert parent's world safely
				glm::mat4 parent_inv = glm::inverse(parent_world);
				local_mat = parent_inv * model;
			}

			// Decompose local matrix and write into local TRS
			glm::vec3 translation, rotation, scale;
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(local_mat), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));

			selected_transform->position(translation);
			selected_transform->rotation(rotation);
			selected_transform->scale(scale);
		} else {
			m_isManipulating = false;
		}
	}
}

void EditorGizmo::UpdateState(ViewMode view_mode) {
	m_viewMode = view_mode;
	// clang-format off
	switch (m_gizmoState) {
		case TRANSLATE3D:
		case TRANSLATE2D: 
      m_gizmoState = (view_mode) ? TRANSLATE3D : TRANSLATE2D; break;
		case ROTATE3D:
		case ROTATE2D: 
      m_gizmoState = (view_mode) ? ROTATE3D : ROTATE2D; break;
		case SCALE3D:
		case SCALE2D: 
      m_gizmoState = (view_mode) ? SCALE3D : SCALE2D; break;
		case UNIVERSAL3D:
		case UNIVERSAL2D: 
      m_gizmoState = (view_mode) ? UNIVERSAL3D : UNIVERSAL2D; break;
	}
	// clang-format on
}
}
