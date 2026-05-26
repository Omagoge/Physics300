/**
 * @file EditorGizmo.hpp
 * @author Dante / Dario
 * @date 28/10/25
 *
 * @brief Editor Gizmo
 */

#pragma once

#include "Objects/Gizmo2D.hpp"

#include <Toast/Event/ListenerComponent.hpp>
#include <glm/fwd.hpp>

// clang-format off
#include <imgui.h>
#include <ImGuizmo.h>
// clang-format on

enum GizmoState : unsigned int {
	TRANSLATE3D = ImGuizmo::TRANSLATE,
	ROTATE3D = ImGuizmo::ROTATE,
	SCALE3D = ImGuizmo::SCALE,
	UNIVERSAL3D = ImGuizmo::UNIVERSAL,

	TRANSLATE2D = (1u << 0) | (1u << 1),
	ROTATE2D = ImGuizmo::ROTATE_Z,
	SCALE2D = (1u << 7) | (1u << 8),
	UNIVERSAL2D = TRANSLATE2D | ROTATE2D | ImGuizmo::SCALE_XU | ImGuizmo::SCALE_YU,
};

enum ViewMode : char {
	VIEW2D = 0,
	VIEW3D
};

namespace editor {
class EditorGizmo {
public:
	void Init();
	void ShowButtons();
	void ShowGrid(glm::mat4 view, glm::mat4 proj) const;
	void ShowGizmo(glm::mat4 view, glm::mat4 proj);
	void UpdateState(ViewMode view_mode);

private:
	event::ListenerComponent m_listener;
	ViewMode m_viewMode;
	bool m_gridEnabled = true;
	bool m_gizmoEnabled = true;
	bool m_debugDraw = true;

	unsigned int m_gizmoMode = ImGuizmo::LOCAL;
	unsigned int m_gizmoState = UNIVERSAL2D;
	bool m_isManipulating = false;

	GizmoList2D gizmoList;
};
}
