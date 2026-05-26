#include "Gizmo2D.hpp"

#include "App.hpp"
#include "Objects/EditorCamera.hpp"

#include <ImGuizmo.h>
#include <Toast/Renderer/DebugDrawLayer.hpp>
#include <Toast/World.hpp>
#include <imgui.h>
#include <iterator>
#include <ranges>
#define GLM_ENABLE_EXPERIMENTAL
#include "Toast/Physics/Collider.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"

namespace editor {

void GizmoList2D::Collider(glm::mat4 view, glm::mat4 proj) {
	auto* obj = editor::ToastEditor::hierarchy().SelectedObject();
	auto* collider = dynamic_cast<physics::Collider*>(obj);
	if (not collider) {
		m.selectedPointId = std::nullopt;
		return;
	}

	switch (collider->currentEditMode()) {
		case physics::ColliderEditMode::VERTICES: ColliderVertices(view, proj, collider); break;
		case physics::ColliderEditMode::EDGES: ColliderEdges(view, proj, collider); break;
		case physics::ColliderEditMode::MULTI: ColliderMulti(view, proj, collider); break;
		default: break;
	}
}

void GizmoList2D::ColliderVertices(glm::mat4& view, glm::mat4& proj, physics::Collider* c) {
	if (m.selectedPointId) {
		if (ImGui::IsKeyPressed(ImGuiKey_B)) {
			c->Bevel(m.selectedPointId.value());
		}

		if (ImGui::IsKeyPressed(ImGuiKey_X)) {
			if (c->GetPoints().size() > 3) {
				c->DeleteAt(m.selectedPointId.value());
				m.selectedPointId = std::nullopt;
			}
			return;
		}

		if (ImGui::IsKeyPressed(ImGuiKey_C) || ImGui::IsMouseClicked(4)) {
			c->CalculatePoints();
		}

		if (ImGui::IsKeyPressed(ImGuiKey_N)) {
			auto edges = c->GetEdges();
			auto it = edges.begin();
			std::advance(it, m.selectedPointId.value());
			physics::Line l = *it;
			glm::vec2 pos = l.p1 + l.tangent * (l.length / 2);
			c->AddPointAt(m.selectedPointId.value(), pos);
			c->CalculatePoints();
			return;
		}
	}

	auto& points = c->GetPoints();

	auto world_mtx = dynamic_cast<toast::Actor*>(c->parent())->transform()->GetWorldMatrix();
	auto world_inv = dynamic_cast<toast::Actor*>(c->parent())->transform()->GetInverse();

	for (size_t i = 0; i < points.size(); ++i) {
		ImGuizmo::PushID(static_cast<int>(i));
		auto& vec = points[i];
		auto point = glm::vec4(vec.x, vec.y, 0, 1);
		auto calc = world_mtx * point;

		glm::mat4 pos = glm::translate(glm::identity<glm::mat4>(), { calc.x, calc.y, 0 });
		ImGuizmo::Manipulate(
		    glm::value_ptr(view),
		    glm::value_ptr(proj),
		    static_cast<ImGuizmo::OPERATION>(m.gizmoState),
		    static_cast<ImGuizmo::MODE>(m.gizmoState),
		    glm::value_ptr(pos)
		);

		if (ImGuizmo::IsUsing()) {
			m.selectedPointId = static_cast<unsigned>(i);
			glm::vec3 t, r, s;
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(pos), glm::value_ptr(t), glm::value_ptr(r), glm::value_ptr(s));
			vec = world_inv * glm::vec4(t.x, t.y, t.z, 1);
			m.wasUsing = true;    // remember that we're currently using the gizmo
		} else {
			// if we were using the gizmo last frame but not this one, the user released it
			if (m.wasUsing) {
				//c->CalculatePoints();
				m.wasUsing = false;
			}
		}
		ImGuizmo::PopID();
	}
}

