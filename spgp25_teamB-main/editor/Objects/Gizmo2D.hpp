/**
 * @file Gizmo2D.hpp
 * @author Dante Harper
 * @date 04/02/26
 */

#pragma once

#include <Toast/Physics/Line.hpp>

#include <glm/glm.hpp>
// clang-format off
#include <imgui.h>
#include <ImGuizmo.h>
#include <optional>
// clang-format on

namespace physics { 
	class Collider;
}

namespace editor {
class GizmoList2D {
	struct {
		unsigned int gizmoState = ImGuizmo::TRANSLATE_X | ImGuizmo::TRANSLATE_Y;
		// Ruler
		glm::vec2 p1;
		glm::vec2 p2;
		// Collider
		std::optional<unsigned> selectedPointId;
    std::vector<unsigned> multi;
    bool wasUsing = false; ///< track if gizmos were being used in the previous frame
	} m;

	void Collider(glm::mat4 view, glm::mat4 proj);
	void ColliderVertices(glm::mat4& view, glm::mat4& proj, physics::Collider* c);
	void ColliderEdges(glm::mat4& view, glm::mat4& proj, physics::Collider* c);
	void ColliderMulti(glm::mat4& view, glm::mat4& proj, physics::Collider* c);

public:
	void Show(glm::mat4 view, glm::mat4 proj);
};
}