void GizmoList2D::ColliderMulti(glm::mat4& view, glm::mat4& proj, physics::Collider* c) {
	if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
		m.multi.clear();
	}

	auto world_mtx = dynamic_cast<toast::Actor*>(c->parent())->transform()->GetWorldMatrix();
	auto world_inv = dynamic_cast<toast::Actor*>(c->parent())->transform()->GetInverse();

	auto& points = c->GetPoints();
	for (size_t i = 0; i < points.size(); ++i) {
		ImGuizmo::PushID(static_cast<int>(i));
		auto& vec = points[i];
		auto point = glm::vec4(vec.x, vec.y, 0, 1);
		auto calc = world_mtx * point;

		glm::mat4 pos = glm::translate(glm::identity<glm::mat4>(), { calc.x, calc.y, 0 });
		ImGuizmo::Manipulate(
		    glm::value_ptr(view),
		    glm::value_ptr(proj),
		    static_cast<ImGuizmo::OPERATION>(m.gizmoState),
		    static_cast<ImGuizmo::MODE>(m.gizmoState),
		    glm::value_ptr(pos)
		);

		if (ImGuizmo::IsUsing()) {
			if (std::ranges::find(m.multi, static_cast<unsigned>(i)) == m.multi.end()) {
				m.multi.push_back(static_cast<unsigned>(i));
			}
			m.wasUsing = true;
		}
		ImGuizmo::PopID();
	}
	if (m.multi.size() < 2) {
		return;
	}
	glm::vec2 old_point {};
	{
		auto& points_ref = c->GetPoints();
		for (size_t index = 0; index < points_ref.size(); ++index) {
			if (std::ranges::find(m.multi, static_cast<unsigned>(index)) != m.multi.end()) {
				old_point += points_ref[index];
			}
		}
	}
	old_point /= static_cast<float>(m.multi.size());

	auto point = glm::vec4(old_point.x, old_point.y, 0, 1);
	auto calc = world_mtx * point;

	glm::mat4 pos = glm::translate(glm::identity<glm::mat4>(), { calc.x, calc.y, 0 });
	ImGuizmo::PushID(6767);
	ImGuizmo::Manipulate(
	    glm::value_ptr(view),
	    glm::value_ptr(proj),
	    static_cast<ImGuizmo::OPERATION>(m.gizmoState),
	    static_cast<ImGuizmo::MODE>(m.gizmoState),
	    glm::value_ptr(pos)
	);

	if (ImGuizmo::IsUsing()) {
		glm::vec3 t, r, s;
		ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(pos), glm::value_ptr(t), glm::value_ptr(r), glm::value_ptr(s));
		glm::vec2 new_point = world_inv * glm::vec4(t.x, t.y, t.z, 1);
		glm::vec2 diff = new_point - old_point;

		{
			auto& points_ref = c->GetPoints();
			for (size_t index = 0; index < points_ref.size(); ++index) {
				if (std::ranges::find(m.multi, static_cast<unsigned>(index)) != m.multi.end()) {
					points_ref[index] += diff;
				}
			}
		}

		c->CalculatePoints();
	}
	ImGuizmo::PopID();
}

void GizmoList2D::ColliderEdges(glm::mat4& view, glm::mat4& proj, physics::Collider* c) {
	if (m.selectedPointId && ImGui::IsKeyPressed(ImGuiKey_N)) {
		auto edges = c->GetEdges();
		auto it = edges.begin();
		std::advance(it, m.selectedPointId.value());
		physics::Line l = *it;
		glm::vec2 pos = l.p1 + l.tangent * (l.length / 2);
		c->AddPointAt(m.selectedPointId.value(), pos);
		c->CalculatePoints();
		return;
	}

	auto edges = c->GetEdges();

	auto world_mtx = dynamic_cast<toast::Actor*>(c->parent())->transform()->GetWorldMatrix();
	auto world_inv = dynamic_cast<toast::Actor*>(c->parent())->transform()->GetInverse();

	for (const auto& [i, edge] : edges | std::views::enumerate) {
		ImGuizmo::PushID(static_cast<int>(i));
		auto p = glm::mix(edge.p1, edge.p2, 0.5f);
		auto point = glm::vec4(p.x, p.y, 0, 1);
		auto calc = world_mtx * point;

		glm::mat4 pos = glm::translate(glm::identity<glm::mat4>(), { calc.x, calc.y, 0 });
		ImGuizmo::Manipulate(
		    glm::value_ptr(view),
		    glm::value_ptr(proj),
		    static_cast<ImGuizmo::OPERATION>(m.gizmoState),
		    static_cast<ImGuizmo::MODE>(m.gizmoState),
		    glm::value_ptr(pos)
		);

		if (ImGuizmo::IsUsing()) {
			// dont ever ask how this works
			m.selectedPointId = i;
			glm::vec3 t, r, s;
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(pos), glm::value_ptr(t), glm::value_ptr(r), glm::value_ptr(s));
			glm::vec2 midpoint = world_inv * glm::vec4(t.x, t.y, t.z, 1);
			glm::vec2 p1 = midpoint - glm::vec2(edge.tangent) * static_cast<float>(edge.length / 2);
			glm::vec2 p2 = midpoint + glm::vec2(edge.tangent) * static_cast<float>(edge.length / 2);

			auto& points = c->GetPoints();
			auto point_iterator = points.begin();
			std::advance(point_iterator, i);
			*point_iterator = p1;
			auto next_point = std::next(point_iterator);
			if (next_point == points.end()) {
				next_point = points.begin();
			}
			*next_point = p2;

			c->CalculatePoints();

			ImGuizmo::PopID();
			return;
		}

		ImGuizmo::PopID();
	}
}

void GizmoList2D::Show(glm::mat4 view, glm::mat4 proj) {
	if (editor::ToastEditor::isSimulate) {
		return;
	}
	Collider(view, proj);

	if (editor::ToastEditor::status_bar().ruler) {
		glm::mat4 pos1 = glm::translate(glm::identity<glm::mat4>(), { m.p1.x, m.p1.y, 0 });
		glm::mat4 pos2 = glm::translate(glm::identity<glm::mat4>(), { m.p2.x, m.p2.y, 0 });

		ImGuizmo::PushID(67);
		ImGuizmo::Manipulate(
		    glm::value_ptr(view),
		    glm::value_ptr(proj),
		    static_cast<ImGuizmo::OPERATION>(m.gizmoState),
		    static_cast<ImGuizmo::MODE>(m.gizmoState),
		    glm::value_ptr(pos1)
		);
		if (ImGuizmo::IsUsing()) {
			glm::vec3 t, r, s;
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(pos1), glm::value_ptr(t), glm::value_ptr(r), glm::value_ptr(s));
			m.p1 = t;
		}
		ImGuizmo::PopID();

		ImGuizmo::PushID(76);
		ImGuizmo::Manipulate(
		    glm::value_ptr(view),
		    glm::value_ptr(proj),
		    static_cast<ImGuizmo::OPERATION>(m.gizmoState),
		    static_cast<ImGuizmo::MODE>(m.gizmoState),
		    glm::value_ptr(pos2)
		);
		if (ImGuizmo::IsUsing()) {
			glm::vec3 t, r, s;
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(pos2), glm::value_ptr(t), glm::value_ptr(r), glm::value_ptr(s));
			m.p2 = t;
		}
		ImGuizmo::PopID();

		ImGui::TextColored({ 0xff / 255.0f, 0x16 / 255.0f, 0x59 / 255.0f, 1 }, "%s", std::format("Distance: {:.2f}", glm::length(m.p1 - m.p2)).c_str());
		renderer::DebugLine(m.p1, m.p2, { 0xff / 255.0f, 0x16 / 255.0f, 0x59 / 255.0f, 1 });
	} else {
		auto cam = EditorCamera::GetInstance()->transform()->position();
		m.p1 = cam;
		m.p2 = cam;
	}
}

}
